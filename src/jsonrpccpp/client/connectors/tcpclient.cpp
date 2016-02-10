/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    tcpclient.cpp
 * @date    11.12.2015
 * @author  Sascha Matti <sascha_matti@bluewin.ch>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "tcpclient.h"
#include "../../server/abstractprotocolhandler.h"


#include <signal.h>
#include <unistd.h>
#include <fcntl.h>


using namespace jsonrpc;
using namespace std;


#define SIG_WAKEUP     (49)

/* Signal handler. */
static void signalHandler(int sig)
{

}

TcpClient::TcpClient(const std::string& url, unsigned short usPort,
                     TcpClientCallback& xCallback, unsigned int uiSelectTimeout)
	: AbstractServerConnector(),
	    TcpConnection(url),
	    m_bConnected(false),
	    m_usPort(usPort),
	    m_xListenerThread(),
	    m_xPersistenceThread(),
	    m_uiSelectTimeout(uiSelectTimeout),
	    m_bListening(false),
	    m_xCallback(xCallback),
	    m_bPersistentListening(false)
{
	pthread_mutex_init(&m_xMapMutex, NULL);
	pthread_mutex_init(&m_xRequestQueueMutex, NULL);
	pthread_cond_init( &m_xRequestQueueSignal, NULL);
}

TcpClient::~TcpClient()
{
	StopPersistentListening();
	StopListening();
	pthread_mutex_destroy(&m_xMapMutex);
	pthread_mutex_destroy( &m_xRequestQueueMutex);
	pthread_cond_destroy( &m_xRequestQueueSignal);
}

bool TcpClient::SendResponse(const std::string& response, void* addInfo,
                             SMessageHeader xMessageHeader)
{
	if(addInfo)
	{
		TcpConnection* pxConnection = static_cast<TcpConnection*>(addInfo);
		if(m_bConnected)
		{
			pxConnection->putOutgoingResponse(response, xMessageHeader);
			return true;
		}
	}
	return false;

}

void TcpClient::SendRPCMessage(const std::string& message, std::string& result)
    throw(JsonRpcException)
{
	StartListening();
	putRequest(message);
	wakeup();
//	if(result != "notification")
//	{
//		fprintf(stdout, "%s : %d\n", __FILE__, __LINE__);
//		getResponseBlocking(uiId, result);
//	}
	return;
}

void TcpClient::SetTimeout(long timeout)
{
	m_uiSelectTimeout = timeout;
}

void TcpClient::closeConnection()
{
	if(m_bConnected && !m_bListening)
	{
		m_bConnected = false;
		close(m_iSocket);
		m_iSocket = -1;
	}
}

void TcpClient::initConnection()
{
	if(!m_bConnected)
	{
		m_bConnectionSetUp = false;

		fflush(stdout);
		int iErrNo = 0;

		struct sockaddr_in server;

		// first, load up address structs with getaddrinfo():
		server.sin_addr.s_addr = inet_addr(m_strIpAddress.data());
		server.sin_family = AF_INET;
		server.sin_port = htons(m_usPort);

		// make a socket:
		m_iSocket = socket(AF_INET, SOCK_STREAM, 0);
		if(m_iSocket < 0)
		{
			iErrNo = errno;
			m_xCallback.rpcNotifyError(strerror(iErrNo));
			return;
		}

		// connect!
		int iConn = connect(m_iSocket, (struct sockaddr *) &server, sizeof(server));
		if(iConn == -1)
		{
			iErrNo = errno;
			m_xCallback.rpcNotifyError(strerror(iErrNo));
			close(m_iSocket);
		}
		else
		{
			m_bConnected = true;
		}
	}
}

bool TcpClient::StartListening()
{
	initConnection();
	if(m_bConnected)
	{
		int iRet = 0;
		if(!m_bListening)
		{
			iRet = pthread_create( &m_xListenerThread, NULL, &TcpClient::listenerLoop, this);
		}
		return iRet == 0; // return true if prhtread_create returned 0, e.g. no error
	}
	else
	{
		return false;
	}
}

bool TcpClient::StartPersistentListening()
{
	int iRet = 0;
	if( !m_bPersistentListening)
	{
		m_bPersistentListening = true;
		iRet = pthread_create( &m_xPersistenceThread, NULL, &TcpClient::persistentListeningLoop,
		    this);

		m_bPersistentListening = (iRet == 0);
	}
	return iRet == 0;
}

void TcpClient::StopPersistentListening()
{
	if(m_bPersistentListening)
	{
		m_bPersistentListening = false;
		pthread_cancel(m_xPersistenceThread);
	}

	return;
}

bool TcpClient::StopListening()
{
	int iRet = 0;
	if(m_bListening)
	{
		m_bListening = false;
		iRet = pthread_join(m_xListenerThread, NULL);
		if(iRet != 0)
		{
			pthread_cancel(m_xListenerThread);
		}
	}
	closeConnection();
	return iRet == 0;
}

void* TcpClient::listenerLoop(void* pxInstance)
{
	int iErrNo = 0;
	TcpClient* pxThis = static_cast<TcpClient*>(pxInstance);
	if(!pxThis->m_bListening && pxThis->m_bConnected)
	{
		pxThis->m_bListening = true;

		fd_set fdSetRead; // reading set
		fd_set fdSetWrite; // writing set

		char szReceiveBuffer[256]; // buffer for client data
		int iBytesReceived = 0;

		struct timespec sTv;

		int iSocket = pxThis->m_iSocket;

		sigset_t xSelectSet;
		sigset_t xOriginalSet;
		struct sigaction actionHandler;
		memset (&actionHandler, 0, sizeof(actionHandler));

		sigemptyset (&xSelectSet);
		sigaddset(&xSelectSet, SIG_WAKEUP);
		if(pthread_sigmask(SIG_BLOCK, &xSelectSet, &xOriginalSet) == -1) {
			iErrNo = errno;
			pxThis->m_xCallback.rpcNotifyError(strerror(iErrNo));
			pxThis->m_bListening = false;
			pxThis->closeConnection();
			return 0;
		}


		actionHandler.sa_handler = signalHandler;
		actionHandler.sa_flags = 0;
		sigemptyset(&actionHandler.sa_mask);

		/* This server should shut down on SIGTERM. */
		if (sigaction(SIG_WAKEUP, &actionHandler, 0)) {
			iErrNo = errno;
			pxThis->m_xCallback.rpcNotifyError(strerror(iErrNo));
			pxThis->m_bListening = false;
			pxThis->closeConnection();
			return 0;
		}

		while(pxThis->m_bListening && pxThis->m_bConnected)
		{
			pthread_mutex_lock(&pxThis->m_xMapMutex);
			FD_ZERO(&fdSetWrite);// clear sets, connections may have been closed or new send-data in buffer
			FD_ZERO(&fdSetRead);
			FD_SET(iSocket, &fdSetRead); // add listener to read set

			if(pxThis->hasPendingMessages())
			{
				FD_SET(iSocket, &fdSetWrite);
			}
			pthread_mutex_unlock(&pxThis->m_xMapMutex);

			sTv.tv_sec = pxThis->m_uiSelectTimeout / 1000;
			sTv.tv_nsec = (pxThis->m_uiSelectTimeout - (sTv.tv_sec * 1000)) * 1000000;
			int iSelectRes = pselect(iSocket + 1, &fdSetRead, &fdSetWrite, NULL, &sTv, &xOriginalSet);
			iErrNo = errno;

			if(iSelectRes == 0)
			{
				// select timed out
			}
			else if(iSelectRes < 0 && iErrNo == EINTR)
			{
				// wake up signal received
			}
			else if(iSelectRes < 0 && iErrNo != EINTR)
			{
				// select error
				pxThis->m_xCallback.rpcNotifyError(strerror(iErrNo));
				pxThis->m_bListening = false;
				pxThis->closeConnection();
			}
			else
			{
				// select reports one of the sockets ready for use
				pthread_mutex_lock(&pxThis->m_xMapMutex);

				if(FD_ISSET(iSocket, &fdSetRead))
				{
					iBytesReceived = recv(iSocket, szReceiveBuffer, sizeof(szReceiveBuffer), 0);
					if(iBytesReceived < 0)
					{
						iErrNo = errno;
						pxThis->m_xCallback.rpcNotifyError(strerror(iErrNo));
						pxThis->m_bListening = false;
						pxThis->closeConnection();
						return 0;
					}
					else if(iBytesReceived == 0)
					{
						pxThis->m_xCallback.rpcNotifyError("Server closed Connection.");
						pxThis->m_bListening = false;
						pxThis->closeConnection();
						return 0;
					}
					else
					{
						// message received
						char* szSeeker = szReceiveBuffer;
						int iBytes = 0;
						int iProcessed = 0;
						while(iProcessed < iBytesReceived)
						{
							char* szFrom = szSeeker;
							bool bTerminated = false;
							iBytes = 0;
							while(((iProcessed + ++iBytes) < (iBytesReceived))
							    && ( *++szSeeker != '\0'))
							{
							}
							if( *szSeeker == '\0')
							{
								++iBytes;
								++szSeeker;
								bTerminated = true;
							}
							pxThis->appendIncoming(szFrom, iBytes - bTerminated);
							iProcessed += iBytes;
							if(bTerminated)
							{
								string strMessage;
								pxThis->getIncomingBuffer(strMessage);
								pxThis->clearReceiveBuffer();

								TcpConnection::SMessageHeader xDebugMessageHeader;
								TcpConnection::readIDHeader(strMessage, xDebugMessageHeader, false);
//								fprintf(stdout,
//								"%s : %d , received message of %d bytes: \"%s\" : header: %d, %d, %d.\n",
//								__FILE__, __LINE__, strMessage.size() + 1, strMessage.c_str(), xDebugMessageHeader.uiSender, xDebugMessageHeader.uiReceiver, xDebugMessageHeader.uiMessageId);
//								fflush(stdout);

//								if(pxThis->isIncomingResponse(strMessage))
//								{
//									fprintf(stdout, "%s : %d.\n", __FILE__, __LINE__);fflush(stdout);
//									pxThis->putIncomingResponse(strMessage);
//								}
//								else
//								{
									SRequest xRequest;
									xRequest.uiConnectionId = pxThis->m_uiConnectionId;
									xRequest.strRequest = strMessage;
//									if(pxThis->m_bContextSwitch && pxThis->m_pxCallback)
//									{
										pthread_mutex_unlock(&pxThis->m_xMapMutex);
										pxThis->handleRequestContextSwitch(xRequest);
										pthread_mutex_lock(&pxThis->m_xMapMutex);
//									}
//									else
//									{
//										pthread_t xThread;
//										pthread_attr_t attr; // thread attribute
//										// set thread detachstate attribute to DETACHED
//										pthread_attr_init( &attr);
//										pthread_attr_setdetachstate( &attr,
//										PTHREAD_CREATE_DETACHED);
//
//										// create the thread
//										pthread_create( &xThread, &attr,
//											&TcpClient::handleRequestAsynch, pxRequest);
//									}
//								}
							}
						}
					}
				}
				else if(FD_ISSET(iSocket, &fdSetWrite))
				{
					int iSent = pxThis->sendComplete();
					if(iSent == -1)
					{
						iErrNo = errno;
						pxThis->m_xCallback.rpcNotifyError(strerror(iErrNo));
						pxThis->m_bListening = false;
						pxThis->closeConnection();
						break;
					}
					else if(iSent == 0)
					{
						// message sent successfully
//						fprintf(stdout, "%s : %d , sent message of %d (%d, %d) bytes: \"%s\".\n",
//						__FILE__, __LINE__, iTotal, iSent, iLeft, strMessage.c_str());
//						fflush(stdout);
					}
					else
					{
						pxThis->m_xCallback.rpcNotifyError(
						    "Could not send complete message: %d of %d bytes sent\n");
					}
				}
				pthread_mutex_unlock(&pxThis->m_xMapMutex);
			}
		}
	}
	pxThis->m_bListening = false;
	return 0;
}

//void* TcpClient::handleRequestAsynch(void* pxRequest)
//{
//	fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);fflush(stdout);
//	SRequest* pxReq = static_cast<SRequest*>(pxRequest);
//	IClientConnectionHandler* xHandler = pxReq->pxClient->GetHandler();
//
//	SMessageHeader xMessageHeader;
//
//	string strResponse;
//	TcpConnection::readIDHeader(pxReq->strRequest, xMessageHeader);
//
//	if(xMessageHeader.uiMessageId == 0)
//	{
//		fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);fflush(stdout);
//		// no request, just header
//		pxReq->pxClient->initIdentification(xMessageHeader);
//	}
//	else
//	{
//		fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);fflush(stdout);
//		xHandler->HandleRequest(pxReq->strRequest, strResponse);
//		if(strResponse.size() > 0)
//		{
//			pxReq->pxClient->SendResponse(strResponse, pxReq->pxConnection, xMessageHeader);
//		}
//	}
//	delete pxReq;
//	return 0;
//}

void* TcpClient::persistentListeningLoop(void* pxTcpClient)
{
	TcpClient* pxThis = static_cast<TcpClient*>(pxTcpClient);

	bool bConnected = false;
	bool bInitialized = false;
	while(pxThis && pxThis->m_bPersistentListening)
	{
		if(pxThis && ( !pxThis->m_bConnected || !pxThis->m_bListening))
		{
			bConnected = pxThis->StartListening();
			if(!bConnected)
			{
				bInitialized = false;
				pxThis->m_xCallback.rpcConnected(bConnected);
			}
		}
		if(!bInitialized && bConnected && pxThis->m_bConnectionSetUp)
		{
			pxThis->m_xCallback.rpcConnected(bConnected);
			bInitialized = true;
		}
		if(!bInitialized && bConnected && !pxThis->m_bConnectionSetUp)
		{
			usleep(10000);
		}
		else
		{
			usleep(5000000);
		}
	}
	return 0;
}


void TcpClient::handleRequestContextSwitch(SRequest& xRequest)
{
	Json::Reader xJsonReader;
	Json::Value xJsonRequest;
	string strMethod;
	TcpConnection::SMessageHeader xMessageHeader;
	TcpConnection::readIDHeader(xRequest.strRequest, xMessageHeader);

	if(xMessageHeader.uiMessageId == 0)
	{
		// no request, just header
		initIdentification(xMessageHeader);
	}
	if(!xRequest.strRequest.empty())
	{
		if(xJsonReader.parse(xRequest.strRequest, xJsonRequest, false))
		{
			if(xJsonRequest.isMember(KEY_REQUEST_METHODNAME))
			{
				strMethod = xJsonRequest[KEY_REQUEST_METHODNAME].asString();
			}
		}
		TcpConnection::prependIDHeader(xRequest.strRequest, xMessageHeader);
		queueRequest(xRequest);
		m_xCallback.rpcRequestReceived(strMethod, xJsonRequest[KEY_REQUEST_PARAMETERS]);
	}
}

void TcpClient::processRequest()
{
	SRequest xRequest;
	getQueuedRequest(xRequest);
	TcpConnection::SMessageHeader xMessageHeader;
	string strResponse;
	TcpConnection::readIDHeader(xRequest.strRequest, xMessageHeader);
	GetHandler()->HandleRequest(xRequest.strRequest, strResponse);
//	if(strResponse.size() > 0)
//	{
//		fprintf(stdout, "%s : %d.\n", __FILE__, __LINE__);fflush(stdout);
//		SendResponse(strResponse, this, xMessageHeader);
//	}
}


void TcpClient::getQueuedRequest(SRequest& xRequest)
{
	pthread_mutex_lock( &m_xRequestQueueMutex);
	if(!m_vecRequestQueue.empty())
	{
		xRequest = m_vecRequestQueue.front();
		m_vecRequestQueue.erase(m_vecRequestQueue.begin());
//		fprintf(stdout, "%s : %d; popped queued request: \"%s\"\n", __FILE__, __LINE__, xRequest.strRequest.c_str());fflush(stdout);
//		fprintf(stdout, "%s : %d; queue size: %u\n", __FILE__, __LINE__, m_vecRequestQueue.size());fflush(stdout);
	}
	else
	{
	}
	pthread_mutex_unlock( &m_xRequestQueueMutex);
}

void TcpClient::queueRequest(SRequest& xRequest)
{
	pthread_mutex_lock( &m_xRequestQueueMutex);
	m_vecRequestQueue.push_back(xRequest);
//	fprintf(stdout, "%s : %d; queued request: \"%s\"\n", __FILE__, __LINE__, xRequest.strRequest.c_str());fflush(stdout);
//	fprintf(stdout, "%s : %d; queue size: %u\n", __FILE__, __LINE__, m_vecRequestQueue.size());fflush(stdout);
	pthread_mutex_unlock( &m_xRequestQueueMutex);
}

void TcpClient::wakeup()
{
	if(m_xListenerThread > 0)
	{
		int iRet = pthread_kill(m_xListenerThread, SIG_WAKEUP);
//		fprintf(stdout, "%s : %d; sent wakeup call, iRet = %d\n", __FILE__, __LINE__, iRet);fflush(stdout);
	}
}

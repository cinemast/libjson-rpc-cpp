/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    bdhttpserver.cpp
 * @date    11.12.2015
 * @author  Sascha Matti <sascha_matti@bluewin.ch>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "tcpserver.h"
#include "../abstractprotocolhandler.h"


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


/**
 * @brief HttpServer, constructor for the included HttpServer
 * @param port on which the server is listening
 * @param enableSpecification - defines if the specification is returned in case of a GET request
 */
TcpServer::TcpServer(unsigned short usPort, TcpServerCallback& xCallback, unsigned int uiSelectTimeout)
	: AbstractServerConnector(),
	    m_bListening(false),
	    m_usPort(usPort),
	    m_uiConnectionIdGen(1),
	    m_xListenerThread(),
	    m_uiSelectTimeout(uiSelectTimeout),
	    m_uiServerId(1),
	    m_xCallback(xCallback)
{
	pthread_mutex_init( &m_xMutex, NULL);
	pthread_mutex_init(&m_xRequestQueueMutex, NULL);
	pthread_cond_init( &m_xRequestQueueSignal, NULL);
}

TcpServer::~TcpServer()
{
	StopListening();
	pthread_mutex_destroy( &m_xMutex);
	pthread_mutex_destroy( &m_xRequestQueueMutex);
	pthread_cond_destroy( &m_xRequestQueueSignal);
}

bool TcpServer::SendResponse(const std::string& response, void* pxConnection,
                             TcpConnection::SMessageHeader xMessageHeader)
{
	if(pxConnection)
	{
		TcpConnection* pxConn = static_cast<TcpConnection*>(pxConnection);
		pxConn->putOutgoingResponse(response, xMessageHeader);
		return true;
	}
	return false;
}

bool TcpServer::SendOptionsResponse(void* pxConnection)
{
	if(pxConnection)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool TcpServer::StartListening()
{
	int iRet = -1;
	if( !m_bListening)
	{
		m_bListening = true;
		iRet = pthread_create( &m_xListenerThread, NULL, &TcpServer::listenerLoop, this);
	}
	return iRet == 0; // return true if prhtread_create returned 0, e.g. no error
}

bool TcpServer::StopListening()
{
	int iRet = -1;
	if(m_bListening)
	{
		m_bListening = false;
		iRet = pthread_join(m_xListenerThread, NULL);
	}
	return iRet == 0;
}

void TcpServer::SendRPCMessage(const std::string& message, std::string& result,
                               unsigned int uiConnectionID) throw(JsonRpcException)
{
	pthread_mutex_lock( &m_xMutex);
	map<unsigned int, TcpConnection>::iterator itConnection = m_mapConnections.find(
	    uiConnectionID);
	bool bFound = (itConnection != m_mapConnections.end());
	if(bFound)
	{
//		bool bNotification = (result != "notification");
		itConnection->second.putRequest(message);
		wakeup();
//		if(bNotification)
//		{
//			itConnection->second->getResponseBlocking(uiId, result);
//			fprintf(stdout, "response: \"%s\"\n", result.c_str());
//			fflush(stdout);
//		}
	}
	else
	{
		fprintf(stdout,
		    "%s : %d , unable to match RPC Message to connection. Connection with id %d not found.\n",
		    __FILE__, __LINE__, uiConnectionID);
		fflush(stdout);
	}
	pthread_mutex_unlock( &m_xMutex);
}

void* TcpServer::listenerLoop(void* pxInstance)
{
	int iErrNo = 0;
	TcpServer* pxThis = static_cast<TcpServer*>(pxInstance);
	if(pxThis->m_bListening)
	{
		fd_set fdSetRead; // reading set
		fd_set fdSetWrite; // writing set

		int iFdMax; // maximum file descriptor number
		int iNewFd; // newly accept()ed socket descriptor

		struct sockaddr_in remoteaddr; // client address
		socklen_t addrlen;

		char szReceiveBuffer[256]; // buffer for client data
		int iBytesReceived = 0;

		char remoteIP[INET_ADDRSTRLEN];

		int iListener = socket(AF_INET, SOCK_STREAM, 0);
		if(iListener < 0 || iListener >= FD_SETSIZE)
		{
			iErrNo = errno;
			pxThis->m_xCallback.rpcNotifyError(0, strerror(iErrNo));
			close(iListener);
			iListener = -1;
			pxThis->m_bListening = false;
			return 0;
		}

		int iOn = 1;
		int iSetSockOptRes = setsockopt(iListener, SOL_SOCKET, SO_REUSEADDR, (char*) &iOn,
		    sizeof(iOn));
		if(iSetSockOptRes < 0)
		{
			iErrNo = errno;
			pxThis->m_xCallback.rpcNotifyError(0, strerror(iErrNo));
			close(iListener);
			iListener = -1;
			pxThis->m_bListening = false;
			return 0;
		}

		sockaddr_in xListenerAddress;

		struct timespec sTv;

		// Bind socket to the ip address
		size_t addrLen = sizeof(xListenerAddress);
		memset( &xListenerAddress, 0, addrLen);
		xListenerAddress.sin_family = AF_INET;
		xListenerAddress.sin_port = htons(pxThis->m_usPort);
		xListenerAddress.sin_addr.s_addr = INADDR_ANY;

		int iBindRes = bind(iListener, (sockaddr*) &xListenerAddress, addrLen);
		if(iBindRes < 0)
		{
			iErrNo = errno;
			pxThis->m_xCallback.rpcNotifyError(0, strerror(iErrNo));
			close(iListener);
			iListener = -1;
			pxThis->m_bListening = false;
			return 0;
		}

		int iListenRes = listen(iListener, 20);
		if(iListenRes < 0)
		{
			iErrNo = errno;
			pxThis->m_xCallback.rpcNotifyError(0, strerror(iErrNo));
			close(iListener);
			pxThis->m_bListening = false;
			return 0;
		}

		map<int, TcpConnection*> mapConnections;

		sigset_t xSelectSet;
		sigset_t xOriginalSet;
		struct sigaction actionHandler;
		memset (&actionHandler, 0, sizeof(actionHandler));

		sigemptyset (&xSelectSet);
		sigaddset(&xSelectSet, SIG_WAKEUP);
		if(pthread_sigmask(SIG_BLOCK, &xSelectSet, &xOriginalSet) == -1) {
			iErrNo = errno;
			pxThis->m_xCallback.rpcNotifyError(0, strerror(iErrNo));
			close(iListener);
			pxThis->m_bListening = false;
			return 0;
		}


		actionHandler.sa_handler = signalHandler;
		actionHandler.sa_flags = 0;
		sigemptyset(&actionHandler.sa_mask);

		/* This server should shut down on SIGTERM. */
		if (sigaction(SIG_WAKEUP, &actionHandler, 0)) {
			iErrNo = errno;
			pxThis->m_xCallback.rpcNotifyError(0, strerror(iErrNo));
			close(iListener);
			pxThis->m_bListening = false;
			return 0;
		}


		while(pxThis->m_bListening)
		{
			pthread_mutex_lock( &pxThis->m_xMutex);
			mapConnections.clear();
			FD_ZERO(&fdSetWrite); // clear sets, connections may have been closed or new send-data in buffer
			FD_ZERO(&fdSetRead);
			FD_SET(iListener, &fdSetRead); // add listener to read set
			iFdMax = iListener;

			map<unsigned int, TcpConnection>::iterator it;
			int iSockBuffer = 0;
			for(it = pxThis->m_mapConnections.begin(); it != pxThis->m_mapConnections.end(); ++it)
			{
				iSockBuffer = it->second.m_iSocket;
				mapConnections[iSockBuffer] = &(it->second);
				FD_SET(iSockBuffer, &fdSetRead);
				if(mapConnections[iSockBuffer]->hasPendingMessages())
				{
					FD_SET(iSockBuffer, &fdSetWrite);
				}
				iFdMax = (iSockBuffer > iFdMax ? iSockBuffer : iFdMax);
			}
			pthread_mutex_unlock( &pxThis->m_xMutex);


			sTv.tv_sec = pxThis->m_uiSelectTimeout / 1000;
			sTv.tv_nsec = (pxThis->m_uiSelectTimeout - (sTv.tv_sec * 1000)) * 1000000;
			int iSelectRes = pselect(iFdMax + 1, &fdSetRead, &fdSetWrite, NULL, &sTv, &xOriginalSet);
			int iErrNo = errno;

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
				pxThis->m_xCallback.rpcNotifyError(0, strerror(iErrNo));
			}
			else
			{
				// select reports one of the sockets ready for use
				pthread_mutex_lock( &pxThis->m_xMutex);

				for(int i = 0; i <= iFdMax; ++i)
				{
					if(FD_ISSET(i, &fdSetRead))
					{
						if(i == iListener)
						{
							// new connection
							addrlen = sizeof remoteaddr;
							iNewFd = accept(iListener, (struct sockaddr *) &remoteaddr, &addrlen);

							if(iNewFd == -1)
							{
								iErrNo = errno;
								pthread_mutex_unlock( &pxThis->m_xMutex);
								pxThis->m_xCallback.rpcNotifyError(0, strerror(iErrNo));
								pthread_mutex_lock( &pxThis->m_xMutex);
							}
							else
							{
								// new connection

								iFdMax = (iNewFd > iFdMax ? iNewFd : iFdMax);
								inet_ntop( AF_INET, &remoteaddr.sin_addr.s_addr, remoteIP,
								INET_ADDRSTRLEN);

								unsigned int uiClient = ++pxThis->m_uiConnectionIdGen;

								bool bAccept = true;
								unsigned int uiServer = ++pxThis->m_uiConnectionIdGen;
								pxThis->m_mapConnections[uiServer].init(uiServer, uiClient,iNewFd, remoteIP);
								pthread_mutex_unlock( &pxThis->m_xMutex);
								bAccept = pxThis->m_xCallback.newConnection(uiServer, remoteIP);
								pthread_mutex_lock( &pxThis->m_xMutex);
								if(bAccept)
								{
									string strMessage;
									TcpConnection::SMessageHeader xMessageHeader;
									xMessageHeader.uiSender = uiServer;
									xMessageHeader.uiReceiver = uiClient;
									xMessageHeader.uiMessageId = 0;

									// send empty message (header only) to set clients connection id
									pxThis->SendResponse(strMessage, &(pxThis->m_mapConnections[uiServer]),
									    xMessageHeader);
								}
								else
								{
									// delete created connection and reset idGen
									pxThis->eraseQueuedRequests(uiServer);
									pxThis->m_mapConnections.erase(uiServer);
									close(iNewFd);
								}
							}
						}
						else
						{
							iBytesReceived = recv(i, szReceiveBuffer, sizeof(szReceiveBuffer), 0);
							if(iBytesReceived < 0)
							{
								// error
								iErrNo = errno;
								unsigned int uiId = mapConnections[i]->m_uiConnectionId;
								close(i);
								map<unsigned int, TcpConnection>::iterator it = pxThis
								    ->m_mapConnections.begin();
								while(it != pxThis->m_mapConnections.end()
								    && i != it->second.m_iSocket)
								{
									++it;
								}
								if(it != pxThis->m_mapConnections.end())
								{
									pxThis->m_mapConnections.erase(it);
								}
								pxThis->eraseQueuedRequests(uiId);
								pthread_mutex_unlock( &pxThis->m_xMutex);
								pxThis->m_xCallback.rpcNotifyError(uiId, strerror(iErrNo));
								pthread_mutex_lock( &pxThis->m_xMutex);
							}
							else if(iBytesReceived == 0)
							{
								// client closed connection
								unsigned int uiId = mapConnections[i]->m_uiConnectionId;
								close(i);
								map<unsigned int, TcpConnection>::iterator it = pxThis
								    ->m_mapConnections.begin();
								while(it != pxThis->m_mapConnections.end()
								    && i != it->second.m_iSocket)
								{
									++it;
								}
								pxThis->m_mapConnections.erase(it);
								pxThis->eraseQueuedRequests(uiId);
								pthread_mutex_unlock( &pxThis->m_xMutex);
								pxThis->m_xCallback.rpcNotifyError(uiId, "Client closed connection\n");
								pthread_mutex_lock( &pxThis->m_xMutex);
							}
							else
							{
								// message received
								TcpConnection* pxConnection = mapConnections[i];
								char* szSeeker = szReceiveBuffer;
								int iBytes = 0;
								int iProcessed = 0;// message sent successfully

								while(iProcessed < iBytesReceived)
								{
									char* szFrom = szSeeker;
									bool bTerminated = false;
									iBytes = 0;
									while(((++iBytes + iProcessed) < (iBytesReceived))
									    && ( *++szSeeker != '\0'))
									{
									}
									if(*szSeeker == '\0')
									{
										++iBytes;
										++szSeeker;
										bTerminated = true;
									}
									pxConnection->appendIncoming(szFrom, iBytes - bTerminated);
									iProcessed += iBytes;

									if(bTerminated)
									{
										string strMessage;
										pxConnection->getIncomingBuffer(strMessage);
										pxConnection->clearReceiveBuffer();

										TcpConnection::SMessageHeader xDebugMessageHeader;
										TcpConnection::readIDHeader(strMessage, xDebugMessageHeader, false);
//										fprintf(stdout,
//										"%s : %d , received message of %d bytes: \"%s\" : header: %d, %d, %d.\n",
//										__FILE__, __LINE__, strMessage.size() + 1, strMessage.c_str(), xDebugMessageHeader.uiSender, xDebugMessageHeader.uiReceiver, xDebugMessageHeader.uiMessageId);
//										fflush(stdout);

//										if(pxConnection->isIncomingResponse(strMessage))
//										{
//											pxConnection->putIncomingResponse(strMessage);
//										}
//										else
//										{
											SRequest xRequest;
											xRequest.uiConnectionId = pxConnection->m_uiConnectionId;
											xRequest.strRequest = strMessage;

//											if(pxThis->m_bContextSwitch && pxThis->m_pxCallback)
//											{
												pthread_mutex_unlock( &pxThis->m_xMutex);
												pxThis->handleRequestContextSwitch(xRequest);
												pthread_mutex_lock( &pxThis->m_xMutex);
//											}
//											else
//											{
//												pthread_t xThread;
//												// its a request
//												pthread_attr_t attr; // thread attribute
//												// set thread detachstate attribute to DETACHED
//												pthread_attr_init( &attr);
//												pthread_attr_setdetachstate( &attr,
//												PTHREAD_CREATE_DETACHED);
//
//												// create the thread
//												pthread_create( &xThread, &attr,
//												    &TcpServer::handleRequestAsynch, pxRequest);
//											}
//										}
									}
								}
							}
						}
					}
					else if(FD_ISSET(i, &fdSetWrite))
					{
						int iSent = mapConnections[i]->sendComplete();
						if(iSent == -1)
						{
							iErrNo = errno;
							unsigned int uiId = mapConnections[i]->m_uiConnectionId;
							pthread_mutex_unlock( &pxThis->m_xMutex);
							pxThis->m_xCallback.rpcNotifyError(uiId, strerror(iErrNo));
							pthread_mutex_lock( &pxThis->m_xMutex);
							break;
						}
						if(iSent == 0)
						{
							// message sent successfully

//							fprintf(stdout,
//							"%s : %d , sent message of %d bytes: \"%s\".\n",
//							__FILE__, __LINE__, iTotal, strMessage.c_str());
//							fflush(stdout);
						}
						else
						{
							unsigned int uiId = mapConnections[i]->m_uiConnectionId;
							pthread_mutex_unlock( &pxThis->m_xMutex);
							pxThis->m_xCallback.rpcNotifyError(
							    uiId,
							    "Could not send complete message.\n");
							pthread_mutex_lock( &pxThis->m_xMutex);
						}
					}
				}
				pthread_mutex_unlock( &pxThis->m_xMutex);
			}
		}
		// cleanup
		close(iListener);
		iListener = -1;
	}
	pxThis->m_bListening = false;
	return 0;
}

//void* TcpServer::handleRequestAsynch(void* pxRequest)
//{
//	fprintf(stdout, "%s : %d\n", __FILE__, __LINE__);
//	fflush(stdout);
//	SRequest* pxReq = static_cast<SRequest*>(pxRequest);
//	IClientConnectionHandler* xHandler = pxReq->pxServer->GetHandler();
//	TcpConnection::SMessageHeader xMessageHeader;
//	string strResponse;
//	TcpConnection::readIDHeader(pxReq->strRequest, xMessageHeader);
//	xHandler->HandleRequest(pxReq->strRequest, strResponse);
//	if(strResponse.size() > 0)
//	{
//		pxReq->pxServer->SendResponse(strResponse, getConnectionById(pxRequest->uiConnectionId), xMessageHeader);
//	}
//	fprintf(stdout, "%s : %d\n", __FILE__, __LINE__);
//	fflush(stdout);
//	delete pxReq;
//	return 0;
//}

void TcpServer::handleRequestContextSwitch(SRequest& xRequest)
{
	Json::Reader xJsonReader;
	Json::Value xJsonRequest;
	string strMethod;
	TcpConnection::SMessageHeader xMessageHeader;
	TcpConnection::readIDHeader(xRequest.strRequest, xMessageHeader);

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
		m_xCallback.rpcRequestReceived(xRequest.uiConnectionId, strMethod, xJsonRequest[KEY_REQUEST_PARAMETERS]);
	}
}

void TcpServer::processRequest(unsigned int uiConnectionId)
{
	SRequest xRequest;
	if(hasQueuedRequests(uiConnectionId))
	{
		do
		{
			getQueuedRequest(xRequest, uiConnectionId);
			TcpConnection::SMessageHeader xMessageHeader;
			string strResponse;
			TcpConnection::readIDHeader(xRequest.strRequest, xMessageHeader);
			GetHandler()->HandleRequest(xRequest.strRequest, strResponse);
		}
		while(uiConnectionId != 0 && hasQueuedRequests(uiConnectionId));
	}
}


bool TcpServer::hasQueuedRequests(unsigned int uiConnectionId)
{
	pthread_mutex_lock( &m_xRequestQueueMutex);
	bool bRet = false;
	if(uiConnectionId == 0)
	{
		bRet = !m_vecRequestQueue.empty();
	}
	else
	{
		if(!m_vecRequestQueue.empty())
		{
			for(int iIdx = 0; iIdx < m_vecRequestQueue.size(); ++iIdx)
			{
				if(m_vecRequestQueue[iIdx].uiConnectionId == uiConnectionId)
				{
					bRet = true;
					break;
				}
			}
		}
	}
	pthread_mutex_unlock( &m_xRequestQueueMutex);
	return bRet;
}


bool TcpServer::getQueuedRequest(SRequest& xRequest, unsigned int uiConnectionId)
{
	pthread_mutex_lock( &m_xRequestQueueMutex);
	bool bRet = false;
	if(!m_vecRequestQueue.empty())
	{
		if(uiConnectionId == 0)
		{
			xRequest = m_vecRequestQueue.front();
			m_vecRequestQueue.erase(m_vecRequestQueue.begin());
			bRet = true;
		}
		else
		{
			vector<SRequest>::iterator xIt = m_vecRequestQueue.begin();
			while(xIt != m_vecRequestQueue.end())
			{
				if((*xIt).uiConnectionId == uiConnectionId)
				{
					xRequest = *xIt;
					bRet = true;
					break;
				}
				++xIt;
			}
			if(xIt != m_vecRequestQueue.end())
			{
				m_vecRequestQueue.erase(xIt);
			}
			else
			{
//				fprintf(stdout, "%s : %d; no request for given ConnectionId in queue!?!?!\n", __FILE__, __LINE__);fflush(stdout);
			}
		}
//		fprintf(stdout, "%s : %d; popped queued request: \"%s\"\n", __FILE__, __LINE__, xRequest.strRequest.c_str());fflush(stdout);
//		fprintf(stdout, "%s : %d; queue size: %u\n", __FILE__, __LINE__, m_vecRequestQueue.size());fflush(stdout);
	}
	else
	{
//		fprintf(stdout, "%s : %d; no request in queue!?!?!\n", __FILE__, __LINE__);fflush(stdout);
	}
	pthread_mutex_unlock( &m_xRequestQueueMutex);
	return bRet;
}

void TcpServer::queueRequest(SRequest& xRequest)
{
	pthread_mutex_lock( &m_xRequestQueueMutex);
	m_vecRequestQueue.push_back(xRequest);
//	fprintf(stdout, "%s : %d; queued request: \"%s\"\n", __FILE__, __LINE__, xRequest.strRequest.c_str());fflush(stdout);
//	fprintf(stdout, "%s : %d; queue size: %u\n", __FILE__, __LINE__, m_vecRequestQueue.size());fflush(stdout);
	pthread_mutex_unlock( &m_xRequestQueueMutex);
}


void TcpServer::eraseQueuedRequests(unsigned int uiConnectionId)
{
	pthread_mutex_lock( &m_xRequestQueueMutex);
	vector<SRequest>::iterator xIt = m_vecRequestQueue.begin();
	while(xIt != m_vecRequestQueue.end())
	{
		if((*xIt).uiConnectionId == uiConnectionId)
		{
			xIt = m_vecRequestQueue.erase(xIt);
		}
		else
		{
			++xIt;
		}
	}
	pthread_mutex_unlock( &m_xRequestQueueMutex);
}

void TcpServer::wakeup()
{
	if(m_xListenerThread > 0)
	{
		int iRet = pthread_kill(m_xListenerThread, SIG_WAKEUP);
//		fprintf(stdout, "%s : %d; sent wakeup call, iRet = %d\n", __FILE__, __LINE__, iRet);fflush(stdout);
	}
}

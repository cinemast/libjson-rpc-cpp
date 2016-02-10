/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    bdhttpserver.cpp
 * @date    11.12.2015
 * @author  Sascha Matti <sascha_matti@bluewin.ch>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "tcpconnection.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>


using namespace jsonrpc;
using namespace std;

const char TcpConnection::s_cHeaderDelim = 23U; // ETB (end of transmission block)

TcpConnection::TcpConnection()
	: m_iSocket(0),
	  m_uiConnectionId(0),
	  m_uiForeignId(0),
	  m_bConnectionSetUp(true),
	  m_uiMessageIdGen(1)
{
	pthread_mutex_init( &m_xBufferMutex, NULL);
}

TcpConnection::TcpConnection(const std::string& url)
	: m_iSocket( -1),
	  m_uiConnectionId(0),
	  m_uiForeignId(0),
	  m_strIpAddress(url),
	  m_bConnectionSetUp(false),
	  m_uiMessageIdGen(1)
{
	pthread_mutex_init( &m_xBufferMutex, NULL);
}

TcpConnection::~TcpConnection()
{
	pthread_mutex_destroy( &m_xBufferMutex);
}

void TcpConnection::init(unsigned int uiConnectionId, unsigned int uiForeignId, int iSocket, string strRemoteIp)
{
	m_uiConnectionId = uiConnectionId;
	m_uiForeignId = uiForeignId;
	m_iSocket = iSocket;
	m_strIpAddress = strRemoteIp;
}

// for char* to uint
void TcpConnection::prependIDHeader(string& strMessage, SMessageHeader xMessageHeader)
{
	strMessage.insert(strMessage.begin(), s_cHeaderDelim);
	do
	{ //Move back, inserting digits as u go
		strMessage.insert(strMessage.begin(), ((xMessageHeader.uiMessageId % 10U) + 1U));
		xMessageHeader.uiMessageId = xMessageHeader.uiMessageId / 10U;
	}
	while(xMessageHeader.uiMessageId);

	strMessage.insert(strMessage.begin(), s_cHeaderDelim);
	do
	{ //Move back, inserting digits as u go
		strMessage.insert(strMessage.begin(), ((xMessageHeader.uiReceiver % 10U) + 1U));
		xMessageHeader.uiReceiver = xMessageHeader.uiReceiver / 10U;
	}
	while(xMessageHeader.uiReceiver);

	strMessage.insert(strMessage.begin(), s_cHeaderDelim);
	do
	{ //Move back, inserting digits as u go
		strMessage.insert(strMessage.begin(), ((xMessageHeader.uiSender % 10U) + 1U));
		xMessageHeader.uiSender = xMessageHeader.uiSender / 10U;
	}
	while(xMessageHeader.uiSender);
}

// for uint to char*
void TcpConnection::readIDHeader(string& strMessage, SMessageHeader& xMessageHeader, bool bPop)
{
	xMessageHeader.uiSender = 0U;
	string::iterator itMessage = strMessage.begin();
	while( *itMessage != s_cHeaderDelim)
	{
		xMessageHeader.uiSender = (xMessageHeader.uiSender * 10U) + (( *itMessage) - 1U);
		++itMessage;
	}

	++itMessage;

	xMessageHeader.uiReceiver = 0U;
	while( *itMessage != s_cHeaderDelim)
	{
		xMessageHeader.uiReceiver = (xMessageHeader.uiReceiver * 10U) + (( *itMessage) - 1U);
		++itMessage;
	}

	++itMessage;


	xMessageHeader.uiMessageId = 0U;
	while( *itMessage != s_cHeaderDelim)
	{
		xMessageHeader.uiMessageId = (xMessageHeader.uiMessageId * 10U) + (( *itMessage) - 1U);
		++itMessage;
	}

	++itMessage;

	if(bPop)
	{
		strMessage.erase(strMessage.begin(), itMessage);
	}
}

unsigned int TcpConnection::putRequest(const std::string strRequest)
{
	pthread_mutex_lock( &m_xBufferMutex);
	unsigned int uiMessageId = m_uiMessageIdGen++;

//	MessageBuffer* pxCmdBuff = new MessageBuffer(uiMessageId);
	SMessageHeader xMessageHeader;
	xMessageHeader.uiSender = m_uiConnectionId;
	xMessageHeader.uiReceiver = m_uiForeignId;
	xMessageHeader.uiMessageId = uiMessageId;

	m_vecOutgoing.push_back(strRequest);
	prependIDHeader(m_vecOutgoing.back(), xMessageHeader);
//	m_vecBuffer.push_back(pxCmdBuff);

	pthread_mutex_unlock( &m_xBufferMutex);
	return uiMessageId;
}

const string TcpConnection::getOutgoing()
{
	fflush(stdout);
	pthread_mutex_lock( &m_xBufferMutex);
	fflush(stdout);
	string strOutgoing = m_vecOutgoing.front();
	m_vecOutgoing.erase(m_vecOutgoing.begin());
	pthread_mutex_unlock( &m_xBufferMutex);
	fflush(stdout);
	return strOutgoing;
}

//bool TcpConnection::isIncomingResponse(std::string& strMessage)
//{
//	fprintf(stdout, "%s : %d\n", __FILE__, __LINE__); fflush(stdout);
//	unsigned int uiSender = 0U;
//	string::iterator itMessage = strMessage.begin();
//	while( *itMessage != s_cHeaderDelim)
//	{
//		uiSender = (uiSender * 10U) + (( *itMessage) - 1U);
//		++itMessage;
//	}
//	bool bRet = (uiSender == m_uiConnectionId);
//	if(!bRet)
//	{
//		fprintf(stdout, "%s : %d. Got message from %d (I am %d)\n", __FILE__, __LINE__, uiSender, m_uiConnectionId); fflush(stdout);
//	}
//	return bRet;
//}

//void TcpConnection::putIncomingResponse(std::string& strResponse)
//{
//	fprintf(stdout, "%s : %d\n", __FILE__, __LINE__); fflush(stdout);
//	SMessageHeader xMessageHeader;
//	readIDHeader(strResponse, xMessageHeader);
//	if(xMessageHeader.uiSender == m_uiConnectionId)
//	{
//		pthread_mutex_lock( &m_xBufferMutex);
//		size_t uiIdx = 0;
//		for(; uiIdx < m_vecBuffer.size() && m_vecBuffer[uiIdx]->m_uiMessageId != xMessageHeader.uiMessageId;
//		    ++uiIdx)
//		{
//
//		}
//		if(uiIdx < m_vecBuffer.size())
//		{
//			pthread_mutex_lock( &(m_vecBuffer[uiIdx]->m_xMessageMutex));
//			m_vecBuffer[uiIdx]->m_strResponse = strResponse;
//			pthread_cond_signal( &(m_vecBuffer[uiIdx]->m_xMessageSignal));
//			pthread_mutex_unlock( &(m_vecBuffer[uiIdx]->m_xMessageMutex));
//			pthread_mutex_unlock( &m_xBufferMutex);
//			return;
//		}
//		pthread_mutex_unlock( &m_xBufferMutex);
//	}
//	fprintf(stdout, "%s : %d , %s\n", __FILE__, __LINE__,
//	    "Unable to match Response with open Requests.");
//	fflush(stdout);
//}

bool TcpConnection::hasPendingMessages() const
{
	return !m_vecOutgoing.empty();
}

void TcpConnection::putOutgoingResponse(std::string strResponse, SMessageHeader xMessageHeader)
{
	prependIDHeader(strResponse, xMessageHeader);
	pthread_mutex_lock( &m_xBufferMutex);
	m_vecOutgoing.push_back(strResponse);
	pthread_mutex_unlock( &m_xBufferMutex);
}

//void TcpConnection::getResponseBlocking(unsigned int uiMessageId, string& strResponse)
//{
//	pthread_mutex_lock( &m_xBufferMutex);
//	size_t uiIdx = 0;
//	for(; uiIdx < m_vecBuffer.size() && m_vecBuffer[uiIdx]->m_uiMessageId != uiMessageId; ++uiIdx)
//	{
//
//	}
//	if(uiIdx < m_vecBuffer.size())
//	{
//		pthread_mutex_unlock( &m_xBufferMutex);
//		pthread_mutex_lock( &(m_vecBuffer[uiIdx]->m_xMessageMutex));
//		pthread_cond_wait( &(m_vecBuffer[uiIdx]->m_xMessageSignal),
//		    &(m_vecBuffer[uiIdx]->m_xMessageMutex));
//		strResponse = m_vecBuffer[uiIdx]->m_strResponse;
//		pthread_mutex_unlock( &(m_vecBuffer[uiIdx]->m_xMessageMutex));
//		return;
//	}
//	else
//	{
//		pthread_mutex_unlock( &m_xBufferMutex);
//		fprintf(stdout, "%s : %d , %s\n", __FILE__, __LINE__,
//		    "Unable to find Response by MessageID");
//		fflush(stdout);
//		return;
//	}
//}

void TcpConnection::initIdentification(SMessageHeader& xMessageHeader)
{
	if(!m_bConnectionSetUp)
	{
		m_uiConnectionId = xMessageHeader.uiReceiver;
		m_uiForeignId =  xMessageHeader.uiSender;
		m_bConnectionSetUp = true;
	}
}

void TcpConnection::clearReceiveBuffer()
{
	pthread_mutex_lock( &m_xBufferMutex);
	m_strReceiveBuffer.clear();
	pthread_mutex_unlock( &m_xBufferMutex);
}

void TcpConnection::appendIncoming(char* szSource, int iBytes)
{
	pthread_mutex_lock( &m_xBufferMutex);
	m_strReceiveBuffer.append(szSource, iBytes);
	pthread_mutex_unlock( &m_xBufferMutex);
}

void TcpConnection::getIncomingBuffer(string& strMessage)
{
	pthread_mutex_lock( &m_xBufferMutex);
	strMessage.assign(m_strReceiveBuffer);
	pthread_mutex_unlock( &m_xBufferMutex);
}

int TcpConnection::sendComplete()
{
	string strMessage = getOutgoing();
	int iRet = 0;
	int iTotal = strlen(strMessage.data()) + 1; // +1 for terminating 0x0
	int iSent = 0; // how many bytes we've sent
	int iLeft = iTotal; // how many we have left to send
	int n;
	while(iSent < iTotal)
	{
		n = send(m_iSocket, strMessage.data() + iSent, iLeft, 0);
		if(n == -1)
		{
			iRet = -1;
			break;
		}
		iSent += n;
		iLeft -= n;
		iRet = iLeft;
	}
	return iRet;
}

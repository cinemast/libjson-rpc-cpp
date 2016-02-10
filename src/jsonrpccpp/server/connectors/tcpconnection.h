/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    bdhttpserver.h
 * @date    11.12.2015
 * @author  Sascha Matti <sascha_matti@bluewin.ch>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_TCPCONNECTION_H_
#define JSONRPC_CPP_TCPCONNECTION_H_

#include <stdlib.h>
#include <string>
#include <pthread.h>
#include <map>
#include <vector>
#include <stdio.h>

namespace jsonrpc
{

class TcpConnection
{
public:
	struct SMessageHeader
	{
		unsigned int uiSender;
		unsigned int uiReceiver;
		unsigned int uiMessageId;
	};

//	class MessageBuffer
//	{
//	public:
//		MessageBuffer(unsigned int uiId)
//			: m_uiMessageId(uiId)
//		{
//			pthread_cond_init( &m_xMessageSignal, NULL);
//			pthread_mutex_init( &(m_xMessageMutex), NULL);
//		}
//
//		~MessageBuffer()
//		{
//			pthread_cond_destroy( &m_xMessageSignal);
//			pthread_mutex_destroy( &m_xMessageMutex);
//		}
//
//		pthread_mutex_t m_xMessageMutex;
//		pthread_cond_t m_xMessageSignal;
//		unsigned int m_uiMessageId;
//		std::string m_strResponse;
//	};

	TcpConnection();

	virtual ~TcpConnection();

	void init(unsigned int uiConnectionId, unsigned int uiForeignId, int iSocket,
	          std::string strRemoteIp);

	static void prependIDHeader(std::string& strMessage, SMessageHeader xMessageHeader);

	static void readIDHeader(std::string& strMessage, SMessageHeader& xMessageHeader, bool bPop =
	                             true);

	unsigned int putRequest(const std::string strRequest);

	const std::string getOutgoing();

	int sendComplete();

//	void getResponseBlocking(unsigned int uiMessageId, std::string& strResponse);

//	bool isIncomingResponse(std::string& strMessage);

//	void putIncomingResponse(std::string& strResponse);

	bool hasPendingMessages() const;

	void putOutgoingResponse(std::string strResponse, SMessageHeader xMessageHeader);

	void clearReceiveBuffer();

	void appendIncoming(char* szSource, int iBytes);

	void getIncomingBuffer(std::string& strMessage);

	const int& getSocket() const
	{
		return m_iSocket;
	}
	;

	int m_iSocket;
	unsigned int m_uiConnectionId;
	unsigned int m_uiForeignId;
	std::string m_strReceiveBuffer;
	std::vector<std::string> m_vecIncoming;
	std::vector<std::string> m_vecOutgoing;

protected:

	TcpConnection(const std::string& url);

	void initIdentification(SMessageHeader& xMessageHeader);

	std::string m_strIpAddress;
	bool m_bConnectionSetUp;

	pthread_mutex_t m_xBufferMutex;
//	std::vector<MessageBuffer*> m_vecBuffer;
	unsigned int m_uiMessageIdGen;
	static const char s_cHeaderDelim;

};

}

#endif

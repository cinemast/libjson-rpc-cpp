/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    bdhttpclient.h
 * @date    11.12.2015
 * @author  Sascha Matti <sascha_matti@bluewin.ch>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_TCPCLIENT_H_
#define JSONRPC_CPP_TCPCLIENT_H_

#include <jsonrpccpp/common/iclientconnector.h>
#include "../../server/abstractserverconnector.h"
#include "../../server/connectors/tcpconnection.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <map>
#include <vector>
#include <cerrno>

namespace jsonrpc
{

class TcpClientCallback
{
public:
	/**
	 * The virtual destructor
	 */
	virtual ~TcpClientCallback()
	{
	}

	/**
	 * Notifies the connection state from the web application to the driver.
	 * @param bConnected True if a connection has been established, false if lost
	 * @warning This method runs in the context of the listener thread!
	 */
	virtual void rpcConnected(const bool bConnected) = 0;

	/**
	 * Error notification on a socket error. Query the error code (if required)
	 * directly with CRpcConnector::getLastError*(). If an error leads to a
	 * lost connection, rpcConnected() will be called separately.
	 * @param strError The human readable error message.
	 * @warning This method runs in the context of the listener thread!
	 */
	virtual void rpcNotifyError(const std::string& strError) = 0;

		/**
	 * Notifies callback client of received rpcrequest if context-switch is enabled.
	 * rpc request is stored by connection id and can be processed
	 * by calling processRequest(unsigned int uiConnectionId)
	 * by processing thread of callbacks choosing.
	 * @param uiConnectionId   id of connection which received rpcrequest
	 */
	virtual void rpcRequestReceived(const std::string& strMethod, const Json::Value& xRequest) = 0;

};


class TcpClient: public IClientConnector, public AbstractServerConnector, public TcpConnection
{
public:
	TcpClient(const std::string& url, unsigned short usPort, TcpClientCallback& xCallback, unsigned int uiSelectTimeout = 1000);
	virtual ~TcpClient();

	virtual void SendRPCMessage(const std::string& message, std::string& result)
	    throw(JsonRpcException);

	void SetTimeout(long timeout);

	bool StartPersistentListening();

	void StopPersistentListening();

	virtual bool StartListening();
	virtual bool StopListening();

	void setContextSwitch(bool bEnabled);

	bool virtual SendResponse(const std::string& response, void* addInfo, SMessageHeader xMessageHeader);

	bool virtual SendResponse(const std::string& response, void* addInfo = NULL) {return SendResponse(response, addInfo, {0,0,0});}

	void processRequest();

private:

	struct SRequest
	{
		unsigned int uiConnectionId;
		std::string strRequest;
	};

	void handleRequestContextSwitch(SRequest& pxRequest);

	void initConnection();

	void closeConnection();

	/* listener loop */
	static void* listenerLoop(void*);

	static void* persistentListeningLoop(void* pxTcpClient);

//	static void* handleRequestAsynch(void* xRequest);

	void getQueuedRequest(SRequest& pxRequest);

	void queueRequest(SRequest& pxRequest);

	void wakeup();

	bool m_bConnected;

	unsigned short m_usPort;

	/* listener thread */
	pthread_t m_xListenerThread;

	/* persistence thread */
	pthread_t m_xPersistenceThread;

	/* mutex to protect map of connections */
	pthread_mutex_t m_xMapMutex;

	unsigned int m_uiSelectTimeout;

	/* if we are listening */
	bool m_bListening;

	/* callback for connection and error report */
	TcpClientCallback& m_xCallback;

	bool m_bPersistentListening;

	std::vector<SRequest> m_vecRequestQueue;
	pthread_mutex_t m_xRequestQueueMutex;
	pthread_cond_t m_xRequestQueueSignal;
};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_TCPCLIENT_H_ */

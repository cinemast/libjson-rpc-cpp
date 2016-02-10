/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    bdhttpserver.h
 * @date    11.12.2015
 * @author  Sascha Matti <sascha_matti@bluewin.ch>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_TCPSERVER_H_
#define JSONRPC_CPP_TCPSERVER_H_

#include "../abstractserverconnector.h"
#include "../iserverconnector.h"
#include <jsonrpccpp/common/iclientconnector.h>
#include "tcpconnection.h"

#include <string>
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

class TcpServerCallback
{
public:
	/**
	 * The virtual destructor
	 */
	virtual ~TcpServerCallback()
	{
	}

	/**
	 * Called to ask the callback if it accepts the new connection.
	 * @param xId Id of the listener with a new connection
	 * @param xPeerAddres IP address and port of the peer which wants to connect
	 * @return True if the callback calls successfully acceptConnection within this method, false else.
	 * @exception
	 */
	virtual bool newConnection(unsigned int uiConnectionId, const std::string& strPeerAddress) = 0;

	/**
	 * This method is called whenever an error occurs on a tcp connection.
	 * @param xId Connection id
	 * @param xError CSocketError object with information about the error
	 * @exception
	 */
	virtual void rpcNotifyError(unsigned int uiConnectionId, const std::string& strError) = 0;

	/**
	 * Notifies callback client of received rpcrequest if context-switch is enabled.
	 * rpc request is stored by connection id and can be processed
	 * by calling processRequest(unsigned int uiConnectionId)
	 * by processing thread of callbacks choosing.
	 * @param uiConnectionId   id of connection which received rpcrequest
	 */
	virtual void rpcRequestReceived(unsigned int uiConnectionId, const std::string& strMethod, const Json::Value& xRequest) = 0;
};

/**
 * This class provides a bidirectional HTTP Server, based on libc, to handle incoming Requests and send HTTP 1.1
 * valid responses. It provides the ability to send tpc messages to connected clients.
 * Note that this class will always send HTTP-Status 200, even though an JSON-RPC Error might have occurred. Please
 * always check for the JSON-RPC Error Header.
 */
class TcpServer: public AbstractServerConnector,
                 public IServerConnector
{
public:

	/**
	 * @brief HttpServer, constructor for the included HttpServer
	 * @param port on which the server is listening
	 */
	TcpServer(unsigned short usPort, TcpServerCallback& xCallback, unsigned int uiSelectTimeout = 1000);

	virtual ~TcpServer();

	/*
	 * starts the listening loop
	 */
	virtual bool StartListening();

	/*
	 * stops the listening loop
	 */
	virtual bool StopListening();

	/*
	 * Sends a response to a rpcrequest to a given connection
	 * @param strResponse     string representation of the response to send
	 * @param pxConnection    pointer to connection to send the response to
	 * @param xMessageHeader  message header of response
	 */
	virtual bool SendResponse(const std::string& strResponse, void* pxConnection, TcpConnection::SMessageHeader xMessageHeader);

	/*
	 * Sends a response to a rpcrequest to a given connection.
	 * This method will call SendResponse with messageheader 0, 0, 0
	 * @param strResponse     string representation of the response to send
	 * @param pxConnection    pointer to connection to send the response to
	 */
	bool virtual SendResponse(const std::string& strResponse, void* pxConnection = NULL)
	{
		return SendResponse(strResponse, pxConnection, {0, 0, 0});
	}

	bool virtual SendOptionsResponse(void* addInfo);

	void SetUrlHandler(const std::string &url, IClientConnectionHandler *handler);

	/*
	 * Sends an rpc request to the connection given by its id.
	 * @param strMessage       string representation of rpcrequest to send
	 * @param strResult        reference of string to parse the answer of the rpcrequest
	 * @param uiConnectionId   id of connection to send request to
	 */
	virtual void SendRPCMessage(const std::string& strMessage, std::string& strResult,
	                            unsigned int uiConnectionId = 0) throw(JsonRpcException);

	/*
	 * Broadcasts an rpc request to all connections.
	 * @param strMessage       string representation of rpcrequest to send
	 * @param strResult        reference of string to parse the answer of the rpcrequest
	 */
	virtual void SendRPCMessage(const std::string& message, std::string& result)
	    throw(JsonRpcException)
	{
		return SendRPCMessage(message, result, 0);
	};

	/*
	 * Processes all currently queued rpcrequests.
	 * @param uiConnectionId    Id of connection to process requests from,
	 *                          if 0, all requests of all connections will pe processed
	 */
	void processRequest(unsigned int uiConnectionId = 0);

	/*
	 * Returns whether a given connection has queued requests.
	 * @param uiConnectionId   Id of the connection to check if rpcrequests are queued.
	 *                         If 0, checks if any connection has a queued request.
	 * @return                 True if the given connection has at least one queued request,
	 *                         false otherwise
	 */
	bool hasQueuedRequests(unsigned int uiConnectionId = 0);

private:
	/*
	 * struct representing a request, used to queue requests
	 */
	struct SRequest
	{
		/* id of connection the request belongs to */
		unsigned int uiConnectionId;
		/* string representatino of the request, including its message header */
		std::string strRequest;
	};

	void handleRequestContextSwitch(SRequest& xRequest);

	/* listener loop */
	static void* listenerLoop(void*);

	bool getQueuedRequest(SRequest& xRequest, unsigned int uiConnectionId = 0);

	void queueRequest(SRequest& xRequest);

	void eraseQueuedRequests(unsigned int uiConnectionId);

	void wakeup();

	/* if we are listening */
	bool m_bListening;
	/* port this server listens on */
	unsigned short m_usPort;
	/* mutex to protect map of connections */
	pthread_mutex_t m_xMutex;
	/* next connection id to give*/
	unsigned int m_uiConnectionIdGen;
	/* map of connections */
	std::map<unsigned int, TcpConnection> m_mapConnections;
	/* listener thread */
	pthread_t m_xListenerThread;
	/* select timout */
	unsigned int m_uiSelectTimeout;
	/* server id */
	unsigned int m_uiServerId;

	/** callback of tcpserver */
	TcpServerCallback& m_xCallback;

	std::vector<SRequest> m_vecRequestQueue;
	pthread_mutex_t m_xRequestQueueMutex;
	pthread_cond_t m_xRequestQueueSignal;
};

}
/* namespace jsonrpc */

#endif /* JSONRPC_CPP_BIDIRECTIONALHTTPSERVER_H_ */

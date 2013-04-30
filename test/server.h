/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file server.h
 * @date    08.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <jsonrpc/rpc.h>

//Methods
void sayHello(const Json::Value& request, Json::Value& response);
void getCounterValue(const Json::Value& request, Json::Value& response);
void add(const Json::Value& request, Json::Value& response);
void sub(const Json::Value& request, Json::Value& response);

//Notifications
void initCounter(const Json::Value& request);
void incrementCounter(const Json::Value& request);

jsonrpc::methodpointer_t getMethodPointer();
jsonrpc::notificationpointer_t getNotificationPointer();

jsonrpc::AbstractServer* getTestServer();
jsonrpc::Client* getTestClient();

#endif // SERVER_H

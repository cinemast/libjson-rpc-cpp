/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    redisclient.cpp
 * @date    12.08.2017
 * @author  Jacques Software <software@jacques.com.au>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "redisclient.h"

#include <string>

using namespace jsonrpc;


RedisClient::RedisClient(const std::string& host, int port, const std::string& queue) throw(JsonRpcException)
    : queue(queue)
{
    this->timeout = 1;
}


RedisClient::~RedisClient()
{
}


void RedisClient::SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException)
{
    throw JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR, "Not implemented");
}


void RedisClient::SetQueue(const std::string& queue)
{
    this->queue = queue;
}


void RedisClient::SetTimeout(long timeout)
{
    this->timeout = timeout;
}

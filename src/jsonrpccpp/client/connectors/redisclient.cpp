/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    redisclient.cpp
 * @date    12.08.2017
 * @author  Jacques Software <software@jacques.com.au>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "redisclient.h"

#include <iostream>
#include <string>
#include <stdlib.h>

using namespace jsonrpc;


/**
 * Generate a random string.
 * @param s Buffer for the random string.
 * @param len The length of the random string.
 */
void genRandom(char *s, int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}



RedisClient::RedisClient(const std::string& host, int port, const std::string& queue) throw(JsonRpcException)
    : queue(queue)
{
    this->timeout = 1;

    con = redisConnect(host.c_str(), port);
    if (con == NULL) {
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "redis error: Failed to connect");
    }

    if (con->err) {
        std::stringstream err;
        err << "redis error: " << con->err;
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, err.str());
    }

    char id[17];
    std::stringstream str;
    genRandom(id, 16);
    str << queue << "_" << id;
    this->ret_queue = str.str();

    redisReply* reply = (redisReply*) redisCommand(con, "EXISTS %s", ret_queue.c_str());
    if (reply == NULL) {
        freeReplyObject(reply);
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "redis error: Failed to run queue check");
    }
    if (reply->type != REDIS_REPLY_INTEGER || reply->integer != 0) {
        std::stringstream err;
        err << "redis error: queue '" << ret_queue << "' already exists";
        freeReplyObject(reply);
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, err.str());
    }
    freeReplyObject(reply);
}


RedisClient::~RedisClient()
{
    redisFree(con);
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

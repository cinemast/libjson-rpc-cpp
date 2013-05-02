/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    abstractrequesthandler.h
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef ABSTRACTREQUESTHANDLER_H
#define ABSTRACTREQUESTHANDLER_H

namespace jsonrpc
{
    class AbstractRequestHandler {
        public:
            virtual void handleMethodCall(Procedure* proc, const Json::Value& input, Json::Value& output) = 0;
            virtual void handleNotificationCall(Procedure* proc, const Json::Value& input) = 0;
    };
}

#endif // ABSTRACTREQUESTHANDLER_H

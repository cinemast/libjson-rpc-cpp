/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    zeromqclient.h
 * @date    17.11.2015
 * @author  Badaev Vladimir <dead.skif@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/
#ifndef JSONRPC_CPP_ZEROMQCLIENT_H_

#define JSONRPC_CPP_ZEROMQCLIENT_H_
#include <string>
#include <zmq.hpp>

#include "../iclientconnector.h"
#include <jsonrpccpp/common/exception.h>

namespace jsonrpc {
    class ZeroMQClient : public IClientConnector
    {
        public:
    		ZeroMQClient(const std::string& endpoint);
            virtual ~ZeroMQClient();
            virtual void SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException);


        private:
            zmq::context_t ctx;
            zmq::socket_t sock;
    };
    
} /* jsonrpc */


#endif /* end of include guard: JSONRPC_CPP_ZEROMQCLIENT_H_ */

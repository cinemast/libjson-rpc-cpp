/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file
 * @date    7/10/2015
 * @author  Peter Spiess-Knafl <dev@spiessknafl.at>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef ZMQSERVER_H
#define ZMQSERVER_H

#include <zmq.hpp>
#include "../abstractserverconnector.h"

namespace jsonrpc
{
    struct zmqserverImp;

    class ZMQServer : public AbstractServerConnector
    {
        public:
            ZMQServer(const std::string &address = "tcp://*:8080");

            virtual bool StartListening();
            virtual bool StopListening();

            bool virtual SendResponse(const std::string& response, void* addInfo = NULL);

        private:
            struct zmqserverImp* imp;
            zmq::context_t m_context;
            zmq::socket_t m_socket;
            std::string m_address;
    };
}
#endif // ZMQSERVER_H

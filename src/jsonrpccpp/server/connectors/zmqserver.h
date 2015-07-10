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
    class ZMQServer : public AbstractServerConnector
    {
        public:
            ZMQServer(int port);

            virtual bool StartListening();
            virtual bool StopListening();

            bool virtual SendResponse(const std::string& response, void* addInfo = NULL);

        private:
            zmq::context_t m_context;
            zmq::socket_t m_socket;
            int m_port;
    };
}
#endif // ZMQSERVER_H

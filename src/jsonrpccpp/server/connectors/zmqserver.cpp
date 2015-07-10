/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file
 * @date    7/10/2015
 * @author  Peter Spiess-Knafl <dev@spiessknafl.at>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "zmqserver.h"
#include <sstream>

using namespace jsonrpc;
using namespace std;

ZMQServer::ZMQServer(int port) :
    m_context (1),
    m_socket (m_context, ZMQ_REP),
    m_port(port)
{
}


bool ZMQServer::StartListening()
{
    try {
        stringstream str;
        str << "tcp://*:" << m_port;
        m_socket.bind(str.str().c_str());
        return true;
    }
    catch (zmq::error_t& error)
    {
        (void)(error);
        return false;
    }
}

bool ZMQServer::StopListening()
{
    stringstream str;
    str << "tcp://*:" << m_port;
    m_socket.unbind(str.str().c_str());
}

bool ZMQServer::SendResponse(const std::string &response, void *addInfo)
{
    m_socket.send(response.c_str(), response.length()+1);
}

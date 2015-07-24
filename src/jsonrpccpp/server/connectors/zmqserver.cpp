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

ZMQServer::ZMQServer(const string &address) :
    m_context (1),
    m_socket (m_context, ZMQ_REP),
    m_address(address)
{
}


bool ZMQServer::StartListening()
{
    try {
        m_socket.bind(m_address.c_str());
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
    m_socket.unbind(m_address.c_str());
    return true;
}

bool ZMQServer::SendResponse(const std::string &response, void *addInfo)
{
    m_socket.send(response.c_str(), response.length()+1);
}

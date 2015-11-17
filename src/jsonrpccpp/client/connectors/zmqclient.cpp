
/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    zmqclient.cpp
 * @date    17.11.2015
 * @author  Badaev Vladimir <dead.skif@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/
#include "zmqclient.h"
using namespace std;
using namespace jsonrpc;
using namespace zmq;

#define REQUEST_TIMEOUT     2500    //  msecs, (> 1000!)

ZeroMQClient::ZeroMQClient(const std::string& endpoint)
    : ctx(), sock(ctx, ZMQ_REQ)
{
    try {
        sock.connect(endpoint.c_str());
        int linger = 0;
        sock.setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
    } catch (const zmq::error_t& e) {
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "Could not connect to " + endpoint + ":" + e.what());
    }
}
ZeroMQClient::~ZeroMQClient()
{}
void ZeroMQClient::SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException)
{
    try {
        sock.send(message.c_str(), message.size());
        pollitem_t items[] = { { sock, 0, ZMQ_POLLIN, 0 } };
        poll (&items[0], 1, REQUEST_TIMEOUT);

        //  If we got a reply, process it
        if (items[0].revents & ZMQ_POLLIN) {
            message_t msg;
            sock.recv(&msg);
            string s(static_cast<const char *>(msg.data()), msg.size());
            result = s;
        } else {
            throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "Timeout");
        }
    } catch (const zmq::error_t& e) {
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, e.what());
    }
}

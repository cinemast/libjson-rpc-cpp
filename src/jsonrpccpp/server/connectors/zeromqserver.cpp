/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    zeromqserver.cpp
 * @date    15.11.2015
 * @author  Badaev Vladimir <dead.skif@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "zeromqserver.h"

using namespace jsonrpc;
using namespace std;
using namespace zmq;

namespace {
    template<typename W>
        struct wrapper {
            W& wrapped;
            wrapper(W& w) : wrapper(w) {};
        };
    typedef wrapper<socket_t> socket_wrapper;
}

template<typename Listener>
ZeroMQServer<Listener>::ZeroMQServer(const string ep)
    : ZeroMQServer(vector<const string>(1, ep))
{
}
template<typename Listener>
ZeroMQServer<Listener>::ZeroMQServer(vector<string> eps) 
    : endpoints(eps), listener(nullptr)
{
}

template<typename Listener>
bool ZeroMQServer<Listener>::StartListening()
{
    if (listener == nullptr) {
        listener = new Listener(this, endpoints);
        return true;
    } 
    return false;
}
template<typename Listener>
bool ZeroMQServer<Listener>::StopListening()
{
    if (listener != nullptr) {
        listener.reset(nullptr);
        return true;
    }
    return false;
}
template<typename Listener>
bool ZeroMQServer<Listener>::SendResponse(const string& response, void* addInfo)
{
    socket_wrapper *sw = reinterpret_cast<socket_wrapper *>(addInfo);
    socket_t& sock = sw->wrapped;
    delete sw;

    sock.send(response.c_str(), response.size());

    return true;
}

MultiThreadListener::MultiThreadListener(AbstractServerConnector *s, const EndPoints& eps, unsigned int threads_count)
    : ZeroMQListener(s, eps)
{
    /* Starting FRONTEND(ROUTER) <-> BACKEND(DEALER) proxies */
    for (size_t i = 0; i < eps.size(); i++) {
        string be = "ipc://jsonrpc_worker" + i;
        thread( [=] { ProxyThread(eps[i], be); }).detach();
        for (size_t w = 0; i < threads_count; i++) {
            worker_threads.emplace_back([=] { WorkerThread(be); });
        }
    }

}
MultiThreadListener::~MultiThreadListener()
{
    for (auto& w : worker_threads) {
        w.join();
    }
}
void MultiThreadListener::ProxyThread(const std::string& fe, const std::string be) {
    
    socket_t frontend(ctx, ZMQ_ROUTER);
    socket_t backend(ctx, ZMQ_DEALER);
    
    frontend.bind(fe.c_str());
    backend.bind(be.c_str());

    /* Works while ctx is active */
    proxy(frontend, backend, NULL);
}
void MultiThreadListener::WorkerThread(const std::string be) {
    socket_t sock(ctx, ZMQ_REP);
    sock.connect(be.c_str());

    while(1) {
        message_t msg;
        sock.recv(&msg);
        string request(reinterpret_cast<const char *>(msg.data()), msg.size());
        server->OnRequest(request, reinterpret_cast<void *>(new socket_wrapper(sock)));
    }
}


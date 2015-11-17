/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    zmqserver.cpp
 * @date    15.11.2015
 * @author  Badaev Vladimir <dead.skif@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "zmqserver.h"

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


    class ZeroMQListener {
        public:
            typedef std::vector<string> EndPoints;
            ZeroMQListener(AbstractServerConnector *s, const EndPoints& eps)
                : server(s), ctx(), endpoints(eps)
            {}
            virtual ~ZeroMQListener() {}

            virtual void doRequest(socket_t& sock);
        protected:
            AbstractServerConnector *server;
            zmq::context_t ctx;
            const EndPoints& endpoints;

    };
    class MultiThreadListener : public ZeroMQListener {
        public:
            MultiThreadListener(AbstractServerConnector *s, const EndPoints& eps, unsigned int worker_threads=1);
            virtual ~MultiThreadListener();
        private:
            void ProxyThread(const string& fe, const string be);
            void WorkerThread(const string be);
            vector<thread> worker_threads;
    };
    class OneThreadListener : public ZeroMQListener {
        public:
            OneThreadListener(AbstractServerConnector *s, const EndPoints& eps);
            virtual ~OneThreadListener(); 
        private:
            thread listening_thread;
            void WorkerThread();
    };
}


ZeroMQServer::ZeroMQServer(const string& ep, unsigned int th_count)
    : ZeroMQServer(vector<const string>(1, 
                (ep.find("://") == std::string::npos) /* If ep is just IP or '*' */
                ? "tcp://" + ep + ":8080" /* Convert it to ZeroMQ endpoint form */
                : ep), th_count)
{
}
ZeroMQServer::ZeroMQServer(vector<string> eps, unsigned int th_count) 
    : endpoints(eps), listener(nullptr), threads_count(th_count)
{
}

bool ZeroMQServer::StartListening()
{
    if (listener == nullptr) {
        if (threads_count)
            listener = new MultiThreadListener(this, endpoints, threads_count);
        else
            listener = new OneThreadListener(this, endpoints);
        return true;
    } 
    return false;
}
bool ZeroMQServer::StopListening()
{
    if (listener != nullptr) {
        listener.reset(nullptr);
        return true;
    }
    return false;
}
bool ZeroMQServer::SendResponse(const string& response, void* addInfo)
{
    socket_wrapper *sw = reinterpret_cast<socket_wrapper *>(addInfo);
    socket_t& sock = sw->wrapped;
    delete sw;

    sock.send(response.c_str(), response.size());

    return true;
}

void ZeroMQListener::doRequest(socket_t& sock)
{
    message_t msg;
    sock.recv(&msg);
    string request(reinterpret_cast<const char *>(msg.data()), msg.size());
    server->OnRequest(request, reinterpret_cast<void *>(new socket_wrapper(sock)));
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
    ctx.close();
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
    try {
        sock.connect(be.c_str());

        while(1) {
            doRequest(sock);
        }
    } catch (const zmq::error_t& e) {
        //if (e.num != ETERM)
            //throw e;
        // if ETERM -- ctx is closed, just exiting thread.

    }
}

OneThreadListener::OneThreadListener(AbstractServerConnector *s, const EndPoints& eps)
    : ZeroMQListener(s, eps)
{
    listening_thread([=] { WorkerThread() });
}
OneThreadListener::~OneThreadListener(); {
    ctx.close();
    listening_thread.join();
}
void OneThreadListener::WorkerThread()
{
    vector<socket_t> socks;
    vector<pollitem_t> items(endpoints.size());
    for(size_t i = 0; i < endpoints.size(); i++) {
        const int linger = 0;
        socks.emplace_back(ctx, ZMQ_REP);
        socks[i].setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        items[i].socket = socks[i];
        items[i].events = ZMQ_POLLIN;
        socks[i].bind(eps[i].c_str());
    }
    try {
        while(1) {
            poll(&items[0], items.size());
            for(size_t i = 0; i < items.size(); i++) {
                if (items[i].revent & ZMQ_POLLIN) {
                    doRequest(socks[i]);
                };
            };
        };
    } catch (const zmq::error_t& e) {
        // if ETERM -- ctx is closed, just exiting thread.
    }
}

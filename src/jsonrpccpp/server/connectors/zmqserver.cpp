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

class jsonrpc::ZMQListener {
    public:
        typedef std::vector<string> EndPoints;
        ZMQListener(AbstractServerConnector *s, const EndPoints& eps)
            : server(s), ctx(), endpoints(eps)
        {}
        virtual ~ZMQListener() {}

        virtual void doRequest(socket_t& sock);
    protected:
        AbstractServerConnector *server;
        zmq::context_t ctx;
        const EndPoints& endpoints;

};
namespace {
    template<typename W>
        struct wrapper {
            W& wrapped;
            wrapper(W& w) : wrapper(w) {};
        };
    typedef wrapper<socket_t> socket_wrapper;


    class MultiThreadListener : public ZMQListener {
        public:
            MultiThreadListener(AbstractServerConnector *s, const EndPoints& eps, unsigned int worker_threads=1);
            virtual ~MultiThreadListener();
        private:
            void ProxyThread(const string& fe, const string be);
            void WorkerThread(const string be);
            vector<thread> worker_threads;
    };
    class OneThreadListener : public ZMQListener {
        public:
            OneThreadListener(AbstractServerConnector *s, const EndPoints& eps);
            virtual ~OneThreadListener(); 
        private:
            thread listening_thread;
            void WorkerThread();
    };
}


ZMQServer::ZMQServer(const string& ep, unsigned int th_count)
    : ZMQServer(vector<string>(1, 
                (ep.find("://") == std::string::npos) /* If ep is just IP or '*' */
                ? "tcp://" + ep + ":8080" /* Convert it to ZeroMQ endpoint form */
                : ep), th_count)
{
}
ZMQServer::ZMQServer(vector<string> eps, unsigned int th_count) 
    : endpoints(eps), listener(nullptr), threads_count(th_count)
{
}

ZMQServer::~ZMQServer()
{
}

bool ZMQServer::StartListening()
{
    if (listener == nullptr) {
        if (threads_count)
            listener.reset(new MultiThreadListener(this, endpoints, threads_count));
        else
            listener.reset(new OneThreadListener(this, endpoints));
        return true;
    } 
    return false;
}
bool ZMQServer::StopListening()
{
    if (listener != nullptr) {
        listener.reset(nullptr);
        return true;
    }
    return false;
}
bool ZMQServer::SendResponse(const string& response, void* addInfo)
{
    socket_wrapper *sw = reinterpret_cast<socket_wrapper *>(addInfo);
    socket_t& sock = sw->wrapped;
    delete sw;

    sock.send(response.c_str(), response.size());

    return true;
}

void ZMQListener::doRequest(socket_t& sock)
{
    message_t msg;
    sock.recv(&msg);
    string request(reinterpret_cast<const char *>(msg.data()), msg.size());
    server->OnRequest(request, reinterpret_cast<void *>(new socket_wrapper(sock)));
}

MultiThreadListener::MultiThreadListener(AbstractServerConnector *s, const EndPoints& eps, unsigned int threads_count)
    : ZMQListener(s, eps)
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
    : ZMQListener(s, eps)
{
    listening_thread = thread([=] { WorkerThread(); });
}
OneThreadListener::~OneThreadListener() {
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
        socks[i].bind(endpoints[i].c_str());
    }
    try {
        while(1) {
            poll(&items[0], items.size());
            for(size_t i = 0; i < items.size(); i++) {
                if (items[i].revents & ZMQ_POLLIN) {
                    doRequest(socks[i]);
                };
            };
        };
    } catch (const zmq::error_t& e) {
        // if ETERM -- ctx is closed, just exiting thread.
    }
}

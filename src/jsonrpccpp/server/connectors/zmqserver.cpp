/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    zmqserver.cpp
 * @date    15.11.2015
 * @author  Badaev Vladimir <dead.skif@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/
#include <sstream>
//#include <iostream>
#include <atomic>

#include "zmqserver.h"

#define POLL_INTERVALL 1000

using namespace jsonrpc;
using namespace std;
using namespace zmq;

class jsonrpc::ZMQListener {
    public:
        typedef std::vector<string> EndPoints;
        ZMQListener(AbstractServerConnector *s, const EndPoints& eps)
            : server(s), ctx(), endpoints(eps), stop(false)
        {}
        virtual ~ZMQListener() {}

        virtual void DoRequest(socket_t& sock);
    protected:
        AbstractServerConnector *server;
        zmq::context_t ctx;
        const EndPoints& endpoints;
        atomic<bool> stop;

};
namespace {
    template<typename W>
        struct wrapper {
            W& wrapped;
            wrapper(W& w) : wrapped(w) {};
        };
    typedef wrapper<socket_t> socket_wrapper;


    class MultiThreadListener : public ZMQListener {
        public:
            MultiThreadListener(AbstractServerConnector *s, const EndPoints& eps, unsigned int worker_threads=1);
            virtual ~MultiThreadListener();
        private:
            void ProxyThread(unique_ptr<socket_t> fe, unique_ptr<socket_t> be);
            void WorkerThread(unique_ptr<socket_t> s);
            vector<thread> worker_threads;
            vector<thread> proxy_threads;
    };
    class OneThreadListener : public ZMQListener {
        public:
            OneThreadListener(AbstractServerConnector *s, const EndPoints& eps);
            virtual ~OneThreadListener(); 
        private:
            thread listening_thread;
            void WorkerThread(unique_ptr<vector<socket_t>> socks);
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
        try {
            if (threads_count)
                listener.reset(new MultiThreadListener(this, endpoints, threads_count));
            else
                listener.reset(new OneThreadListener(this, endpoints));
            return true;
        } catch (const std::exception& e) {
            listener.reset(nullptr);
            return false;
        }
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

    //cerr << __LINE__ << __FUNCTION__ << " " << response << endl;
    sock.send(response.c_str(), response.size());

    return true;
}

void ZMQListener::DoRequest(socket_t& sock)
{
    message_t msg;
    sock.recv(&msg);
    string request(reinterpret_cast<const char *>(msg.data()), msg.size());
    //cerr << __LINE__ << __FUNCTION__ << " " << request << endl;
    server->OnRequest(request, reinterpret_cast<void *>(new socket_wrapper(sock)));
}

MultiThreadListener::MultiThreadListener(AbstractServerConnector *s, const EndPoints& eps, unsigned int threads_count)
    : ZMQListener(s, eps)
{
    /* Starting FRONTEND(ROUTER) <-> BACKEND(DEALER) proxies */
    for (size_t i = 0; i < eps.size(); i++) {
        ostringstream bes;
        bes << "inproc://jsonrpc_worker" << i;
        string be = bes.str();

        unique_ptr<socket_t> frontend(new socket_t(ctx, ZMQ_ROUTER));
        unique_ptr<socket_t> backend(new socket_t(ctx, ZMQ_DEALER));

        frontend->bind(eps[i].c_str());
        backend->bind(be.c_str());

        proxy_threads.emplace_back( [=, &frontend, &backend] {
                ProxyThread(std::move(frontend), std::move(backend));
        });
        //cerr << "MultiThreadListener::MultiThreadListener() "
        //    << eps[i] << "<->" << be << endl;
        for (size_t w = 0; w < threads_count; w++) {
            unique_ptr<socket_t> worker(new socket_t(ctx, ZMQ_REP));
            worker->connect(be.c_str());

            worker_threads.emplace_back(&MultiThreadListener::WorkerThread,
                    this, move(worker));
        }
    }

}
MultiThreadListener::~MultiThreadListener()
{
    stop = true;
    for (auto& w : worker_threads) {
        w.join();
    }
    ctx.close();
    for (auto& p : proxy_threads) {
        p.join();
    }
}
void MultiThreadListener::ProxyThread(unique_ptr<socket_t> frontend, unique_ptr<socket_t> backend) {
    
    
    try {

        /* Works while ctx is active */
        proxy(*frontend, *backend, NULL);
    } catch (const zmq::error_t& e) {
        //cerr << "MultiThreadListener::ProxyThread() throw " << e.what() << endl;
    }
}
void MultiThreadListener::WorkerThread(unique_ptr<socket_t> sock) {
    try {
        while(!stop) {
            pollitem_t item;
            item.socket = *sock;
            item.events = ZMQ_POLLIN;

            poll(&item, 1, POLL_INTERVALL);
            if (item.revents & ZMQ_POLLIN) {
                DoRequest(*sock);
            };
        }
    } catch (const zmq::error_t& e) {
        //if (e.num != ETERM)
            //throw e;
        // if ETERM -- ctx is closed, just exiting thread.

        //cerr << "MultiThreadListener::WorkerThread() throw " << e.what() << endl;
    }
}

OneThreadListener::OneThreadListener(AbstractServerConnector *s, const EndPoints& eps)
    : ZMQListener(s, eps)
{
    unique_ptr<vector<socket_t>> socks(new vector<socket_t>());
    
    for(size_t i = 0; i < endpoints.size(); i++) {
        const int linger = 0;
        socks->emplace_back(ctx, ZMQ_REP);
        socks->at(i).setsockopt(ZMQ_LINGER, &linger, sizeof(linger));
        socks->at(i).bind(endpoints[i].c_str());
    }
    listening_thread = thread(&OneThreadListener::WorkerThread,
            this, move(socks));
}
OneThreadListener::~OneThreadListener() 
{
    stop = true;
    listening_thread.join();
    ctx.close();
}
void OneThreadListener::WorkerThread(unique_ptr<vector<socket_t>> socks)
{
    vector<pollitem_t> items(endpoints.size());
    for(size_t i = 0; i < endpoints.size(); i++) {
        items[i].socket = socks->at(i);
        items[i].events = ZMQ_POLLIN;
    }
    try {
        while(!stop) {
            poll(&items[0], items.size(), POLL_INTERVALL);
            if (stop)
                break;
            for(size_t i = 0; i < items.size(); i++) {
                if (items[i].revents & ZMQ_POLLIN) {
                    DoRequest(socks->at(i));
                };
            };
        };
    } catch (const zmq::error_t& e) {
        // if ETERM -- ctx is closed, just exiting thread.
        //cerr << __LINE__ << __FUNCTION__ << " " << e.what() << endl;
    }
}

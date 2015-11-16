/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    zeromqserver.h
 * @date    15.11.2015
 * @author  Badaev Vladimir <dead.skif@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_ZEROMQSERVER_H_

#define JSONRPC_CPP_ZEROMQSERVER_H_
#include <vector>
#include <string>
#include <thread>

#include <zmq.hpp>
#include "../abstractserverconnector.h"

namespace jsonrpc
{
    template<typename Listener>
    class ZeroMQServer;
    class ZeroMQListener {
        public:
            typedef std::vector<std::string> EndPoints;
            ZeroMQListener(AbstractServerConnector *s, const EndPoints& eps)
                : server(s), ctx(), endpoints(eps)
            {}
            virtual ~ZeroMQListener() {}

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
            void ProxyThread(const std::string& fe, const std::string be);
            void WorkerThread(const std::string be);
            std::vector<std::thread> worker_threads;
    };
	/**
	 * This class provides ZeroMQ REP/REQ Socket Server,to handle incoming Requests.
	 */
    template<typename Listener>
	class ZeroMQServer: public AbstractServerConnector
	{
		public:

			/**
			 * @brief ZeroMQSocketServer, constructor for the included ZeroMQSocketServer
			 * @param endpoint, a string containing the ZeroMQ endpoint
			 */
			ZeroMQServer(std::string endpoint);
            ZeroMQServer(std::vector<std::string> endpoints);

            virtual ~ZeroMQServer();

			virtual bool StartListening();
			virtual bool StopListening();

			bool virtual SendResponse(const std::string& response, void* addInfo = NULL);


		protected:
            std::vector<std::string> endpoints;
            unsigned int threads;

            std::unique_ptr<ZeroMQListener> listener;

    };
#if 0
    template<>
    class ZeroMQSocketServer<1>: public AbstractServerConnector
    {
        public:

			/**
			 * @brief ZeroMQSocketServer, constructor for the included ZeroMQSocketServer
			 * @param endpoint, a string containing the ZeroMQ endpoint
			 */
			ZeroSocketServer(const char *endpoint);
            ZeroSocketServer(const char *endpoints[], size_t endpoints_count);

			virtual bool StartListening();
			virtual bool StopListening();

			bool virtual SendResponse(const std::string& response, void* addInfo = NULL);


		private:
            zmq::context_t ctx;
            zmq::socket_t sock;
    };
#endif
} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_ZEROMQSERVER_H_ */

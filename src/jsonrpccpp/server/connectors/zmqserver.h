/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    zmqserver.h
 * @date    15.11.2015
 * @author  Badaev Vladimir <dead.skif@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_ZMQSERVER_H_

#define JSONRPC_CPP_ZMQSERVER_H_
#include <vector>
#include <string>
#include <thread>

#include <zmq.hpp>
#include "../abstractserverconnector.h"

namespace jsonrpc
{
    class ZMQListener;
	/**
	 * This class provides ZeroMQ REP/REQ Socket Server,to handle incoming Requests.
	 */
	class ZMQServer: public AbstractServerConnector
	{
		public:

			/**
			 * @brief ZeroMQSocketServer, constructor for the included ZeroMQSocketServer
			 * @param endpoint, a string containing the ZeroMQ endpoint or IP address.
             * @param threads_count, 0 for 1-thread variant, else will run threads_count threads for each endpoint.
			 */
			ZMQServer(const std::string& endpoint="*", unsigned int threads_coun=0);
            ZMQServer(std::vector<std::string> endpoints, unsigned int threads_count=0);

            virtual ~ZMQServer();

			virtual bool StartListening();
			virtual bool StopListening();

			bool virtual SendResponse(const std::string& response, void* addInfo = NULL);


		protected:
            std::vector<std::string> endpoints;
            std::unique_ptr<ZMQListener> listener;
            unsigned int threads_count;

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
#endif /* JSONRPC_CPP_ZMQSERVER_H_ */

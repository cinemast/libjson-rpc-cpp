/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    abstractserverconnector.h
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_SERVERCONNECTOR_H_
#define JSONRPC_CPP_SERVERCONNECTOR_H_

#include <string>
#include "iclientconnectionhandler.h"

namespace jsonrpc
{
    
    class AbstractServerConnector
    {
        public:
            AbstractServerConnector();
            virtual ~AbstractServerConnector();

            /**
             * This method should signal the Connector to start waiting for requests, in any way that is appropriate for the derived connector class.
             * If something went wrong, this method should return false, otherwise true.
             */
            virtual bool StartListening() = 0;
            /**
             * This method should signal the Connector to stop waiting for requests, in any way that is appropriate for the derived connector class.
             * If something went wrong, this method should return false, otherwise true.
             */
            virtual bool StopListening() = 0;

            /**
             * This method should send a response to the client in any way that is appropriate for the derived connector class.
             * @param response - the response that should be send to the client
             * @param addInfo - additional Info, that the Connector might need for responding.
             * @return returns true on success, false otherwise
             */
            bool virtual SendResponse(const std::string& response, void* addInfo = NULL) = 0;

            /**
             * This method must be called, when a request is recognised. It will do everything else for you (including sending the response).
             * @param request - the request that has been recognised.
             * @param addInfo - additional Info, that the Connector might need for responding.
             */
            bool OnRequest(const std::string& request, void* addInfo = NULL);

            void SetHandler(IClientConnectionHandler* handler);
            IClientConnectionHandler* GetHandler();

        private:
            IClientConnectionHandler *handler;
    };

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_ERVERCONNECTOR_H_ */

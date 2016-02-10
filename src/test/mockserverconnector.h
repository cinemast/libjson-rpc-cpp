/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    mockserverconnector.h
 * @date    10/10/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_MOCKSERVERCONNECTOR_H
#define JSONRPC_MOCKSERVERCONNECTOR_H

#include <jsonrpccpp/server/abstractserverconnector.h>
#include <jsonrpccpp/common/jsonparser.h>

namespace jsonrpc {

    class MockServerConnector : public AbstractServerConnector
    {
        public:
            MockServerConnector();

            virtual bool StartListening();
            virtual bool StopListening();

            bool virtual SendResponse(const std::string& response, void* addInfo = NULL);

            bool SetRequest(const std::string &request);
            Json::Value GetJsonRequest();

            std::string GetResponse();
            Json::Value GetJsonResponse();

        private:
            std::string request;
            std::string response;
    };

} // namespace jsonrpc

#endif // JSONRPC_MOCKSERVERCONNECTOR_H

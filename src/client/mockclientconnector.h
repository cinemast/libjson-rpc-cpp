#pragma once

#include "../connector/iclientconnector.h"
#include "../jsonparser.h"

namespace jsonrpc {

    class MockClientConnector : public IClientConnector
    {
        public:
            MockClientConnector();

            void SetResponse(const std::string &response);

            std::string GetRequest();
            Json::Value GetJsonRequest();
            virtual std::string SendRPCMessage(const std::string& message);

        private:
            std::string response;
            std::string request;
    };

} // namespace jsonrpc

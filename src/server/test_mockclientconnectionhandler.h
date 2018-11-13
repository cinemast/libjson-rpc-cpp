#pragma once

#include "../connector/iclientconnectionhandler.h"
#include <string>

namespace jsonrpc
{
    class MockClientConnectionHandler : public IClientConnectionHandler
    {
        public:
            MockClientConnectionHandler();

            virtual std::string HandleRequest(const std::string& request);

            std::string response;
            std::string request;
            long timeout;
    };
}

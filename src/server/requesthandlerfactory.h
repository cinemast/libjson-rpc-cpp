#pragma once

#include "iprocedureinvokationhandler.h"
#include "abstractprotocolhandler.h"

namespace jsonrpc {

    typedef enum {JSONRPC_SERVER_V1, JSONRPC_SERVER_V2, JSONRPC_SERVER_V1V2} serverVersion_t;

    class RequestHandlerFactory
    {
        public:
            static AbstractProtocolHandler* createProtocolHandler(serverVersion_t type, IProcedureInvokationHandler& handler);
    };

} // namespace jsonrpc

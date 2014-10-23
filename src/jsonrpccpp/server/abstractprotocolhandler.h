/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    abstractprotocolhandler.h
 * @date    10/23/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_ABSTRACTPROTOCOLHANDLER_H
#define JSONRPC_CPP_ABSTRACTPROTOCOLHANDLER_H

#include "iprocedureinvokationhandler.h"
#include "iclientconnectionhandler.h"
#include <map>
#include <string>
#include <jsonrpccpp/common/procedure.h>

namespace jsonrpc {

    class AbstractProtocolHandler : public IClientConnectionHandler {
        public:
            AbstractProtocolHandler(IProcedureInvokationHandler &handler);
            virtual ~AbstractProtocolHandler();

            void AddProcedure(Procedure& procedure);

        protected:
            IProcedureInvokationHandler &handler;
            std::map<std::string, Procedure> procedures;
    };

} // namespace jsonrpc

#endif // JSONRPC_CPP_ABSTRACTPROTOCOLHANDLER_H

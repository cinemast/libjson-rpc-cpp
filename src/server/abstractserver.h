#pragma once

#include <map>
#include <string>
#include <vector>
#include "../connector/abstractserverconnector.h"
#include "../connector/iclientconnectionhandler.h"
#include "procedure.h"
#include "abstractprotocolhandler.h"
#include "iprocedureinvokationhandler.h"
#include "requesthandlerfactory.h"

namespace jsonrpc
{

    template<class S>
    class AbstractServer : public IProcedureInvokationHandler, public IClientConnectionHandler
    {
        public:
            typedef void(S::*methodPointer_t)       (const Json::Value &parameter, Json::Value &result);
            typedef void(S::*notificationPointer_t) (const Json::Value &parameter);

            AbstractServer(serverVersion_t type = JSONRPC_SERVER_V2)
            {
                this->handler = RequestHandlerFactory::createProtocolHandler(type, *this);
            }

            virtual ~AbstractServer()
            {
                delete this->handler;
            }

            virtual bool HandleRequest(const std::string& request, std::string& response) {
                return this->handler->HandleRequest(request, response);
            }

            virtual void HandleMethodCall(Procedure &proc, const Json::Value& input, Json::Value& output)
            {
                S* instance = dynamic_cast<S*>(this);
                (instance->*methods[proc.GetProcedureName()])(input, output);
            }

            virtual void HandleNotificationCall(Procedure &proc, const Json::Value& input)
            {
                S* instance = dynamic_cast<S*>(this);
                (instance->*notifications[proc.GetProcedureName()])(input);
            }

        protected:
            bool bindAndAddMethod(const Procedure& proc, methodPointer_t pointer)
            {
                if(proc.GetProcedureType() == RPC_METHOD && !this->symbolExists(proc.GetProcedureName()))
                {
                    this->handler->AddProcedure(proc);
                    this->methods[proc.GetProcedureName()] = pointer;
                    return true;
                }
                return false;
            }

            bool bindAndAddNotification(const Procedure& proc, notificationPointer_t pointer)
            {
                if(proc.GetProcedureType() == RPC_NOTIFICATION && !this->symbolExists(proc.GetProcedureName()))
                {
                    this->handler->AddProcedure(proc);
                    this->notifications[proc.GetProcedureName()] = pointer;
                    return true;
                }
                return false;
            }

        private:
            AbstractProtocolHandler                         *handler;
            std::map<std::string, methodPointer_t>          methods;
            std::map<std::string, notificationPointer_t>    notifications;

            bool symbolExists(const std::string &name)
            {
                if (methods.find(name) != methods.end())
                    return true;
                if (notifications.find(name) != notifications.end())
                    return true;
                return false;
            }
    };

} /* namespace jsonrpc */
/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    server.h
 * @date    30.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <string>
#include <vector>

#include "rpcprotocolserver.h"
#include "serverconnector.h"

namespace jsonrpc
{
    template<class S>
    class AbstractServer : public AbstractRequestHandler
    {
        public:
            typedef void(S::*methodPointer_t)(const Json::Value &parameter, Json::Value &result);
            typedef void(S::*notificationPointer_t)(const Json::Value &parameter);

            AbstractServer(AbstractServerConnector *connector) :
                connection(connector),
                handler(this)
            {
                connector->SetHandler(this->handler);
            }

            AbstractServer(const std::string &configfile, AbstractServerConnector *connector) :
                handler(this, SpecificationParser::GetProceduresFromFile(configfile)),
                connection(connector)
            {
                connector->SetHandler(this->handler);
            }

            virtual ~AbstractServer()
            {
                this->StopListening();
                delete this->connection;
            }

            /**
             * @brief StartListening starts the AbstractServerConnector to listen for incoming requests.
             * @return
             */
            virtual bool StartListening()
            {
                return this->connection->StartListening();
            }


            /**
             * @brief StopListening stops the AbstractServerConnector, no more requests will be answered.
             * @return
             */
            virtual bool StopListening()
            {
                return this->connection->StopListening();
            }

            /**
             * @brief Returns the protocol instance, which can be used to get all registered Methods.
             * @return
             */
            virtual RpcProtocolServer* GetProtocolHanlder()
            {
                return &this->handler;
            }

            virtual void handleMethodCall(Procedure* proc, const Json::Value& input, Json::Value& output)
            {
                S* instance = dynamic_cast<S*>(this);
                (instance->*methods[proc->GetProcedureName()])(input, output);
            }

            virtual void handleNotificationCall(Procedure* proc, const Json::Value& input)
            {
                S* instance = dynamic_cast<S*>(this);
                (instance->*notifications[proc->GetProcedureName()])(input);
            }

        protected:
            virtual bool bindMethod(std::string& name, methodPointer_t method)
            {
                if(this->handler.GetProcedures().find(name) != this->handler.GetProcedures().end() && this->handler.GetProcedures()[name]->GetProcedureType() == RPC_METHOD)
                {
                    this->methods[name] = method;
                    return true;
                }
                return false;
            }

            virtual bool bindNotification(std::string& name, notificationPointer_t notification)
            {
                if(this->handler.GetProcedures().find(name) != this->handler.GetProcedures().end() && this->handler.GetProcedures()[name]->GetProcedureType() == RPC_NOTIFICATION)
                {
                    this->notifications[name] = notification;
                    return true;
                }
                return false;
            }

            virtual bool bindAndAddMethod(Procedure* proc, methodPointer_t pointer)
            {
                if(proc->GetProcedureType() == RPC_METHOD)
                {
                    this->handler.AddProcedure(proc);
                    this->methods[proc->GetProcedureName()] = pointer;
                    return true;
                }
                return false;
            }

            virtual bool bindAndAddNotification(Procedure* proc, notificationPointer_t pointer)
            {
                if(proc->GetProcedureType() == RPC_NOTIFICATION)
                {
                    this->handler.AddProcedure(proc);
                    this->notifications[proc->GetProcedureName()] = pointer;
                    return true;
                }
                return false;
            }

        private:
            AbstractServerConnector* connection;
            RpcProtocolServer handler;
            std::map<std::string, methodPointer_t> methods;
            std::map<std::string, notificationPointer_t> notifications;
    };

} /* namespace jsonrpc */
#endif /* SERVER_H_ */

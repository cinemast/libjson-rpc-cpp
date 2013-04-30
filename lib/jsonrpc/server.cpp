/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    server.cpp
 * @date    30.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "server.h"
#include "exception.h"
#include <fstream>

using namespace std;

namespace jsonrpc
{
    Server::Server(AbstractServerConnector *connector) :
        connection(connector)
    {
        connector->SetHandler(this->handler);
    }

    Server::Server(const string &configfile, methodpointer_t &methods, notificationpointer_t &notifications, AbstractServerConnector *connector) :
        handler(SpecificationParser::GetProcedures(configfile, methods, notifications)),
        connection(connector)
    {
        connector->SetHandler(this->handler);
    }

    Server::~Server()
    {
        this->StopListening();
        delete this->connection;
    }
    
    bool Server::StartListening()
    {
        return this->connection->StartListening();
    }
    
    bool Server::StopListening()
    {
        return this->connection->StopListening();
    }

    void Server::setAuthenticator(AbstractAuthenticator *auth)
    {
        this->handler.setAuthenticator(auth);
    }
} /* namespace jsonrpc */


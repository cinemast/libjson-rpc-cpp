/**
 * @file server.cpp
 * @date 30.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include "server.h"

#include <fstream>

using namespace std;

namespace jsonrpc
{
    Server::Server(const std::string& name, const std::string& configfile,
            methods_t& methods, notifications_t& notifications,
            ServerConnector* connector, Authenticator* auth)
    {
        this->handler = new RequestHandler(name);
        this->configFile = configfile;
        this->auth = auth;
        Procedure* proc;
        Json::Reader reader;
        Json::Value val, line;
        string value;
        ifstream myfile;
        char tmp[500];

        myfile.open(configfile.c_str(), ios::app | ios::in);
        while (!myfile.eof())
        {
            myfile.getline(tmp, 255);
            value.append(tmp);

        }
        myfile.close();

        if (!reader.parse(value, val))
        {
            throw std::string(
                    "Error while parsing json-method file: " + this->configFile);
        }

        notifications_t::iterator it_notifications;
        methods_t::iterator it_methods;
        for (unsigned int i = 0; i < val.size(); i++)
        {
            line = val[i];
            proc = new Procedure(line);
            if (proc->GetProcedureType() == RPC_METHOD)
            {
                it_methods = methods.find(proc->GetProcedureName());
                if (it_methods != methods.end())
                {
                    proc->SetMethodPointer(it_methods->second);
                }
            }
            else
            {
                it_notifications = notifications.find(
                        proc->GetProcedureName());
                if (it_notifications != notifications.end())
                {
                    proc->SetNotificationPointer(it_notifications->second);
                }
            }
            this->handler->AddProcedure(proc);
        }
        this->connection = connector;
        this->connection->SetHandler(this->handler);
    }

    Server::~Server()
    {
        delete this->handler;
        this->StopListening();
        delete this->connection;
        if(this->auth != NULL)
        {
            delete this->auth;
        }
    }
    
    bool Server::StartListening()
    {
        return this->connection->StartListening();
    }
    
    bool Server::StopListening()
    {
        return this->connection->StopListening();
    }

} /* namespace jsonrpc */

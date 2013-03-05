/**
 * @file server.cpp
 * @date 30.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include "server.h"
#include "exception.h"
#include <fstream>

using namespace std;

namespace jsonrpc
{
    Server::Server(const std::string& name, const std::string& configfile,
            methods_t& methods, notifications_t& notifications,
            AbstractServerConnector* connector, Authenticator* auth)
    {
        this->handler = new RequestHandler(name);
        this->configFile = configfile;
        this->auth = auth;
        Procedure* proc;
        Json::Reader reader;
        Json::Value val, line;
        string value;

        ifstream config(configFile.c_str());

        if (config)
        {
            config.open(configfile.c_str(), ios::in);
            value.assign((std::istreambuf_iterator<char>(config)),
                    (std::istreambuf_iterator<char>()));
        }
        else
        {
            throw Exception(ERROR_CONFIGURATIONFILE_NOT_FOUND, configFile);
        }

        if (!reader.parse(value, val))
        {
            throw Exception(ERROR_PARSING_JSON);
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
                it_notifications = notifications.find(proc->GetProcedureName());
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
        if (this->auth != NULL)
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

    std::vector<Procedure*> Server::ParseProcedures(const std::string& configfile)
    {
        Procedure* proc;
        Json::Reader reader;
        Json::Value val, line;
        string value;
        vector<Procedure*> result;

        ifstream config(configfile.c_str());

        if (config)
        {
            config.open(configfile.c_str(), ios::in);
            value.assign((std::istreambuf_iterator<char>(config)),
                    (std::istreambuf_iterator<char>()));
        }
        else
        {
            throw Exception(ERROR_CONFIGURATIONFILE_NOT_FOUND, configfile);
        }

        if (!reader.parse(value, val))
        {
            throw Exception(ERROR_PARSING_JSON);
        }

        notifications_t::iterator it_notifications;
        methods_t::iterator it_methods;
        for (unsigned int i = 0; i < val.size(); i++)
        {
            line = val[i];
            proc = new Procedure(line);
            result.push_back(proc);
        }

        return result;

    }

} /* namespace jsonrpc */


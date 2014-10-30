/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    httpserver.cpp
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "httpserver.h"
#include <cstdlib>
#include <sstream>
#include <iostream>

using namespace jsonrpc;
using namespace std;

#define BUFFERSIZE 65536

struct mhd_coninfo {
        struct MHD_PostProcessor *postprocessor;
        MHD_Connection* connection;
        stringstream request;
        HttpServer* server;
        int code;
};

IClientConnectionHandler *HttpServer::getHandler(const std::string &url)
{
    if (this->GetHandler() != NULL)
        return this->GetHandler();
    map<string, IClientConnectionHandler*>::iterator it = this->urlhandler.find(url);
    if (it != this->urlhandler.end())
        return it->second;
    return NULL;
}

HttpServer::HttpServer(int port, const std::string &sslcert) :
    AbstractServerConnector(),
    port(port),
    running(false),
    sslcert(sslcert),
    daemon(NULL)
{
}

bool HttpServer::StartListening()
{
    if(!this->running)
    {
        this->daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, this->port, NULL, NULL, HttpServer::callback, this, NULL, MHD_OPTION_END);
        if (this->daemon != NULL)
            this->running = true;

    }
    return this->running;
}

bool HttpServer::StopListening()
{
    if(this->running)
    {
        MHD_stop_daemon(this->daemon);
        this->running = false;
    }
    return true;
}

bool HttpServer::SendResponse(const string& response, void* addInfo)
{
    struct mhd_coninfo* client_connection = (struct mhd_coninfo*)addInfo;
    struct MHD_Response *result = MHD_create_response_from_data(response.size(),(void *) response.c_str(), MHD_NO, MHD_NO);

    if (result == NULL)
        return false;

    MHD_add_response_header(result, "Content-Type", "application/json");
    MHD_add_response_header(result, "Access-Control-Allow-Origin", "*");

    int ret = MHD_queue_response(client_connection->connection, client_connection->code, result);
    MHD_destroy_response(result);
    return ret == MHD_YES;
}

void HttpServer::SetUrlHandler(const string &url, IClientConnectionHandler *handler)
{
    this->urlhandler[url] = handler;
    this->SetHandler(NULL);
}

int HttpServer::callback(void *cls, MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls)
{
    if (*con_cls == NULL)
    {
        struct mhd_coninfo* client_connection = new mhd_coninfo;

        if (client_connection == NULL)
            return MHD_NO;

        client_connection->connection = connection;
        client_connection->server = (HttpServer*)cls;

        if (string("POST") == method)
        {
            *con_cls = client_connection;
            return MHD_YES;
        }
    }

    if (string("POST") == method)
    {
        struct mhd_coninfo* client_connection = (struct mhd_coninfo*)*con_cls;
        if (*upload_data_size != 0)
        {
            client_connection->request.write(upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        }
        else
        {
            string response;
            IClientConnectionHandler* handler = client_connection->server->getHandler(string(url));

            int ret = MHD_YES;
            if (handler == NULL)
            {
                client_connection->code = MHD_HTTP_INTERNAL_SERVER_ERROR;
                client_connection->server->SendResponse("No client conneciton handler found", client_connection);
            }
            else
            {
                client_connection->code = MHD_HTTP_OK;
                handler->HandleRequest(client_connection->request.str(), response);
                if (!client_connection->server->SendResponse(response, client_connection))
                    ret = MHD_NO;
            }
            delete client_connection;

            return ret;
        }
    }
    return MHD_NO;
}


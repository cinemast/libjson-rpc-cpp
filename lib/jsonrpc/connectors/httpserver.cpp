/**
 * @file httpserverconnector.cpp
 * @date 31.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include "httpserver.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace jsonrpc
{
    static void *callback(enum mg_event event, struct mg_connection *conn)
    {
        const struct mg_request_info *request_info = mg_get_request_info(conn);
        char* readBuffer = NULL;
        int postSize = 0;

        HttpServer* _this = (HttpServer*) request_info->user_data;

        if (event == MG_NEW_REQUEST)
        {

            if (strcmp(request_info->request_method, "GET") == 0)
            {
                //Mark the request as unprocessed.
                return NULL;
            }
            else if (strcmp(request_info->request_method, "POST") == 0)
            {
                //get size of postData
                sscanf(mg_get_header(conn, "Content-Length"), "%d", &postSize);
                readBuffer = (char*) malloc(sizeof(char) * (postSize + 1));
                mg_read(conn, readBuffer, postSize);
                _this->OnRequest(readBuffer, conn);
                free(readBuffer);

                //Mark the request as processed by our handler.
                return (void*) "";
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            return NULL;
        }
    }

    HttpServer::HttpServer(int port) : port(port), resPath("")
    {
        this->running = false;
    }

    HttpServer::HttpServer(int port, const std::string& getResourcePath)
        : AbstractServerConnector()
    {
        this->port = port;
        this->ctx = NULL;
        this->resPath = getResourcePath;
        this->running = false;
    }

    HttpServer::~HttpServer()
    {
        this->StopListening();
    }
    
    bool HttpServer::StartListening()
    {
        char port[6];
        sprintf(port, "%d", this->port);
        if(this->resPath == "")
        {
            const char *options[] = { "listening_ports", port, NULL };
            this->ctx = mg_start(&callback, this, options);
        }
        else
        {
            const char *options[] =
            { "document_root", this->resPath.c_str(), "listening_ports",
              port, NULL };
            this->ctx = mg_start(&callback, this, options);
        }

        if (this->ctx != NULL)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
    bool HttpServer::StopListening()
    {
        if(this->running)
        {
            mg_stop(this->ctx);
            return true;
        }
    }
    
    bool HttpServer::SendResponse(const std::string& response, void* addInfo)
    {
        struct mg_connection* conn = (struct mg_connection*) addInfo;
        if (mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                      "Content-Type: application/json\r\n"
                      "Content-Length: %d\r\n"
                      "\r\n"
                      "%s",(int)response.length(), response.c_str()) > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

} /* namespace jsonrpc */

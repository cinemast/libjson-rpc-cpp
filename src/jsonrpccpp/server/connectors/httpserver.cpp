/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    httpserver.cpp
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "httpserver.h"
#include "mongoose.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>

using namespace jsonrpc;

int HttpServer::callback(struct mg_connection *conn)
{
    const struct mg_request_info *request_info = mg_get_request_info(conn);
    char* readBuffer = NULL;
    int postSize = 0;

    HttpServer* _this = (HttpServer*) request_info->user_data;

    if (strcmp(request_info->request_method, "GET") == 0)
    {

        //Mark the request as unprocessed.
        return 0;
    }
    else if (strcmp(request_info->request_method, "POST") == 0)
    {
        //get size of postData
        const char* size_header = mg_get_header(conn, "Content-Length");
        if (size_header != NULL)
        {
            sscanf(size_header, "%d", &postSize);
            readBuffer = (char*) malloc(sizeof(char) * (postSize + 1));
            mg_read(conn, readBuffer, postSize);
            _this->OnRequest(readBuffer, conn);
            free(readBuffer);
        }
        else
        {
            _this->OnRequest("", conn);
        }


        //Mark the request as processed by our handler.
        return 1;
    }
    else
    {
        return 0;
    }
}

HttpServer::HttpServer(int port, bool enableSpecification, const std::string &sslcert, int threads) :
    AbstractServerConnector(),
    ctx(NULL),
    running(false),
    showSpec(enableSpecification),
    sslcert(sslcert),
    threads(threads)
{
    std::stringstream ss;
    ss << port;
    this->port = ss.str();
}

HttpServer::HttpServer(const std::string &port, bool enableSpecification, const std::string &sslcert, int threads) :
    AbstractServerConnector(),
    port(port),
    ctx(NULL),
    running(false),
    showSpec(enableSpecification),
    sslcert(sslcert),
    threads(threads)
{
}

bool HttpServer::StartListening()
{
    if(!this->running)
    {
        char threads[6];
        struct mg_callbacks callbacks;
        memset(&callbacks, 0, sizeof(callbacks));
        callbacks.begin_request = callback;

        sprintf(threads, "%d", this->threads);

        if(this->sslcert == "")
        {
            const char *options[] = { "listening_ports", this->port.c_str(), "num_threads", threads, NULL };
            this->ctx = mg_start(&callbacks, this, options);
        }
        else
        {
            const char *options[] =
            { "listening_ports", this->port.c_str(), "ssl_certificate", this->sslcert.c_str(), "num_threads", threads, NULL };
            this->ctx = mg_start(&callbacks, this, options);
        }

        if (this->ctx != NULL)
        {
            this->running =  true;
            return true;
        }
        else
        {
            this->running = false;
            return false;
        }
    }
    else
    {
        return true;
    }
}

bool HttpServer::StopListening()
{
    if(this->running)
    {
        mg_stop(this->ctx);
        this->running = false;
        return true;
    }
    return true;
}

bool HttpServer::SendResponse(const std::string& response, void* addInfo)
{
    struct mg_connection* conn = (struct mg_connection*) addInfo;
    if (mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: %d\r\n"
                  "Access-Control-Allow-Origin: *\r\n"
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

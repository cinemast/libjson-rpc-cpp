/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    httpserver.h
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef HTTPSERVERCONNECTOR_H_
#define HTTPSERVERCONNECTOR_H_

#include "mongoose.h"
#include "../serverconnector.h"

namespace jsonrpc
{
    /**
     * This class provides an embedded HTTP Server, based on Mongoose, to handle incoming Requests and send HTTP 1.1
     * valid responses.
     * Note that this class will always send HTTP-Status 200, even though an JSON-RPC Error might have occurred. Please
     * always check for the JSON-RPC Error Header.
     */
    class HttpServer: public AbstractServerConnector
    {
        public:

            /**
             * @brief HttpServer, constructor for the included HttpServer
             * @param port on which the server is listening
             * @param enableSpecification - defines if the specification is returned in case of a GET request
             * @param sslcert - defines the path to a SSL certificate, if this path is != "", then SSL/HTTPS is used with the given certificate.
             */
            HttpServer(int port, bool enableSpecification = true, const std::string& sslcert = "", int threads = 50);
            virtual ~HttpServer();

            virtual bool StartListening();
            virtual bool StopListening();

            bool virtual SendResponse(const std::string& response,
                    void* addInfo = NULL);

        private:
            int port;
            struct mg_context *ctx;
            bool running;
            bool showSpec;
            std::string sslcert;
            int threads;

            static int callback(struct mg_connection *conn);

    };

} /* namespace jsonrpc */
#endif /* HTTPSERVERCONNECTOR_H_ */

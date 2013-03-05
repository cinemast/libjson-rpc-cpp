/**
 * @file httpserverconnector.h
 * @date 31.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#ifndef HTTPSERVERCONNECTOR_H_
#define HTTPSERVERCONNECTOR_H_

#include <mongoose/mongoose.h>
#include "../serverconnector.h"

namespace jsonrpc
{
    /**
     * Typedefinition to differ between Text and Binary ressources
     */
    typedef enum
    {
        TEXT, BINARY
    } ResourceType;

    /**
     * This class provides an embedded HTTP Server, based on Mongoose, to handle incoming Requests and send HTTP 1.1
     * valid responses.
     * Note that this class will always send HTTP-Status 200, even though an JSON-RPC Error might have occurred. Please
     * always check for the JSON-RPC Error Header.
     *
     * Query String Parameters:
     *  You can send Query String Parameters in a GET Request. E.g.: http://testserver:port?someParameter
     *
     *  Possible Parameters:
     *      jsonrpcmethods: Will send you the json-rpc-cpp Configuration file with all method descriptions
     *      html: Will read a defined (see Constructor) HTML File.
     */
    class HttpServer: public AbstractServerConnector
    {
        public:
            HttpServer(int port);
            HttpServer(int port, const std::string& getResourcePath);
            virtual ~HttpServer();

            virtual bool StartListening();
            virtual bool StopListening();

            bool virtual SendResponse(const std::string& response,
                    void* addInfo = NULL);

        private:
            int port;
            struct mg_context *ctx;
            std::string resPath;
    };

} /* namespace jsonrpc */
#endif /* HTTPSERVERCONNECTOR_H_ */

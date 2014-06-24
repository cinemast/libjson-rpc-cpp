/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    httpclient.h
 * @date    02.01.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include "../clientconnector.h"
#include "../exception.h"
#include <map>
#include <curl/curl.h>

namespace jsonrpc
{

    class HttpClient : public AbstractClientConnector
    {
        public:
            HttpClient(const std::string& url) throw (JsonRpcException);
            virtual ~HttpClient();

            virtual void SendMessage(const std::string& message, std::string& result) throw (JsonRpcException);

            void SetUrl(const std::string& url);

            void AddHeader(const std::string attr, const std::string val);
            void RemoveHeader(const std::string attr);

        private:
            std::map<std::string,std::string> headers;
            std::string url;
            CURL* curl;
    };

} /* namespace jsonrpc */
#endif /* HTTPCLIENT_H_ */

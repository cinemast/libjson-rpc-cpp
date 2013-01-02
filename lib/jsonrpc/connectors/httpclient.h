/**
 * @file httpclient.h
 * @date 02.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include "../clientconnector.h"

#include <curl/curl.h>

namespace jsonrpc
{
    
    class HttpClient : public ClientConnector
    {
        public:
            HttpClient(const std::string& url);
            virtual ~HttpClient();

            virtual std::string SendMessage(const std::string& message);

            void SetUrl(const std::string& url);

        private:
            const std::string url;
            CURL* curl;
    };

} /* namespace jsonrpc */
#endif /* HTTPCLIENT_H_ */

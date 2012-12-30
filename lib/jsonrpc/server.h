/**
 * @file server.h
 * @date 30.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief This class provides the basic instance of a json rpc server
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <string>

namespace jsonrpc
{
    //typedef std::map<std::string,jsonRequestPointer> methods_t;
    //typedef std::map<std::string,jsonNotificationPointer> notifications_t;

    class Server
    {
        public:
            Server();
            virtual
            ~Server();
    };

} /* namespace jsonrpc */
#endif /* SERVER_H_ */

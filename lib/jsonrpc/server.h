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
#include <vector>

#include "requesthandler.h"
#include "serverconnector.h"

namespace jsonrpc
{
    typedef std::map<std::string,pRequest_t> methods_t;
    typedef std::map<std::string,pNotification_t> notifications_t;

    class Server
    {
        public:
            Server(const std::string& name, const std::string& configfile, methods_t& methods, notifications_t& notifications, AbstractServerConnector* connector, Authenticator* auth = NULL);
            virtual ~Server();

            bool StartListening();
            bool StopListening();

            const std::string& GetConfigFile() const
            {
                return configFile;
            }

            static std::vector<Procedure*> ParseProcedures(const std::string& configfile);

        private:
            Authenticator* auth;
            AbstractServerConnector* connection;
            RequestHandler* handler;
            std::string configFile;

    };

} /* namespace jsonrpc */
#endif /* SERVER_H_ */

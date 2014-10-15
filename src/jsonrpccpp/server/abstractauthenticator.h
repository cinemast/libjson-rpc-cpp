/**
 * @file authenticationmanager.h
 * @date 31.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#ifndef AUTHENTICATIONMANAGER_H_
#define AUTHENTICATIONMANAGER_H_

#include "json/json.h"

namespace jsonrpc
{
    
    class AbstractAuthenticator
    {
        public:
            /**
             * This method must be defined in any derived class from JsonRpcAuthenticationManager.
             * @param authentication - should contain the current authentication header of the incoming request.
             * @param method - the method that is planned to be processed.
             * @return ERROR_NO if permission should be granted. returns ERROR_PERMISSION_DENIED if no permission should be granted.
             *      If something was wrong with the authentication header, it should return ERROR_MALLFORMED_AUTHENTICATION_HEADER.
             */
            virtual int CheckPermission(const Json::Value &authentication,
                    const std::string &method) = 0;

            /**
             * This method must be defined in any derived class from JsonRpcAuthenticationManager.
             * @param authentication - will contain the current authentication Header of the incoming request.
             * @param returnValue - a reference to a valid Json::Value, which will contain the new authHeader.
             */
            virtual void ProcessAuthentication(
                    const Json::Value &authentication,
                    Json::Value &returnValue) = 0;

            virtual ~AbstractAuthenticator() {}
    };

} /* namespace jsonrpc */
#endif /* AUTHENTICATIONMANAGER_H_ */

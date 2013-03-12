/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    requesthandler.h
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef REQUESTHANDLER_H_
#define REQUESTHANDLER_H_

#include <string>
#include <vector>
#include <map>

#include "specificationparser.h"
#include "abstractauthenticator.h"

#define KEY_REQUEST_METHODNAME "method"
#define KEY_REQUEST_VERSION "jsonrpc"
#define KEY_REQUEST_ID "id"
#define KEY_REQUEST_PARAMETERS "params"
#define KEY_RESPONSE_ERROR "error"
#define KEY_RESPONSE_RESULT "result"
#define KEY_AUTHENTICATION "auth"

#define JSON_RPC_VERSION "2.0"

namespace jsonrpc
{

    /**
     * typedef for observerCallback Functions
     */
    typedef void (*observerFunction)(const std::string&, const Json::Value&);

    class RequestHandler
    {
        public:
            RequestHandler(procedurelist_t* procedures, AbstractAuthenticator* auth);
            virtual ~RequestHandler();

            /**
             * This is the key feature of this class, it deals with the JSOn-RPC 2.0 protocol.
             *  @param request - holds (hopefully) a valid JSON-Request Object.
             *  @param retValue a reference to string object which will hold the response after this method;
             */
            void HandleRequest(const std::string& request, std::string& retValue);

        private:

            int ValidateRequest(const Json::Value &val);

            /**
             * @pre the request must be a valid request
             * @param request - the request Object compliant to Json-RPC 2.0
             * @param retValue - a reference to an object which will hold the returnValue afterwards.
             *
             * after calling this method, the requested Method will be executed. It is important, that this method only gets called once per request.
             */
            void ProcessRequest(const Json::Value &request,
                    Json::Value &retValue);

            /**
             * This map holds all procedures. The string holds the name of each procedure.
             */
            procedurelist_t* procedures;
            /**
             * this objects decides whether a request is allowed to be processed or not.
             */
            AbstractAuthenticator* authManager;

    };

} /* namespace jsonrpc */
#endif /* REQUESTHANDLER_H_ */

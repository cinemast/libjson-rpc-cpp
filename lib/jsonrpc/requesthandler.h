/**
 * @file requesthandler.h
 * @date 31.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#ifndef REQUESTHANDLER_H_
#define REQUESTHANDLER_H_

#include <string>
#include <vector>
#include <map>

#include "procedure.h"
#include "authenticator.h"

#define KEY_REQUEST_METHODNAME "method"
#define KEY_REQUEST_VERSION "jsonrpc"
#define KEY_REQUEST_ID "id"
#define KEY_REQUEST_PARAMETERS "params"
#define KEY_RESPONSE_ERROR "error"
#define KEY_RESPONSE_RESULT "result"
#define KEY_AUTHENTICATION "auth"

#define JSON_RPC_VERSION 2.0

namespace jsonrpc
{
    /**
     * typedef for observerCallback Functions
     */
    typedef void (*observerFunction)(const std::string&, const Json::Value&);

    /**
     * typedef for different Observer Types
     */
    typedef enum
    {
        ON_RESPONSE, ON_REQUEST, ON_REQUEST_RESPONSE
    } observer_t;
    
    typedef std::map<std::string, Procedure*> procedurelist_t;

    class RequestHandler
    {
        public:
            RequestHandler(const std::string& instanceName);
            virtual ~RequestHandler();

            /**
             * @param fp - the passed function is called on every request that comes in.
             * @param t - sets the type of this observer.
             */
            void AddObserver(observerFunction fp, observer_t t);
            void RemoveObserver(observerFunction fp);

            bool AddProcedure(Procedure* procedure);
            bool RemoveProcedure(const std::string& procedure);

            const Authenticator& GetAuthManager() const;
            const std::string& GetInstanceName() const;

            const std::vector<observerFunction>& GetResponseObservers() const;
            const std::vector<observerFunction>& GetRequestObservers() const;
            const procedurelist_t& GetProcedures() const;

            void SetAuthManager(Authenticator* authManager);
            void SetProcedures(const procedurelist_t& procedures);

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
             * This method is called on each request, to notify all Registered Observers.
             */
            void NotifyObservers(
                    const std::vector<observerFunction>& observerGroup,
                    const Json::Value& request);

            /**
             * Each Request Handler has its instancename to identify for logging purposes.
             */
            std::string instanceName;

            /**
             * This vector holds all observers that want to be notified on each request.
             */
            std::vector<observerFunction> requestObservers;
            std::vector<observerFunction> responseObservers;

            /**
             * This map holds all procedures. The string holds the name of each procedure.
             */
            procedurelist_t procedures;

            /**
             * this objects decides whether a request is allowed to be processed or not.
             */
            Authenticator* authManager;

    };

} /* namespace jsonrpc */
#endif /* REQUESTHANDLER_H_ */

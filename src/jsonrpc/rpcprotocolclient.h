/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    responsehandler.h
 * @date    13.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef RESPONSEHANDLER_H
#define RESPONSEHANDLER_H

#include "abstractauthenticator.h"
#include "exception.h"
#include "json/value.h"
#include <string>

namespace jsonrpc {

    typedef std::multimap<const std::string, const Json::Value> batchProcedureCall_t;

    /**
     * @brief The RpcProtocolClient class handles the json-rpc 2.0 protocol for the client side.
     */
    class RpcProtocolClient
    {
        public:
            RpcProtocolClient();
            RpcProtocolClient(AbstractAuthenticator& authenticator);

            /**
             * @brief This method builds a valid json-rpc 2.0 request object based on passed paramters.
             * The id starts at 1 and is incremented for each request. To reset this value to one, call
             * the jsonrpc::RpcProRpcProtocolClient::resetId() method.
             * @param method - name of method or notification to be called
             * @param parameter - parameters represented as json objects
             * @return the string representation of the request to be built.
             */
            std::string BuildRequest(const std::string& method, const Json::Value& parameter, bool isNotification);

            /**
             * @brief BuildBatchRequest, this method builts a batch request. jsonrpc::batchProcedureCall_t is basically a map.
             * This method only works for either batch method calls, or batch notification calls, but not mixed ones.
             * @param requests - the map that holds the parameters for each method
             * @return the string representation of the request to be built.
             */
            std::string BuildBatchRequest(batchProcedureCall_t& requests, bool isNotification);

            /**
             * @brief BuildRequest does the same as std::string jsonrpc::RpcProRpcProtocolClient::BuildRequest(const std::string& method, const Json::Value& parameter);
             * The only difference here is that the result is returend by value, using the result parameter.
             * @param method - name of method or notification to be called
             * @param parameter - parameters represented as json objects
             * @param result - the string representation will be hold within this reference.
             */
            void BuildRequest(const std::string& method, const Json::Value& parameter, std::string& result, bool isNotification);

            /**
             * @brief The same is valid as for
             * void jsonrpc::RpcProtocolClient::BuildRequest(const std::string& method, const Json::Value& parameter, std::string& result);
             */
            void BuildBatchRequest(batchProcedureCall_t& requests, std::string& result, bool isNotification);


            /**
             * @brief This method parses the result of a json-rpc-server and returns the result object according
             * to the json-rpc 2.0 specification.
             * In case of an Error, an jsonrpc::Exception is thrown.
             * @param response - the resonse of a json-rpc server represented as string.
             * @return The result object inside the json-rpc response message
             */
            Json::Value HandleResponse(const std::string& response) throw(JsonRpcException);


            /**
             * @brief Does the same as Json::Value RpcProtocolClient::HandleResponse(const std::string& response) throw(Exception)
             * but returns result as reference for performance speed up.
             */
            void HandleResponse(const std::string& response, Json::Value& result) throw (JsonRpcException);


            /**
             * @brief resets the id used for building request objects.
             */
            void resetId();

            static const std::string KEY_PROTOCOL_VERSION;
            static const std::string KEY_PROCEDURE_NAME;
            static const std::string KEY_ID;
            static const std::string KEY_PARAMETER;
            static const std::string KEY_AUTH;
            static const std::string KEY_RESULT;
            static const std::string KEY_ERROR;
            static const std::string KEY_ERROR_CODE;
            static const std::string KEY_ERROR_MESSAGE;

        private:
            AbstractAuthenticator* authenticator;
            int id;

            void BuildRequest(const std::string& method, const Json::Value& parameter, Json::Value& result, bool isNotification);
    };
}
#endif // RESPONSEHANDLER_H

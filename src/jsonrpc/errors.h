/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    errors.h
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef ERRORS_H_
#define ERRORS_H_

#include <map>
#include <string>

#include "json/json.h"

namespace jsonrpc
{   
    class JsonRpcException;

    class Errors
    {
        public:
            /**
             * @param errorCode - the errorCode of which you want to create an errorBlock
             * @return - returns an error message as described in JSON-RPC 2.0 e.g. \code {"code" : -32600, "message": "INVALID_JSON_REQUEST: The JSON sent is not a valid JSON-RPC Request object" } \endcode
             * @pre the errorcode must be known, because it is not checked.
             * @see http://groups.google.com/group/json-rpc/web/json-rpc-1-2-proposal?pli=1#error-object
             */
            static Json::Value GetErrorBlock(const Json::Value& request, const int& errorCode);

            /**
             * Same as previous, but using user-generated exception.
             */
            static Json::Value GetErrorBlock(const Json::Value& request, const JsonRpcException& exc);

            /**
             * @return error message to corresponding error code.
             */
            static std::string GetErrorMessage(int errorCode);

            static class _init
            {
                public:
                _init();
            } _initializer;

            /**
             * Official JSON-RPC 2.0 Errors
             */
            static const int ERROR_RPC_JSON_PARSE_ERROR;
            static const int ERROR_RPC_METHOD_NOT_FOUND;
            static const int ERROR_RPC_INVALID_REQUEST;
            static const int ERROR_RPC_INVALID_PARAMS;
            static const int ERROR_RPC_INTERNAL_ERROR;

            /**
             * Server Library Errors
             */
            static const int ERROR_SERVER_PROCEDURE_IS_METHOD;
            static const int ERROR_SERVER_PROCEDURE_IS_NOTIFICATION;
            static const int ERROR_SERVER_PROCEDURE_POINTER_IS_NULL;
            static const int ERROR_SERVER_PROCEDURE_SPECIFICATION_NOT_FOUND;
            static const int ERROR_SERVER_PROCEDURE_SPECIFICATION_SYNTAX;
            static const int ERROR_SERVER_CONNECTOR;

            /**
             * Client Library Errors
             */
            static const int ERROR_CLIENT_CONNECTOR;
            static const int ERROR_CLIENT_INVALID_RESPONSE;
        private:
            static std::map<int, std::string> possibleErrors;
    };
} /* namespace jsonrpc */
#endif /* ERRORS_H_ */

/**
 * @file errors.cpp
 * @date 31.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include "errors.h"

namespace jsonrpc
{
    static Errors* instance = NULL;
    
    Errors::Errors()
    {
        //Official Errors
        this->possibleErrors[ERROR_INVALID_JSON_REQUEST] =
                "INVALID_JSON_REQUEST: The JSON sent is not a valid JSON-RPC Request object";
        this->possibleErrors[ERROR_METHOD_NOT_FOUND] =
                "METHOD_NOT_FOUND: The method being requested is not available on this server";
        this->possibleErrors[ERROR_INVALID_PARAMS] =
                "INVALID_PARAMS: Invalid method parameters (invalid name and/or type) recognised";
        this->possibleErrors[ERROR_JSON_PARSE_ERROR] =
                "JSON_PARSE_ERROR: The JSON-Object is not JSON-Valid";
        this->possibleErrors[ERROR_INTERNAL_ERROR] = "INTERNAL_ERROR: ";

        //Specific Errors
        this->possibleErrors[ERROR_PROCEDURE_IS_METHOD] =
                "PROCEDURE_IS_METHOD: The requested notification is declared as a method";
        this->possibleErrors[ERROR_PROCEDURE_IS_NOTIFICATION] =
                "PROCEDURE_IS_NOTIFICATION: The requested method is declared as notification";
        this->possibleErrors[ERROR_PROCEDURE_PARSE_ERROR] =
                "PROCEDURE_PARSE_ERROR: Something went wrong during parsing a procedure-signature";
        this->possibleErrors[ERROR_PROCEDURE_POINTER_IS_NULL] =
                "PROCEDURE_POINTER_IS_NULL: Server has no function Reference registered";
        this->possibleErrors[ERROR_PERMISSION_DENIED] =
                "PERMISSION_DENIED: Insufficient permissions for this request";
        this->possibleErrors[ERROR_MALLFORMED_AUTHENTICATION_HEADER] =
                "MALLFORMED_AUTHENTICATION_HEADER: Authentication header has wrong format";
    }
    
    Errors::~Errors()
    {
        this->possibleErrors.clear();
    }

    Errors* Errors::GetInstance()
    {
        if (instance == NULL)
        {
            instance = new Errors();
        }
        return instance;
    }

    bool Errors::AddError(int errorCode, const std::string& errorMessage)
    {
        if (errorCode < SPECIFIC_ERROR_STOP || errorCode > SPECIFIC_ERROR_START)
        {
            return false;
        }
        else
        {
            this->possibleErrors[errorCode] = errorMessage;
            return true;
        }
    }

    Json::Value Errors::GetErrorBlock(int errorCode)
    {
        Json::Value error;
        error[KEY_ERROR_CODE] = errorCode;
        error[KEY_ERROR_MESSAGE] = this->possibleErrors[errorCode];
        return error;
    }

    const std::string& Errors::GetErrorMessage(int errorCode)
    {
        return this->possibleErrors[errorCode];
    }


} /* namespace jsonrpc */

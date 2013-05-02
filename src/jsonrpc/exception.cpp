/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    exception.cpp
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "exception.h"

namespace jsonrpc
{
    JsonRpcException::JsonRpcException(int code)
            : code(code)
    {
        this->message = Errors::GetErrorMessage(code);
    }

    JsonRpcException::JsonRpcException(int code, const std::string& message)
            : code(code)
    {
        this->message = Errors::GetErrorMessage(code) +": "+ message;
    }

    JsonRpcException::JsonRpcException(const std::string& message)
    {
        this->message = message;
    }

    JsonRpcException::~JsonRpcException() throw ()
    {

    }

    int JsonRpcException::GetCode() const
    {
        return code;
    }

    const std::string& JsonRpcException::GetMessage() const
    {
        return message;
    }

    const char* JsonRpcException::what() const throw ()
    {
        std::stringstream ss;
        ss << "Error  " << this->code << " : " << this->message;
        return ss.str().c_str();
    }

} /* namespace jsonrpc */

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
        this->setWhatMessage();
    }

    JsonRpcException::JsonRpcException(int code, const std::string& message)
            : code(code)
    {
        this->message = Errors::GetErrorMessage(code) +": "+ message;
        this->setWhatMessage();
    }

    JsonRpcException::JsonRpcException(const std::string& message)
    {
        this->message = message;
        this->setWhatMessage();
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
        return this->whatString.c_str();
    }

    void JsonRpcException::setWhatMessage()
    {
        std::stringstream ss;
        ss << "Exception " << this->code << " : " << this->message;
        this->whatString = ss.str();
    }

} /* namespace jsonrpc */

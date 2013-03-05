/**
 * @file exception.cpp
 * @date 04.03.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include "exception.h"

namespace jsonrpc
{
    Exception::Exception(int code)
            : code(code)
    {
        this->message = Errors::GetInstance()->GetErrorMessage(code);
    }

    Exception::Exception(int code, const std::string& message)
            : code(code)
    {
        this->message = Errors::GetInstance()->GetErrorMessage(code) +": "+ message;
    }

    Exception::Exception(const std::string& message)
    {
        this->message = message;
    }

    Exception::~Exception() throw ()
    {

    }

    int Exception::GetCode() const
    {
        return code;
    }

    const std::string& Exception::GetMessage() const
    {
        return message;
    }

    const char* Exception::what() const throw ()
    {
        std::stringstream ss;
        ss << "Error  " << this->code << " : " << this->message;
        return ss.str().c_str();
    }

} /* namespace jsonrpc */

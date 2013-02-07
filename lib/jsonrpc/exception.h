/**
 * @file exception.h
 * @date 31.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <string>
#include <sstream>
#include <exception>

#include "errors.h"

namespace jsonrpc
{
    
    class Exception: public std::exception
    {
        public:
            Exception(int code)
                    : code(code)
            {
                this->message = Errors::GetInstance()->GetErrorMessage(code);
            }

            Exception(int code, const std::string& message)
                    : code(code)
            {
                this->message = Errors::GetInstance()->GetErrorMessage(code) +": "+ message;
            }

            Exception(const std::string& message)
            {
                this->message = message;
            }

            virtual ~Exception() throw ()
            {

            }

            inline int GetCode() const
            {
                return code;
            }

            inline const std::string& GetMessage() const
            {
                return message;
            }

            virtual const char* what() const throw ()
            {
                std::stringstream ss;
                ss << "Error  " << this->code << " : " << this->message;
                return ss.str().c_str();
            }

        private:
            int code;
            std::string message;
    };

} /* namespace jsonrpc */
#endif /* EXCEPTION_H_ */

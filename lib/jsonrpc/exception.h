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
            Exception(int code);

            Exception(int code, const std::string& message);

            Exception(const std::string& message);

            virtual ~Exception() throw ();

            int GetCode() const;

            const std::string& GetMessage() const;

            virtual const char* what() const throw ();

        private:
            int code;
            std::string message;
    };

} /* namespace jsonrpc */
#endif /* EXCEPTION_H_ */

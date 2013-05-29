/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    exception.h
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <string>
#include <sstream>
#include <exception>

#include "errors.h"

namespace jsonrpc
{
    
    class JsonRpcException: public std::exception
    {
        public:
            JsonRpcException(int code);

            JsonRpcException(int code, const std::string& message);

            JsonRpcException(const std::string& message);

            virtual ~JsonRpcException() throw ();

            int GetCode() const;

            const std::string& GetMessage() const;

            virtual const char* what() const throw ();

        private:
            int code;
            std::string message;
            std::string whatString;

            void setWhatMessage();
    };

} /* namespace jsonrpc */
#endif /* EXCEPTION_H_ */

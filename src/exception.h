#pragma once

#include <exception>
#include <sstream>
#include <string>

namespace jsonrpc {
  enum ExceptionCode { ERROR_CLIENT_CONNECTOR = -32003, ERROR_INVALID_JSON = -32700 };

  class JsonRpcException : public std::exception {
   public:
    JsonRpcException(int code, const std::string& message) : code(code) {
      std::stringstream str;
      str << "JsonRpcException " << code << ": " << message;
      this->message = str.str();
    }

    int GetCode() { return code; }

    virtual const char* what() const throw() { return message.c_str(); }

   private:
    int code;
    std::string message;
  };
}
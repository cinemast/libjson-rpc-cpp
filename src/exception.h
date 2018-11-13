#pragma once

#include "jsonparser.h"
#include <exception>
#include <sstream>
#include <string>

namespace jsonrpc {
  enum ExceptionCode { 
    NONE = 0,
    ERROR_CLIENT_INVALID_RESPONSE = -32001, 
    ERROR_CLIENT_CONNECTOR = -32003, 
    ERROR_INVALID_JSON = -32700,
    ERROR_SERVER_INVALID_REQUEST = -32600,
    ERROR_SERVER_PROCEDURE_IS_METHOD = -32604,
    ERROR_SERVER_PROCEDURE_IS_NOTIFICATION = -32605,
    ERROR_SERVER_CONNECTOR = -32002,
    ERROR_RPC_METHOD_NOT_FOUND = -32601,
    ERROR_RPC_INVALID_PARAMS = -32602
  };

  class JsonRpcException : public std::exception {
   public:
    JsonRpcException(int code, const std::string& message) : code(code) {
      std::stringstream str;
      str << "JsonRpcException " << code << ": " << message;
      this->message = str.str();
    }

    JsonRpcException(int code, const std::string& message, const Json::Value &data) : code(code), data(data) {
      std::stringstream str;
      str << "JsonRpcException " << code << ": " << message;
      this->message = str.str();
    }

    int GetCode() const { return code; }
    const std::string GetMessage() const { return this->message; }
    const Json::Value GetData() const { return data; }

    virtual const char* what() const throw() { return message.c_str(); }

   private:
    int code;
    std::string message;
    const Json::Value data;
  };
}
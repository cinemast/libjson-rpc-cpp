#include "errors.h"
#include "../exception.h"

using namespace jsonrpc;

std::map<int, std::string> Errors::possibleErrors;
Errors::_init Errors::_initializer;




Errors::_init::_init() {
  // Official Errors
  possibleErrors[ERROR_RPC_INVALID_REQUEST] = "INVALID_JSON_REQUEST: The JSON "
                                              "sent is not a valid JSON-RPC "
                                              "Request object";
  possibleErrors[ERROR_RPC_METHOD_NOT_FOUND] = "METHOD_NOT_FOUND: The method "
                                               "being requested is not "
                                               "available on this server";
  possibleErrors[ERROR_RPC_INVALID_PARAMS] = "INVALID_PARAMS: Invalid method "
                                             "parameters (invalid name and/or "
                                             "type) recognised";
  possibleErrors[ERROR_RPC_JSON_PARSE_ERROR] =
      "JSON_PARSE_ERROR: The JSON-Object is not JSON-Valid";
  possibleErrors[ERROR_RPC_INTERNAL_ERROR] = "INTERNAL_ERROR: ";

  possibleErrors[ERROR_SERVER_PROCEDURE_IS_METHOD] =
      "PROCEDURE_IS_METHOD: The requested notification is declared as a method";
  possibleErrors[ERROR_SERVER_PROCEDURE_IS_NOTIFICATION] =
      "PROCEDURE_IS_NOTIFICATION: The requested method is declared as "
      "notification";
  possibleErrors[ERROR_SERVER_PROCEDURE_POINTER_IS_NULL] =
      "PROCEDURE_POINTER_IS_NULL: Server has no function Reference registered";
  possibleErrors[ERROR_SERVER_PROCEDURE_SPECIFICATION_NOT_FOUND] =
      "Configuration file was not found";

  possibleErrors[ERROR_SERVER_PROCEDURE_SPECIFICATION_SYNTAX] =
      "Procedure specification mallformed";

  possibleErrors[ERROR_CLIENT_INVALID_RESPONSE] = "The response is invalid";
  possibleErrors[ERROR_CLIENT_CONNECTOR] = "Client connector error";
  possibleErrors[ERROR_SERVER_CONNECTOR] = "Server connector error";
}

std::string Errors::GetErrorMessage(int errorCode) {
  if (possibleErrors.find(errorCode) == possibleErrors.end()) {
    return "";
  }
  return possibleErrors[errorCode];
}

std::string Errors::GetErrorMessage(ExceptionCode code) {
    return "";
}

#include "rpcprotocolserverv1.h"
#include "../exception.h"

using namespace jsonrpc;

RpcProtocolServerV1::RpcProtocolServerV1(IProcedureInvokationHandler &handler)
    : AbstractProtocolHandler(handler) {}

void RpcProtocolServerV1::HandleJsonRequest(const Json::Value &req,
                                            Json::Value &response) {
  if (req.isObject()) {
    ExceptionCode error = this->ValidateRequest(req);
    if (error == 0) {
      try {
        this->ProcessRequest(req, response);
      } catch (const JsonRpcException &exc) {
        this->WrapException(req, exc, response);
      }
    } else {
      this->WrapError(req, error, this->GetErrorMessage(error), response);
    }
  } else {
    this->WrapError(Json::nullValue, ExceptionCode::ERROR_SERVER_INVALID_REQUEST,
                    "Invalid JSON-RPC request, expected object",
                    response);
  }
}

bool RpcProtocolServerV1::ValidateRequestFields(const Json::Value &request) {
  if (!(request.isMember(KEY_REQUEST_METHODNAME) &&
        request[KEY_REQUEST_METHODNAME].isString()))
    return false;
  if (!request.isMember(KEY_REQUEST_ID))
    return false;
  if (!request.isMember(KEY_REQUEST_PARAMETERS))
    return false;
  if (!(request[KEY_REQUEST_PARAMETERS].isArray() ||
        request[KEY_REQUEST_PARAMETERS].isNull()))
    return false;
  return true;
}

void RpcProtocolServerV1::WrapResult(const Json::Value &request,
                                     Json::Value &response,
                                     Json::Value &retValue) {
  response[KEY_RESPONSE_RESULT] = retValue;
  response[KEY_RESPONSE_ERROR] = Json::nullValue;
  response[KEY_REQUEST_ID] = request[KEY_REQUEST_ID];
}

void RpcProtocolServerV1::WrapError(const Json::Value &request, int code,
                                    const std::string &message,
                                    Json::Value &result) {
  result["error"]["code"] = code;
  result["error"]["message"] = message;
  if (request.isObject() && request.isMember("id")) {
    result["id"] = request["id"];
  } else {
    result["id"] = Json::nullValue;
  }
}

void RpcProtocolServerV1::WrapException(const Json::Value &request,
                                        const JsonRpcException &exception,
                                        Json::Value &result) {
  this->WrapError(request, exception.GetCode(), exception.GetMessage(), result);
  result["error"]["data"] = exception.GetData();
}

procedure_t RpcProtocolServerV1::GetRequestType(const Json::Value &request) {
  if (request[KEY_REQUEST_ID] == Json::nullValue)
    return RPC_NOTIFICATION;
  return RPC_METHOD;
}

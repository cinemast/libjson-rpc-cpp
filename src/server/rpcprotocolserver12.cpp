#include "rpcprotocolserver12.h"
#include "../jsonparser.h"

using namespace jsonrpc;

RpcProtocolServer12::RpcProtocolServer12(IProcedureInvokationHandler &handler)
    : rpc1(handler), rpc2(handler) {}

void RpcProtocolServer12::AddProcedure(const Procedure &procedure) {
  this->rpc1.AddProcedure(procedure);
  this->rpc2.AddProcedure(procedure);
}

void RpcProtocolServer12::HandleRequest(const std::string &request,
                                        std::string &retValue) {
  Json::Reader reader;
  Json::Value req;
  Json::Value resp;
  Json::FastWriter w;

  if (reader.parse(request, req, false)) {
    this->GetHandler(req).HandleJsonRequest(req, resp);
  } else {
    this->GetHandler(req).WrapError(
        Json::nullValue, ExceptionCode::ERROR_RPC_JSON_PARSE_ERROR,
        Errors::GetErrorMessage(ExceptionCode::ERROR_RPC_JSON_PARSE_ERROR), resp);
  }
  if (resp != Json::nullValue)
    retValue = w.write(resp);
}

AbstractProtocolHandler &
RpcProtocolServer12::GetHandler(const Json::Value &request) {
  if (request.isArray() || (request.isObject() && request.isMember("jsonrpc") &&
                            request["jsonrpc"].asString() == "2.0"))
    return rpc2;
  return rpc1;
}

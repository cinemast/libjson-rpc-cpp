/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    rpcprotocolserver12.cpp
 * @date    10/25/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "rpcprotocolserver12.h"

using namespace jsonrpc;

RpcProtocolServer12::RpcProtocolServer12(IProcedureInvokationHandler &handler) :
    AbstractProtocolHandler(handler),
    rpc1(handler),
    rpc2(handler)
{
}

void RpcProtocolServer12::AddProcedure(Procedure &procedure)
{
    this->rpc1.AddProcedure(procedure);
    this->rpc2.AddProcedure(procedure);
}

void RpcProtocolServer12::HandleJsonRequest(const Json::Value &request, Json::Value &response)
{
    this->GetHandler(request).HandleJsonRequest(request, response);
}

bool RpcProtocolServer12::ValidateRequestFields(const Json::Value &val)
{
    return this->GetHandler(val).ValidateRequestFields(val);
}

void RpcProtocolServer12::WrapResult(const Json::Value &request, Json::Value &response, Json::Value &retValue)
{
    this->GetHandler(request).WrapResult(request,response, retValue);
}

void RpcProtocolServer12::WrapError(const Json::Value &request, int code, const std::string &message, Json::Value &result)
{
    this->GetHandler(request).WrapError(request, code, message, result);
}

procedure_t RpcProtocolServer12::GetRequestType(const Json::Value &request)
{
    return this->GetHandler(request).GetRequestType(request);
}

AbstractProtocolHandler &RpcProtocolServer12::GetHandler(const Json::Value &request)
{
    if (request.isArray() || (request.isObject() && request.isMember("jsonrpc") && request["jsonrpc"].asString() == "2.0"))
        return rpc2;
    return rpc1;
}

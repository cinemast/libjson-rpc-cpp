/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    rpcprotocolserverv1.cpp
 * @date    10/23/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "rpcprotocolserverv1.h"

using namespace jsonrpc;

RpcProtocolServerV1::RpcProtocolServerV1(IProcedureInvokationHandler &handler) :
    AbstractProtocolHandler(handler)
{
}

void RpcProtocolServerV1::HandleRequest(const std::string &request, std::string &retValue)
{

}

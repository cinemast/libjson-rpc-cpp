/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    abstractprotocolhandler.cpp
 * @date    10/23/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "abstractprotocolhandler.h"

using namespace jsonrpc;

AbstractProtocolHandler::AbstractProtocolHandler(IProcedureInvokationHandler &handler) :
    handler(handler)
{
}

AbstractProtocolHandler::~AbstractProtocolHandler()
{
}

void AbstractProtocolHandler::AddProcedure(Procedure &procedure)
{
    this->procedures[procedure.GetProcedureName()] = procedure;
}

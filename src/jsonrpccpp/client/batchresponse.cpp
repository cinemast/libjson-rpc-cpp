/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    batchresponse.cpp
 * @date    10/9/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "batchresponse.h"
#include <algorithm>

using namespace jsonrpc;
using namespace std;


BatchResponse::BatchResponse()
{
}

void BatchResponse::addResponse(int id, Json::Value response, bool isError)
{
    if (isError) {
        errorResponses.push_back(id);
    }
    responses[id] = response;
}

Json::Value BatchResponse::getResult(int id)
{
    Json::Value result;
    getResult(id, result);
    return result;
}

void BatchResponse::getResult(int id, Json::Value &result)
{
    if (getErrorCode(id) == 0)
        result = responses[id];
    else
        result = Json::nullValue;
}

int BatchResponse::getErrorCode(int id)
{
    if(std::find(errorResponses.begin(), errorResponses.end(), id) != errorResponses.end())
    {
        return responses[id]["code"].asInt();
    }
    return 0;
}

string BatchResponse::getErrorMessage(int id)
{
    if(std::find(errorResponses.begin(), errorResponses.end(), id) != errorResponses.end())
    {
        return responses[id]["message"].asString();
    }
    return "";
}

bool BatchResponse::hasErrors()
{
    return !errorResponses.empty();
}

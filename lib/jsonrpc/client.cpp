/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    client.cpp
 * @date    03.01.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "client.h"
#include "requesthandler.h"
#include "exception.h"

namespace jsonrpc
{
    Client::Client(AbstractClientConnector* connector, bool validateResponse)
            : connector(connector), validateResponse(validateResponse)
    {
    }

    Client::~Client()
    {
        delete this->connector;
    }
    
    Json::Value Client::CallMethod(const std::string& name,
                                   const Json::Value& parameter) throw(Exception)
    {
        Json::FastWriter writer;
        Json::Reader reader;
        Json::Value result;

        int id = 1;

        std::string str_result = this->connector->SendMessage(
                writer.write(this->BuildRequestObject(name, parameter, id)));

        if (reader.parse(str_result, result, false))
        {
            if (validateResponse)
            {
                if (result[KEY_RESPONSE_ERROR] != Json::nullValue)
                {
                    throw Exception(result[KEY_RESPONSE_ERROR]["message"].asString());
                }

                if (result[KEY_RESPONSE_RESULT] == Json::nullValue)
                {
                    throw Exception(Errors::ERROR_CLIENT_INVALID_RESPONSE);
                }

                if (result[KEY_REQUEST_ID].asInt() != id)
                {
                    throw Exception(Errors::ERROR_CLIENT_INVALID_RESPONSE,
                            str_result + " / "
                                    + writer.write(
                                            this->BuildRequestObject(name,
                                                    parameter, id)));
                }
            }
            return result[KEY_RESPONSE_RESULT];
        }
        else
        {
            throw Exception(Errors::ERROR_RPC_JSON_PARSE_ERROR,
                    "Server response could not be parsed: " + str_result);
        }

        return Json::nullValue;
    }
    
    void Client::CallNotification(const std::string& name,
                                  const Json::Value& parameter) throw(Exception)
    {
        Json::FastWriter writer;
        this->connector->SendMessage(writer.write(this->BuildRequestObject(name,parameter, -1)));
    }

   /* std::vector<Json::Value> Client::BatchCallMethod(
            std::map<std::string, Json::Value> methodcalls)
    {
        //return Json::nullValue;
    }

    void Client::BatchCallNotification(
            std::map<std::string, Json::Value> methodcalls)
    {

    }*/
    
    /**
     * This method builds a method call if id > 0, it builds a notification call else (id <= 0).
     * @return valid request object
     */
    Json::Value Client::BuildRequestObject(const std::string& name,
            const Json::Value& parameters, int id) const
    {
        Json::Value request;
        request[KEY_METHOD_NAME] = name;
        request[KEY_REQUEST_VERSION] = 2.0;
        request[KEY_PROCEDURE_PARAMETERS] = parameters;

        if (id > 0)
        {
            request[KEY_REQUEST_ID] = id;
        }
        return request;
    }

} /* namespace jsonrpc */

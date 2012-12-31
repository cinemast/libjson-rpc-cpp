/**
 * @file serverconnector.cpp
 * @date 31.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#include "serverconnector.h"
#include <cstdlib>

using namespace std;

namespace jsonrpc
{
    
    ServerConnector::ServerConnector()
    {
        this->handler = NULL;
    }
    
    ServerConnector::~ServerConnector()
    {
    }
    
    bool ServerConnector::OnRequest(const std::string& request, void* addInfo)
    {
        string response;
        if (this->handler != NULL)
        {
            this->handler->HandleRequest(request, response);
            this->SendResponse(response, addInfo);
            return true;
        }
        else
        {
            return false;
        }
    }

} /* namespace jsonrpc */

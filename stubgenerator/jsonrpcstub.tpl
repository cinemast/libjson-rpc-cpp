#ifndef _<STUBNAME>_H_
#define _<STUBNAME>_H_

#include <jsonrpc/clientconnector.h>
#include <jsonrpc/client.h>
#include <string>
#include <json/json.h>

class <stubname>
{
    public:
        <stubname>(ClientConnector* conn)
        {
            this->client = new Client(conn,false);
        }
        ~<stubname>()
        {
            delete this->client;
        }

<methods>
        
    private:
        jsonrpc::Client* client;
}
#endif //_<STUBNAME>_H_
#ifndef _FOOBAR_H_
#define _FOOBAR_H_

#include <jsonrpc/clientconnector.h>
#include <jsonrpc/client.h>
#include <string>
#include <json/json.h>

class FooBar
{
    public:
        FooBar(jsonrpc::ClientConnector* conn)
        {
            this->client = new jsonrpc::Client(conn,false);
        }
        ~FooBar()
        {
            delete this->client;
        }
Json::Value sayHello(const std::string& name)
{
	Json::Value p;
	p["name"] = name; 

	return this->client->CallMethod("sayHello",p);
}

void notifyServer()
{
	Json::Value p;
	
	this->client->CallMethod("notifyServer",p);
}

  
    private:
        jsonrpc::Client* client;
};
#endif //_FOOBAR_H_

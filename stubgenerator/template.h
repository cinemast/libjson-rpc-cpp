#ifndef TEMPLATE_H_
#define TEMPLATE_H_

#define TEMPLATE_METHOD "\
        <return_type> <methodname>(<parameters>) throw (jsonrpc::Exception)\n\
        {\n\
            Json::Value p;\n\
            <parameter_assign>\n\
            <return_statement>\n\
        }\n\
"

#define TEMPLATE_STUB "\
/**\n\
 * THIS FILE IS GENERATED BY jsonrpcstub, DO NOT CHANGE IT!!!!!\n\
 */\n\
\n\
#ifndef _<STUBNAME>_H_\n\
#define _<STUBNAME>_H_\n\
\n\
#include <jsonrpc/clientconnector.h>\n\
#include <jsonrpc/client.h>\n\
#include <string>\n\
#include <json/json.h>\n\
\n\
class <stubname>\n\
{\n\
    public:\n\
        <stubname>(jsonrpc::AbstractClientConnector* conn)\n\
        {\n\
            this->client = new jsonrpc::Client(conn,false);\n\
        }\n\
        ~<stubname>()\n\
        {\n\
            delete this->client;\n\
        }\n\n\
<methods>\
    private:\n\
        jsonrpc::Client* client;\n\
};\n\
#endif //_<STUBNAME>_H_\n\
"

#endif
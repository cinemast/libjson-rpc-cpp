#pragma once

namespace jsonrpc
{
    enum procedure_t
    {
        RPC_METHOD, RPC_NOTIFICATION
    };

    enum jsontype_t
    {
        JSON_STRING = 1,
        JSON_BOOLEAN = 2,
        JSON_INTEGER = 3,
        JSON_REAL = 4,
        JSON_OBJECT = 5,
        JSON_ARRAY = 6
    };
}

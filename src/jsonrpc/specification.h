/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    specification.h
 * @date    30.04.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef SPECIFICATION_H
#define SPECIFICATION_H

#define KEY_METHOD_NAME "method"
#define KEY_NOTIFICATION_NAME "notification"
#define KEY_PROCEDURE_PARAMETERS "params"
#define KEY_RETURN_TYPE "returns"

namespace jsonrpc
{
    /**
     * This enum describes whether a Procdeure is a notification procdeure or a method procdeure
     * @see http://groups.google.com/group/json-rpc/web/json-rpc-2-0
     */
    typedef enum
    {
        RPC_METHOD, RPC_NOTIFICATION
    } procedure_t;

    /**
     * This enum represents all processable json Types of this framework.
     */
    enum jsontype_t
    {
        JSON_STRING = 1,
        JSON_BOOLEAN = 2,
        JSON_INTEGER = 3,
        JSON_REAL = 4,
        JSON_OBJECT = 5,
        JSON_ARRAY = 6,
        JSON_NULL = 7
    } ;
}

#endif // SPECIFICATION_H

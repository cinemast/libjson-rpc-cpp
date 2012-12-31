/**
 * @file procedure.h
 * @date 31.12.2012
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#ifndef PROCEDURE_H_
#define PROCEDURE_H_

#include <string>
#include <map>
#include <json/json.h>

/**
 * String literal for describing type string in the json-description file.
 */
#define TYPE_STRING "string"
/**
 * String literal for describing type integer in the json-description file.
 */
#define TYPE_INTEGER "integer"
/**
 * String literal for describing type real in the json-description file.
 */
#define TYPE_REAL "real"
/**
 * String literal for describing type boolean in the json-description file.
 */
#define TYPE_BOOLEAN "boolean"
/**
 * String literal for describing type object in the json-description file.
 */
#define TYPE_OBJECT "object"
/**
 * String literal for describing type array in the json-description file.
 */
#define TYPE_ARRAY "array"

/**
 * bool literal for describing type of the procedure (method or notification)
 */
#define JSON_RPC_METHOD false
#define JSON_RPC_NOTIFICATION true

#define KEY_PROCEDURE_NAME "method"
#define KEY_PROCEDURE_PARAMETERS "params"
#define KEY_PROCEDURE_TYPE "isNotification"

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
    typedef enum
    {
        JSON_STRING,
        JSON_BOOLEAN,
        JSON_INTEGER,
        JSON_REAL,
        JSON_OBJECT,
        JSON_ARRAY
    } jsontype_t;

    /**
     * Type declaration signature of an requestable Method
     * e.g. Json::Value doSomething(Json::Value parameter);
     */
    typedef void (*pRequest_t)(const Json::Value&, Json::Value&);

    /**
     * Type declaration signature of an notifyable Method
     * e.g. void doSomething(Json::Value parameter);
     */
    typedef void (*pNotification_t)(const Json::Value&);

    typedef std::map<std::string, jsontype_t> parameterlist_t;

    class Procedure
    {
        public:
            /**
             * @param name - describes the name of the corresponding procedure
             * @param procedure_type - holds the type of the procedure
             * @param parameters - holds a map of parameters which will get copied in the constructor.
             * @see JsonProcedureType
             */
            Procedure(const std::string &name, const procedure_t procedure_type, const parameterlist_t &parameters);

            /**
             * @param signature - Is a JSON-Value from which each procedure can parse itself. The parameter should have the following format:
             *      e.g. {
             *              "method" : "doSomething",
             *              "type" : "method",
             *              "params": [
             *                      "name" : "some literal"
             *                      "price" : 0.0
             *                      "ssnr" : 4711
             *                  ]
             *              }
             *
             *  please note, that you'll have to provide a valid "parameters" array, otherwise an Exception of type JsonRpcException will be thrown.
             *  The params Object looks a bit confusing. The Key of each paramArray Entry is the name of the parameter. The JSON-Value behind is only needed for parsing the right type.
             *  This could be any appropriate literal, it doesn't matter.
             */
            Procedure(const Json::Value &signature);


            ~Procedure();

            /**
             * This method is validating the incoming parameters for each procedure.
             * @param parameters - should contain the parameter-object of an valid json-rpc 2.0 request
             * @see http://groups.google.com/group/json-rpc/web/json-rpc-2-0
             * @return 0 on successful validation a negative errorcode otherwise.
             *
             * If the valid parameters are of Type JSON_ARRAY or JSON_OBJECT, they can only be checked for name and not for their structure.
             */
            int ValdiateParameters(const Json::Value &parameters);

            const parameterlist_t& GetParameters() const;
            procedure_t GetProcedureType() const;
            const std::string& GetProcedureName() const;

            /**
             * This method only returns a valid method Pointer if the corresponding procedure is of type METHOD
             * @see JsonProcedureType
             * @return returns a pointer to the corresponding function, or NULL if there is no pointer or the procedure is of type NOTIFICATION
             */
            pRequest_t GetMethodPointer();

            /**
             * This method only returns a valid method Pointer if the corresponding procedure is of type NOTIFICATION
             * @see JsonProcedureType
             * @return returns a pointer to the corresponding function, or NULL if there is no pointer or the procedure is of type METHOD
             */
            pNotification_t GetNotificationPointer();

            /**
             * sets in case of an Method the methodPointer
             * @return false if this procedure is not declared as Method.
             */
            bool SetMethodPointer(pRequest_t rp);

            /**
             * sets in case of an Notification the notificationPointer
             * @return false if this procedure is declared as Notification.
             */
            bool SetNotificationPointer(pNotification_t np);

        private:
            /**
             * Each Procedure should have a name.
             */
            std::string procedureName;
            /**
             * This map represents all necessary Parameters of each Procedure.
             * The string represents the name of each parameter and JsonType the type it should have.
             */
            parameterlist_t parameters;
            /**
             * defines whether the procedure is a real procedure or just a notification
             */
            procedure_t procedureType;

            /**
             * Because we can't decide at first whether it is a method or notification procedure, we have to keep the function Pointer as a union.
             * To get the right method pointer, one has to call getProcedureType to clarify which kind of procedure it is. After that the correspoinding getMethodPointer
             * or getNotificationPointer can be called.
             */
            union
            {
                    pRequest_t rp;
                    pNotification_t np;
            } procedurePointer;
    };

} /* namespace jsonrpc */
#endif /* PROCEDURE_H_ */

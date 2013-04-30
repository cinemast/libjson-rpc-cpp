/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    procedure.h
 * @date    31.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef PROCEDURE_H_
#define PROCEDURE_H_

#include <string>
#include <map>
#include <json/json.h>

#include "specification.h"

namespace jsonrpc
{
    /**
     * Type declaration signature of an requestable Method
     * e.g. Json::Value doSomething(Json::Value parameter);
     */
    typedef void (*pMethod_t)(const Json::Value&, Json::Value&);

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
             * @brief Constructor for notificaiton with parameters as va_list. The last parameter must be NULL.
             * If no parameters are passed, parameters either do not exist, or cannot be checked for type compliance by the library.
             * @param name
             */
            Procedure(const std::string &name, ...);

            /**
             * @brief Constructor for method with parameters as va_list. The last parameter must be NULL.
             * If no parameters are passed, parameters either do not exist, or cannot be checked for type compliance by the library.
             * @param name
             * @param returntype
             */
            Procedure(const std::string& name, jsontype_t returntype, ...);


            ~Procedure();

            /**
             * This method is validating the incoming parameters for each procedure.
             * @param parameters - should contain the parameter-object of an valid json-rpc 2.0 request
             * @see http://groups.google.com/group/json-rpc/web/json-rpc-2-0
             * @return true on successful validation false otherwise.
             *
             * If the valid parameters are of Type JSON_ARRAY or JSON_OBJECT, they can only be checked for name and not for their structure.
             */
            bool ValdiateParameters(const Json::Value &parameters);

            const parameterlist_t& GetParameters() const;
            procedure_t GetProcedureType() const;
            const std::string& GetProcedureName() const;

            /**
             * This method only returns a valid method Pointer if the corresponding procedure is of type METHOD
             * @see JsonProcedureType
             * @return returns a pointer to the corresponding function, or NULL if there is no pointer or the procedure is of type NOTIFICATION
             */
            pMethod_t GetMethodPointer() const;

            /**
             * This method only returns a valid method Pointer if the corresponding procedure is of type NOTIFICATION
             * @see JsonProcedureType
             * @return returns a pointer to the corresponding function, or NULL if there is no pointer or the procedure is of type METHOD
             */
            pNotification_t GetNotificationPointer() const;

            jsontype_t GetReturnType() const;

            /**
             * sets in case of an Method the methodPointer
             * @return false if this procedure is not declared as Method.
             */
            bool SetMethodPointer(pMethod_t rp);

            /**
             * sets in case of an Notification the notificationPointer
             * @return false if this procedure is declared as Notification.
             */
            bool SetNotificationPointer(pNotification_t np);

            void AddParameter(const std::string& name, jsontype_t type);

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
             * this field is only valid if procedure is of type method (not notification).
             */
            jsontype_t returntype;

            /**
             * Because we can't decide at first whether it is a method or notification procedure, we have to keep the function Pointer as a union.
             * To get the right method pointer, one has to call getProcedureType to clarify which kind of procedure it is. After that the correspoinding getMethodPointer
             * or getNotificationPointer can be called.
             */
            union
            {
                    pMethod_t rp;
                    pNotification_t np;
            } procedurePointer;
    };

} /* namespace jsonrpc */
#endif /* PROCEDURE_H_ */

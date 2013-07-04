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

#include "json/json.h"
#include "specification.h"

namespace jsonrpc
{
    typedef std::map<std::string, jsontype_t> parameterlist_t;

    class Procedure
    {
        public:

            /**
             * @brief Constructor for notificaiton with parameters as va_list. The last parameter must be NULL.
             * If no parameters are passed, parameters either do not exist, or cannot be checked for type compliance by the library.
             * @param name
             */
            Procedure(const std::string name, ...);

            /**
             * @brief Constructor for method with parameters as va_list. The last parameter must be NULL.
             * If no parameters are passed, parameters either do not exist, or cannot be checked for type compliance by the library.
             * @param name
             * @param returntype
             */
            Procedure(const std::string name, jsontype_t returntype, ...);


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

            parameterlist_t& GetParameters();
            procedure_t GetProcedureType() const;
            const std::string& GetProcedureName() const;

            jsontype_t GetReturnType() const;

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
    };

    typedef std::map<std::string, Procedure*> procedurelist_t;

} /* namespace jsonrpc */
#endif /* PROCEDURE_H_ */

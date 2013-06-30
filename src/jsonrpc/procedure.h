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


    class Procedure
    {
        public:

            /**
             * @brief Constructor for notificaiton with parameters as va_list. The last parameter must be NULL.
             * If no parameters are passed, parameters either do not exist, or cannot be checked for type compliance by the library.
             * @param name
             */
            Procedure(const std::string name, bool usePositinalParams ,...);

            /**
             * @brief Constructor for method with parameters as va_list. The last parameter must be NULL.
             * If no parameters are passed, parameters either do not exist, or cannot be checked for type compliance by the library.
             * @param name
             * @param returntype
             */
            Procedure(const std::string name, jsontype_t returntype, bool usePositinalParams, ...);


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


            /**
             * @brief GetNamedParameters
             * @return a map with all named parameters (only valid if procudre is declared with named parameters)
             */
            std::map<std::string, jsontype_t> GetNamedParameters();


            /**
             * @brief GetNamedParameters
             * @return a list with all positional parameters (only valid if procudre is declared with positional parameters)
             */
            std::vector<jsontype_t> GetPositionalParameters();


            jsontype_t GetProcedureType() const;

            const std::string& GetProcedureName() const;

            jsontype_t GetReturnType() const;

       //
        private:
            /**
             * Each Procedure should have a name.
             */
            std::string procedureName;


            std::map<std::string, jsontype_t> namedList;
            std::vector<jsontype_t> positionList;

            /**
             * defines whether the procedure is a real procedure or just a notification.
             */
            procedure_t procedureType;

            /**
             * this field is only valid if procedure is of type method (not notification).
             */
            jsontype_t returntype;

            /**
             * @brief this flags descides if this method uses positional or named parameters
             */
            bool positionalParameters;


            void AddNamedParameter(const std::string& name, jsontype_t type);
            void AddPositionalParameter(jsontype_t);

    };

    typedef std::map<std::string, Procedure*> procedurelist_t;

} /* namespace jsonrpc */
#endif /* PROCEDURE_H_ */

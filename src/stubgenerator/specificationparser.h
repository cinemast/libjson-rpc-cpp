#pragma once

#include "procedure.h"
#include "../exception.h"

namespace jsonrpc {

    class SpecificationParser
    {
        public:
            static std::vector<Procedure> GetProceduresFromFile(const std::string& filename)    ;
            static std::vector<Procedure> GetProceduresFromString(const std::string& spec)      ;

            static void         GetFileContent  (const std::string& filename, std::string& target);

        private:
            static Procedure    GetProcedure    (Json::Value& val);
            static void         GetMethod       (Json::Value& val, Procedure &target);
            static void         GetNotification (Json::Value& val, Procedure &target);
            static jsontype_t   toJsonType      (Json::Value& val);

            static void         GetPositionalParameters (Json::Value &val, Procedure &target);
            static void         GetNamedParameters      (Json::Value &val, Procedure &target);
            static std::string  GetProcedureName        (Json::Value &signature);

    };
}
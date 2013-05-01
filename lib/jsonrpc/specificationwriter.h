/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    specificationwriter.h
 * @date    30.04.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef SPECIFICATIONWRITER_H
#define SPECIFICATIONWRITER_H

#include "procedure.h"
#include "specification.h"

namespace jsonrpc {
    class SpecificationWriter
    {
        public:
            static Json::Value toJsonValue(procedurelist_t& procedures);
            static std::string toString(procedurelist_t& procedures);
            static void toFile(const std::string& filename, procedurelist_t& procedures);

        private:
            static Json::Value toJsonLiteral(jsontype_t type);
            static void procedureToJsonValue(Procedure* procedure, Json::Value& target);
    };
}

#endif // SPECIFICATIONWRITER_H

/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    specificationparser.h
 * @date    12.03.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef SPECIFICATIONPARSER_H
#define SPECIFICATIONPARSER_H

#include "procedure.h"
#include "exception.h"

namespace jsonrpc {

    typedef std::map<std::string, Procedure*> procedurelist_t;

    class SpecificationParser
    {
        public:
            static procedurelist_t* GetProcedures(const std::string& filename) throw (JsonRpcException);

        private:
            static Procedure* GetProcedure(Json::Value& val);
            static void GetFileContent(const std::string& filename, std::string& target);
    };
}
#endif // SPECIFICATIONPARSER_H

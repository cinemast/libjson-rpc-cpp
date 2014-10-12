/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    stubgenerator.h
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_STUBGENERATOR_H
#define JSONRPC_CPP_STUBGENERATOR_H

#include <string>
#include <jsonrpccpp/common/procedure.h>

#include "codegenerator.h"

namespace jsonrpc
{
    enum connectiontype_t {CONNECTION_HTTP};

    class StubGenerator
    {
        public:
            StubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, CodeGenerator &cg);
            virtual ~StubGenerator();


            virtual std::string getStubName();
            virtual void generateStub() = 0;

            static std::string replaceAll(const std::string& text, const std::string& fnd, const std::string& rep);
            static void replaceAll2(std::string &text, const std::string &find, const std::string &replace);

        protected:
            std::string             stubname;
            std::vector<Procedure>  &procedures;
            CodeGenerator           &cg;
    };
}


#endif // JSONRPC_CPP_STUBGENERATOR_H

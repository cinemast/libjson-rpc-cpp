/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    clientstubgenerator.h
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef CLIENTSTUBGENERATOR_H
#define CLIENTSTUBGENERATOR_H

#include "stubgenerator.h"

namespace jsonrpc
{
    class ClientStubGenerator : public StubGenerator
    {
        public:
            ClientStubGenerator(const std::string& stubname, const std::string& filename);

            virtual std::string generateStub();

        private:
            static std::string generateMethod(Procedure& proc);
    };
}
#endif // CLIENTSTUBGENERATOR_H

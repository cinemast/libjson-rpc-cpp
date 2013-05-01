/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    stubgenerator.h
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef STUBGENERATOR_H
#define STUBGENERATOR_H

#include <string>
#include <jsonrpc/procedure.h>

namespace jsonrpc
{
    class StubGenerator
    {
        public:
            StubGenerator(const std::string& stubname, const std::string& filename);

            ~StubGenerator();

            virtual std::string generateStub() = 0;
            void generateStubToFile(const std::string& outputpath);

            static void replaceAll(std::string& text, const std::string& fnd, const std::string& rep);

        protected:
            procedurelist_t* procedures;
            std::string stubname;
    };
}


#endif // STUBGENERATOR_H

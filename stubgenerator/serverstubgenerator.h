/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    serverstubgenerator.h
 * @date    01.05.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef SERVERSTUBGENERATOR_H
#define SERVERSTUBGENERATOR_H

#include "stubgenerator.h"

namespace jsonrpc
{
    class ServerStubGenerator : public StubGenerator
    {
        public:
            ServerStubGenerator(const std::string& stubname, const std::string& filename);
    };
}

#endif // SERVERSTUBGENERATOR_H

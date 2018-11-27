#pragma once

#include <vector>
#include "stubgenerator.h"

namespace stubgenerator {
class Procedure;
class App {
    public:
        int runStubGenerator(int argc, char** argv);
        std::vector<Procedure> procedures;
        std::vector<StubGenerator*> stubgens;
};

}



#pragma once

#include <vector>
#include "stubgenerator.h"

namespace jsonrpc {

    class StubGeneratorFactory
    {
        public:
            static bool createStubGenerators(int argc, char** argv, std::vector<Procedure> &procedures, std::vector<StubGenerator*> &stubgenerators, ostream& _stdout, ostream& _stderr);
            static void deleteStubGenerators(std::vector<StubGenerator*> &stubgenerators);
    };

} // namespace jsonrpc

#pragma once

#include "../stubgenerator.h"
#include "../codegenerator.h"

namespace stubgenerator
{
    class CPPClientStubGenerator : public StubGenerator
    {
        public:


            CPPClientStubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, std::ostream& outputstream);
            CPPClientStubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, const std::string filename);

            virtual void generateStub();

            void generateMethod(Procedure& proc);
            void generateAssignments(Procedure& proc);
            void generateProcCall(Procedure &proc);
    };
}


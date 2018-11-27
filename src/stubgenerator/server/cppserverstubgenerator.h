#pragma once

#include "../stubgenerator.h"
#include "../codegenerator.h"

namespace stubgenerator
{
    class CPPServerStubGenerator : public StubGenerator
    {
        public:
            CPPServerStubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, std::ostream &outputstream);
            CPPServerStubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, const std::string &filename);

            virtual void generateStub();

            void generateBindings();
            void generateProcedureDefinitions();
            void generateAbstractDefinitions();
            std::string generateBindingParameterlist(const Procedure &proc);
            void generateParameterMapping(const Procedure &proc);
    };
}

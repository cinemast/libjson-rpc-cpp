#ifndef PYTHON_CLIENT_STUB_GENERATOR_H
#define PYTHON_CLIENT_STUB_GENERATOR_H

#include "../stubgenerator.h"

namespace jsonrpc
{
    class PythonClientStubGenerator : public StubGenerator
    {
        public:


            PythonClientStubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, std::ostream& outputstream);
            PythonClientStubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, const std::string filename);

            virtual void generateStub();

            void generateMethod(Procedure& proc);
            void generateAssignments(Procedure& proc);
            void generateProcCall(Procedure &proc);
            std::string generateParameterDeclarationList(Procedure &proc);
    };
}
#endif // PYTHON_CLIENT_STUB_GENERATOR_H

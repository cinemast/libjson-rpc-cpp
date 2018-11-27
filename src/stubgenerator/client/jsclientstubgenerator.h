#pragma once

#include "../stubgenerator.h"

namespace stubgenerator {

    class JSClientStubGenerator : public StubGenerator
    {
        public:
            JSClientStubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, std::ostream &outputstream);
            JSClientStubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, const std::string &filename);

            static std::string class2Filename(const std::string &classname);

            virtual void generateStub();

        private:
            virtual void generateMethod(Procedure &proc);
            static std::string noramlizeJsLiteral(const std::string &literal);

    };

} // namespace jsonrpc

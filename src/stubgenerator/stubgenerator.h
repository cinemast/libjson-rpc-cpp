#pragma once

#include <string>
#include "procedure.h"

#include "codegenerator.h"

namespace jsonrpc
{
    enum connectiontype_t {CONNECTION_HTTP};

    class StubGenerator : public CodeGenerator
    {
        public:
            StubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, std::ostream &outputstream);
            StubGenerator(const std::string& stubname, std::vector<Procedure> &procedures, const std::string &filename);

            virtual ~StubGenerator();
            virtual void generateStub() = 0;

            static std::string replaceAll(const std::string& text, const std::string& fnd, const std::string& rep);
            static void replaceAll2(std::string &text, const std::string &find, const std::string &replace);

        protected:
            std::string             stubname;
            std::vector<Procedure>  &procedures;
    };
}
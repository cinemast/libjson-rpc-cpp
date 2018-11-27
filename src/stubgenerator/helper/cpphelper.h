#pragma once

#include <string>
#include <vector>
#include "../procedure.h"
#include "../codegenerator.h"

namespace stubgenerator
{
    class CPPHelper
    {
        public:
            static std::string normalizeString  (const std::string &text);
            static std::string toCppType        (jsonrpc::jsontype_t type, bool isConst = false, bool isReference = false);
            static std::string toCppConversion  (jsonrpc::jsontype_t);
            static std::string isCppConversion(jsonrpc::jsontype_t);
            static std::string toString         (jsonrpc::jsontype_t type);
            static std::string generateParameterDeclarationList(Procedure& proc);

            static std::string toCppReturntype  (jsonrpc::jsontype_t type);
            static std::string toCppParamType   (jsonrpc::jsontype_t type);

            static std::string class2Filename(const std::string &classname);
            static std::vector<std::string> splitPackages(const std::string &classname);

            static void prolog(CodeGenerator &cg, const std::string &stubname);
            static void epilog(CodeGenerator &cg, const std::string &stubname);

            static int namespaceOpen(CodeGenerator &cg, const std::string &classname);
            static void namespaceClose(CodeGenerator &cg, int depth);

    };
}
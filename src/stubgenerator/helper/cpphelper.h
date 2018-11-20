#pragma once

#include <string>
#include <vector>
#include "../procedure.h"
#include "../codegenerator.h"

namespace jsonrpc
{
    class CPPHelper
    {
        public:
            static std::string normalizeString  (const std::string &text);
            static std::string toCppType        (jsontype_t type, bool isConst = false, bool isReference = false);
            static std::string toCppConversion  (jsontype_t);
            static std::string isCppConversion(jsontype_t);
            static std::string toString         (jsontype_t type);
            static std::string generateParameterDeclarationList(Procedure& proc);

            static std::string toCppReturntype  (jsontype_t type);
            static std::string toCppParamType   (jsontype_t type);

            static std::string class2Filename(const std::string &classname);
            static std::vector<std::string> splitPackages(const std::string &classname);

            static void prolog(CodeGenerator &cg, const std::string &stubname);
            static void epilog(CodeGenerator &cg, const std::string &stubname);

            static int namespaceOpen(CodeGenerator &cg, const std::string &classname);
            static void namespaceClose(CodeGenerator &cg, int depth);

    };
}
#pragma once

#include <string>
#include <ostream>
#include <fstream>
#include <sstream>

namespace stubgenerator
{
    class CodeGenerator
    {
        public:
            CodeGenerator(const std::string &filename);
            CodeGenerator(std::ostream &outputstream);
            virtual ~CodeGenerator();

            void write (const std::string &line);
            void writeLine(const std::string &line);
            void writeNewLine();
            void increaseIndentation();
            void decreaseIndentation();

            void setIndentSymbol(const std::string &symbol);

        private:
            std::ostream *output;
            std::ofstream file;
            std::string indentSymbol;
            int indentation;
            bool atBeginning;
    };
}

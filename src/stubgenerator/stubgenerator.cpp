#include <algorithm>
#include <fstream>

#include "client/cppclientstubgenerator.h"
#include "client/jsclientstubgenerator.h"
#include "client/pyclientstubgenerator.h"
#include "helper/cpphelper.h"
#include "server/cppserverstubgenerator.h"
#include "stubgenerator.h"
#include "specificationparser.h"

using namespace std;
using namespace jsonrpc;
using namespace stubgenerator;

StubGenerator::StubGenerator(const string &stubname,
                             std::vector<Procedure> &procedures,
                             ostream &outputstream)
    : CodeGenerator(outputstream), stubname(stubname), procedures(procedures) {}

StubGenerator::StubGenerator(const string &stubname,
                             std::vector<Procedure> &procedures,
                             const std::string &filename)
    : CodeGenerator(filename), stubname(stubname), procedures(procedures) {}

StubGenerator::~StubGenerator() {}

string StubGenerator::replaceAll(const string &text, const string &fnd,
                                 const string &rep) {
  string result = text;
  replaceAll2(result, fnd, rep);
  return result;
}

void StubGenerator::replaceAll2(string &result, const string &find,
                                const string &replace) {
  size_t pos = result.find(find);
  while (pos != string::npos) {
    result.replace(pos, find.length(), replace);
    pos = result.find(find, pos + replace.length());
  }
}

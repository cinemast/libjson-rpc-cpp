#include <iostream>
#include <string>
#include <vector>
#include <cli/cli.hpp>

#include "app.h"
#include "specificationparser.h"
#include "helper/cpphelper.h"
#include "server/cppserverstubgenerator.h"
#include "client/cppclientstubgenerator.h"
#include "client/jsclientstubgenerator.h"
#include "client/pyclientstubgenerator.h"

using namespace std;
using namespace jsonrpc;
using namespace stubgenerator;

int App::runStubGenerator(int argc, char **argv) {

  CLI::App app{"stubgenerator for libjson-rpc-cpp based applications", "jsonrpcstub"};
  bool versionFlag;
  string filePath;
  string cppServerClass;
  string cppServerPath;
  string cppClientClass;
  string cppClientPath;
  string jsClientClass;
  string jsClientPath;
  string pythonClientClass;
  string pythonClientPath;

  app.add_option("specification", filePath, "the JSON file holding the specification")->check(CLI::ExistingFile)->required();

  app.add_flag("--version,-v", versionFlag, "print version info and exit");
  app.add_option("--cpp-server", cppServerClass, "name of the C++ server stub class")->type_name("namespace::classname");
  app.add_option("--cpp-server-file", cppServerPath, "filename for the C++ server stub");
  app.add_option("--cpp-client", cppClientClass, "name of the C++ client stub class")->type_name("namespace::classname");
  app.add_option("--cpp-client-file", cppClientPath, "filename for the C++ client stub");
  app.add_option("--js-client", jsClientClass, "name of the JavaScript client stub class")->type_name("classname");
  app.add_option("--js-client-file", jsClientPath, "filename for the JavaScript client stub");
  app.add_option("--py-client", pythonClientClass, "name of the Python client stub class")->type_name("classname");
  app.add_option("--py-client-file", pythonClientPath, "filename for the Python client stub");
  CLI11_PARSE(app, argc, argv);

  try {
    procedures = SpecificationParser::GetProceduresFromFile(filePath);

    cout << "Found " << procedures.size() << " in " << filePath << endl;
    for (auto proc : procedures) {
        cout << proc.ToString() << endl;
    }

    if (cppServerClass != "") {
        if (cppServerPath == "")
            cppServerPath = CPPHelper::class2Filename(cppServerClass);
        CPPServerStubGenerator stub(cppServerClass, procedures, cppServerPath);
        cout << "Generating C++ server stub to " << cppServerPath << endl;
        stub.generateStub();
    } 

    if (cppClientClass != "") {
        if (cppClientPath == "")
            cppClientPath = CPPHelper::class2Filename(cppClientClass);
        CPPClientStubGenerator stub(cppClientClass, procedures, cppClientPath);
        cout << "Generating C++ client stub to " << cppClientPath << endl;
        stub.generateStub();
    }

    if (jsClientClass != "") {
        if (jsClientPath == "")
            jsClientPath = JSClientStubGenerator::class2Filename(jsClientClass);
        JSClientStubGenerator stub(jsClientClass, procedures, jsClientPath);
        cout << "Generating JavaScript client stub to " << jsClientPath << endl;
        stub.generateStub();
    }

    if (pythonClientClass != "") {
        if (pythonClientPath == "")
            pythonClientPath = PythonClientStubGenerator::class2Filename(pythonClientClass);
        JSClientStubGenerator stub(pythonClientClass, procedures, pythonClientPath);
        cout << "Generating Python client stub to " << pythonClientPath << endl;
        stub.generateStub();
    }

  } catch (const JsonRpcException &ex) {
      cerr << "Error parsing file: " << ex.what() << endl;
      return 1;
  }

  return 0;
}

/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    stubgeneratorfactory.cpp
 * @date    11/19/2014
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "stubgeneratorfactory.h"
#include <jsonrpccpp/common/specificationparser.h>
#include <jsonrpccpp/version.h>
#include <iostream>
#include <argtable2.h>
#include "helper/cpphelper.h"
#include "client/cppclientstubgenerator.h"
#include "client/jsclientstubgenerator.h"
#include "server/cppserverstubgenerator.h"

using namespace jsonrpc;
using namespace std;

bool StubGeneratorFactory::createStubGenerators(int argc, char **argv, vector<Procedure> &procedures, vector<StubGenerator*> &stubgenerators, FILE *stdout, FILE *stderr)
{
    struct arg_file *inputfile      = arg_file0(NULL, NULL, "<specfile>", "path of input specification file");
    struct arg_lit *help            = arg_lit0("h","help", "print this help and exit");
    struct arg_lit *version         = arg_lit0(NULL,"version", "print version and exit");
    struct arg_lit *verbose         = arg_lit0("v","verbose", "print more information about what is happening");
    struct arg_str *cppserver       = arg_str0(NULL, "cpp-server", "<namespace::classname>", "name of the C++ server stub class");
    struct arg_str *cppserverfile   = arg_str0(NULL, "cpp-server-file", "<filename.h>", "name of the C++ server stub file");
    struct arg_str *cppclient       = arg_str0(NULL, "cpp-client", "<namespace::classname>", "name of the C++ client stub class");
    struct arg_str *cppclientfile   = arg_str0(NULL, "cpp-client-file", "<namespace::classname>", "name of the C++ client stub file");
    struct arg_str *jsclient        = arg_str0(NULL, "js-client", "<classname>", "name of the JavaScript client stub class");
    struct arg_str *jsclientfile    = arg_str0(NULL, "js-client-file", "<filename.js>", "name of the JavaScript client stub file");


    struct arg_end *end         = arg_end(20);
    void* argtable[] = {inputfile, help, version, verbose, cppserver, cppserverfile, cppclient, cppclientfile, jsclient, jsclientfile,end};

    if (arg_parse(argc,argv,argtable) > 0)
    {
        arg_print_errors(stderr, end, argv[0]);
        arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));
        return false;
    }

    if (help->count > 0)
    {
        fprintf(stdout, "Usage: %s ", argv[0]);
        arg_print_syntax(stdout,argtable,"\n"); cout << endl;
        arg_print_glossary_gnu(stdout, argtable);
        arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));
        return true;
    }

    if (version->count > 0)
    {
        fprintf(stdout, "jsonrpcstub version %d.%d.%d\n", JSONRPC_CPP_MAJOR_VERSION, JSONRPC_CPP_MINOR_VERSION, JSONRPC_CPP_PATCH_VERSION);
        arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));
        return true;
    }

    if (inputfile->count == 0) {
        fprintf(stderr, "Invalid arguments: specfile must be provided.\n");
        arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));
        return false;
    }

    try {
        procedures = SpecificationParser::GetProceduresFromFile(inputfile->filename[0]);
        if (verbose->count > 0)
        {
            fprintf(stdout, "Found %zu procedures in %s\n", procedures.size(), inputfile->filename[0]);
            for (unsigned int i = 0; i < procedures.size(); ++i) {
                if (procedures.at(i).GetProcedureType() == RPC_METHOD)
                {
                    fprintf(stdout, "\t[Method]         ");
                }
                else
                {
                    fprintf(stdout, "\t[Notification]   ");
                }
                fprintf(stdout, "%s\n", procedures.at(i).GetProcedureName().c_str());
            }
            fprintf(stdout, "\n");
        }

        if (cppserver->count > 0)
        {
            string filename;
            if (cppserverfile->count > 0)
                filename = cppserverfile->sval[0];
            else
                filename = CPPHelper::class2Filename(cppserver->sval[0]);
            if (verbose->count > 0)
                fprintf(stdout, "Generating C++ Serverstub to: %s\n", filename.c_str());
            stubgenerators.push_back(new CPPServerStubGenerator(cppserver->sval[0], procedures, filename));
        }

        if (cppclient->count > 0)
        {
            string filename;
            if (cppclientfile->count > 0)
                filename = cppclientfile->sval[0];
            else
                filename = CPPHelper::class2Filename(cppclient->sval[0]);
            if (verbose->count > 0)
                fprintf(stdout, "Generating C++ Clientstub to: %s\n", filename.c_str());
            stubgenerators.push_back(new CPPClientStubGenerator(cppclient->sval[0], procedures, filename));
        }

        if (jsclient->count > 0)
        {
            string filename;
            if (jsclientfile->count > 0)
                filename = jsclientfile->sval[0];
            else
                filename = JSClientStubGenerator::class2Filename(jsclient->sval[0]);

            if (verbose->count > 0)
                fprintf(stdout, "Generating JavaScript Clientstub to: %s\n", filename.c_str());
            stubgenerators.push_back(new JSClientStubGenerator(jsclient->sval[0], procedures, filename));
        }
    }
    catch (const JsonRpcException &ex)
    {
        fprintf(stderr, "%s\n", ex.what());
        arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));
        return false;
    }
    arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));
    return true;
}

void StubGeneratorFactory::deleteStubGenerators(std::vector<StubGenerator *> &stubgenerators)
{
    for (unsigned int i=0; i < stubgenerators.size(); ++i)
    {
        delete stubgenerators[i];
    }
}

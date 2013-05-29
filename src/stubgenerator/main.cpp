/**
 * @file main.cpp
 * @date 03.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief This application generates stubs for all specified methods
 * and notifications of a given file
 */

#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <cstring>

#include <jsonrpc/rpc.h>
#include <jsonrpc/procedure.h>

#include "clientstubgenerator.h"
#include "serverstubgenerator.h"

using namespace std;
using namespace jsonrpc;

void printHelp()
{
    cout << "jsonrpcstub Usage: jsonrpcstub {-s | -c} [-o /home/somepath/] inputfile stubname" << endl;
    cout << "Warning: stubname parameter may not contain any characters dissalowed in a C++ class name" << endl << endl;
    cout << "Options: " << endl;
    cout << "\t -h, --help\t\tPrint this menu" << endl;
    cout << "\t -s, --server\tCreate stub for server" << endl;
    cout << "\t -c, --client\tCreate stub for client" << endl;
    //   cout << "\t -b, --both\t\tCreate stub for client and server" << endl;
    cout << "\t -o, --output\tSpecify output path for stubfile. If not specified, outputpath will be CWD" << endl << endl;

    cout << "Example for client stub:" << endl;
    cout << "\tjsonrpcstub -c /home/user/procedures.json MyStub" << endl << endl;

    cout << "Example for server stub:" << endl;
    cout << "\tjsonrpcstub -s /home/user/procedures.json MyStub" << endl << endl;
}

int main(int argc, char** argv)
{
    if(argc == 1)
    {
        cerr << "Not enough arguments! Use --help for further info." << endl;
        return -1;
    }
    else if(argc < 3 && !(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
    {
        cerr << "Not enough arguments! Use --help for further info." << endl;
        return -1;
    }
    else
    {
        bool createClient = false;
        bool createServer = false;
        string inpath = string(argv[argc-2]);
        string stubname = string(argv[argc-1]);
        string outpath = "./";

        for(int i=1; i < argc-2; i++)
        {
            if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--server") == 0)
            {
                createServer = true;
            }
            else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--client") == 0)
            {
                createClient = true;
            }
            else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
                printHelp();
                return 0;
            }
            else if((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && i+1 < argc -2)
            {
                outpath = string(argv[++i]);
            }
            else
            {
                cerr << "Unrecognized option or missing parameter, use --help." << endl;
                return -1;
            }
        }

        if(argc == 2)
        {
            printHelp();
        }

        try
        {
            if(createClient)
            {
                ClientStubGenerator stub(stubname, inpath);
                cout << "Client Stub genearted to: " << stub.generateStubToFile(outpath) << endl;
            }
            if(createServer)
            {
                ServerStubGenerator stub(stubname, inpath);
                cout << "Server Stub genearted to: " + stub.generateStubToFile(outpath) << endl;
            }
        }
        catch(JsonRpcException e)
        {
            cerr << e.what() << endl;
        }
    }

    return 0;
}

/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    test_integration.cpp
 * @date    28.09.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE

#include <boost/test/unit_test.hpp>
#include <jsonrpccpp/server.h>
#include <jsonrpccpp/client.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <jsonrpccpp/client/connectors/httpclient.h>

#include <iostream>
#include "server.h"

using namespace jsonrpc;
using namespace std;

BOOST_AUTO_TEST_SUITE(integration)



BOOST_AUTO_TEST_SUITE_END()

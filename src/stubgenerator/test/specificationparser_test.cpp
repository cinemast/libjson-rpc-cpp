#include <catch.hpp>
#include <vector>
#include "module.h"
#include "../specificationparser.h"

using namespace std;
using namespace stubgenerator;
using namespace jsonrpc;
using Catch::Matchers::Contains;

TEST_CASE("invalid json", TEST_MODULE) {
    REQUIRE_THROWS_WITH(SpecificationParser::GetProceduresFromFile("testspec1.json"), Contains("JsonRpcException -32700") && Contains("specification file contains syntax errors"));
}


TEST_CASE("missing method name", TEST_MODULE) {
    vector<Procedure> procedures = SpecificationParser::GetProceduresFromFile("testspec2.json");
    REQUIRE_THROWS_WITH(SpecificationParser::GetProceduresFromFile("testspec2.json"), Contains("JsonRpcException -32700") && Contains("method name is missing for:"));
}
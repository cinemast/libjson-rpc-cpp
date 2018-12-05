#include <fstream>
#include <iomanip>
#include "../jsonparser.h"
#include "../exception.h"
#include "specificationparser.h"

using namespace std;
using namespace jsonrpc;
using namespace stubgenerator;

vector<Procedure>
SpecificationParser::GetProceduresFromFile(const string &filename) {
  string content;
  GetFileContent(filename, content);
  return GetProceduresFromString(content);
}
vector<Procedure>
SpecificationParser::GetProceduresFromString(const string &content) {

  Json::Reader reader;
  Json::Value val;
  if (!reader.parse(content, val)) {
    throw JsonRpcException(ExceptionCode::ERROR_INVALID_JSON,
                           " specification file contains syntax errors");
  }

  if (!val.isArray()) {
    throw JsonRpcException(ExceptionCode::ERROR_INVALID_JSON,
                           " top level json value is not an array");
  }

  vector<Procedure> result;
  map<string, int> procnames;
  for (unsigned int i = 0; i < val.size(); i++) {
    Procedure proc = GetProcedure(val[i]);
    if (procnames.find(proc.GetProcedureName()) != procnames.end()) {
      throw JsonRpcException(
          ExceptionCode::ERROR_INVALID_JSON,
          "Procedurename not unique: " + proc.GetProcedureName());
    }
    procnames[proc.GetProcedureName()] = 1;
    result.push_back(proc);
  }
  return result;
}
Procedure SpecificationParser::GetProcedure(Json::Value &signature) {
  if(!signature.isMember("name") || signature["name"].isString()) {
    throw JsonRpcException(ExceptionCode::ERROR_INVALID_JSON, "method name is missing for: " + signature.toStyledString());
  }

  Procedure result(signature["name"].asString());

  if (signature.isMember("returns")) {
    result.SetProcedureType(RPC_METHOD);
    result.SetReturnType(toJsonType(signature["returns"]));
  } else {
    result.SetProcedureType(RPC_NOTIFICATION);
  }

  if (signature.isMember("params")) {
     if (signature["params"].isObject()) {
       result.SetParameterDeclarationType(PARAMS_BY_NAME);
       GetNamedParameters(signature, result);
     } else if (signature["params"].isArray()) {
        result.SetParameterDeclarationType(PARAMS_BY_POSITION);
        GetPositionalParameters(signature, result);
     } else {
       throw JsonRpcException(ExceptionCode::ERROR_INVALID_JSON, "parms field is neither array nor object: " + signature.toStyledString());
     }
  }
  return result;
}

void SpecificationParser::GetFileContent(const std::string &filename, std::string &target) {
  ifstream config(filename.c_str());

  if (config) {
    config.open(filename.c_str(), ios::in);
    target.assign((std::istreambuf_iterator<char>(config)),
                  (std::istreambuf_iterator<char>()));
  } else {
    throw JsonRpcException(
        ExceptionCode::ERROR_INVALID_JSON, filename);
  }
}
jsontype_t SpecificationParser::toJsonType(Json::Value &val) {
  jsontype_t result;
  switch (val.type()) {
  case Json::uintValue:
  case Json::intValue:
    result = JSON_INTEGER;
    break;
  case Json::realValue:
    result = JSON_REAL;
    break;
  case Json::stringValue:
    result = JSON_STRING;
    break;
  case Json::booleanValue:
    result = JSON_BOOLEAN;
    break;
  case Json::arrayValue:
    result = JSON_ARRAY;
    break;
  case Json::objectValue:
    result = JSON_OBJECT;
    break;
  default:
    throw JsonRpcException(ExceptionCode::ERROR_INVALID_JSON, "Unknown parameter type: " + val.toStyledString());
  }
  return result;
}

void SpecificationParser::GetPositionalParameters(Json::Value &val, Procedure &result) {
  // Positional parameters
  for (unsigned int i = 0; i < val["params"].size(); i++) {
    stringstream paramname;
    paramname << "param" << std::setfill('0') << std::setw(2) << (i + 1);
    result.AddParameter(paramname.str(), toJsonType(val["params"][i]));
  }
}

void SpecificationParser::GetNamedParameters(Json::Value &val, Procedure &result) {
  vector<string> parameters = val["params"].getMemberNames();
  for (unsigned int i = 0; i < parameters.size(); ++i) {
    result.AddParameter(parameters.at(i), toJsonType(val["params"][parameters.at(i)]));
  }
}

#include "procedure.h"
#include "../exception.h"
#include "helper/cpphelper.h"
#include <cstdarg>
#include <vector>

using namespace std;
using namespace jsonrpc;

Procedure::Procedure(const std::string &name)
    : procedureName(name) {}

const parameterNameList_t &Procedure::GetParameters() const {
  return this->parametersName;
}
procedure_t Procedure::GetProcedureType() const { return this->procedureType; }
const std::string &Procedure::GetProcedureName() const {
  return this->procedureName;
}
parameterDeclaration_t Procedure::GetParameterDeclarationType() const {
  return this->paramDeclaration;
}
jsontype_t Procedure::GetReturnType() const { return this->returntype; }


void Procedure::SetProcedureType(procedure_t type) {
  this->procedureType = type;
}
void Procedure::SetReturnType(jsontype_t type) { this->returntype = type; }
void Procedure::SetParameterDeclarationType(parameterDeclaration_t type) {
  this->paramDeclaration = type;
}

void Procedure::AddParameter(const string &name, jsontype_t type) {
  this->parametersName[name] = type;
  this->parametersPosition.push_back(type);
}

string Procedure::ToString() {
  stringstream result;

  if (this->procedureType == RPC_NOTIFICATION)
    result << "void ";
  else
    result << CPPHelper::toCppReturntype(this->returntype) << " ";

  result << this->procedureName << "(";

  if (this->paramDeclaration == PARAMS_BY_NAME) {
    for(auto& param : this->parametersName) {
      result << param.first << ":" << CPPHelper::toCppParamType(param.second) << ", ";
    }
  } else {
    for (auto& param : this->parametersPosition) {
      result << CPPHelper::toCppParamType(param) << ", ";
    }
  }

  result << ")";
  return result.str();
}

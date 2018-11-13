#pragma once

namespace Json {
    class Value;
}

namespace jsonrpc {

    class Procedure;

    class IProcedureInvokationHandler {
        public:
            virtual ~IProcedureInvokationHandler() {}
            virtual void HandleMethodCall(Procedure& proc, const Json::Value& input, Json::Value& output) = 0;
            virtual void HandleNotificationCall(Procedure& proc, const Json::Value& input) = 0;
    };
}
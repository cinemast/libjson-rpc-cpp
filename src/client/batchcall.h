#pragma once

#include "../jsonparser.h"

namespace jsonrpc
{
    class BatchCall
    {
        public:
            BatchCall   ();

            /**
             * @brief addCall
             * @param methodname
             * @param params
             * @param isNotification
             * @return the id of the geneared request inside the batchcall
             */
            int         addCall     (const std::string &methodname, const Json::Value &params, bool isNotification = false);
            std::string toString    (bool fast = true) const;

        private:
            Json::Value result;
            int id;
    };
}
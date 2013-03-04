/**
 * @file clientconnector.h
 * @date 02.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief to be defined
 */

#ifndef CLIENTCONNECTOR_H_
#define CLIENTCONNECTOR_H_

#include <string>
#include "exception.h"

namespace jsonrpc
{
    
    class ClientConnector
    {
        public:
            virtual ~ClientConnector() {}

            /**
             * This method should take the message and send it via the concrete connector.
             * The result of the request must be returned as string.
             */
            virtual std::string SendMessage(const std::string& message) throw(Exception) = 0;
    };

} /* namespace jsonrpc */
#endif /* CLIENTCONNECTOR_H_ */

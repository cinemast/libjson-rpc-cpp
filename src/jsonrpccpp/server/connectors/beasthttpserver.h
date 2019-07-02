/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    beasthttpserver.h
 * @date    04.02.2019
 * @author  Javier Gutierrez (github: mr-j0nes)
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_BEASTHTTPSERVERCONNECTOR_H_
#define JSONRPC_CPP_BEASTHTTPSERVERCONNECTOR_H_

#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>


#if defined(_WIN32) && !defined(__CYGWIN__)
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#endif

#include <map>
#include <microhttpd.h>
#include "../abstractserverconnector.h"

namespace jsonrpc
{

    class BeastHttpServer;

namespace beast
{

    // Handles an HTTP server connection
    class session : public std::enable_shared_from_this<session>
    {
        // This is the C++11 equivalent of a generic lambda.
        // The function object is used to send an HTTP message.
        struct send_lambda
        {
            session& self_;

            explicit
                send_lambda(session& self)
                : self_(self)
                {
                }

            template<bool isRequest, class Body, class Fields>
                void
                operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg) const
                {
                    // The lifetime of the message has to extend
                    // for the duration of the async operation so
                    // we use a shared_ptr to manage it.
                    auto sp = std::make_shared<
                        boost::beast::http::message<isRequest, Body, Fields>>(std::move(msg));

                    // Store a type-erased version of the shared
                    // pointer in the class to keep it alive.
                    self_.res_ = sp;

                    // Write the response
                    boost::beast::http::async_write(
                            self_.socket_,
                            *sp,
                            boost::asio::bind_executor(
                                self_.strand_,
                                std::bind(
                                    &session::on_write,
                                    self_.shared_from_this(),
                                    std::placeholders::_1,
                                    std::placeholders::_2,
                                    sp->need_eof())));
                }
        };

        BeastHttpServer*                                                server_;
        boost::asio::ip::tcp::socket                                    socket_;
        boost::asio::strand<
            boost::asio::io_context::executor_type>                     strand_;
        boost::beast::flat_buffer                                       buffer_;
        boost::beast::http::request<boost::beast::http::string_body>    req_;
        std::shared_ptr<void>                                           res_;
        send_lambda                                                     lambda_;

    public:
        // Take ownership of the socket
        explicit session(BeastHttpServer* server, boost::asio::ip::tcp::socket socket);

        // Start the asynchronous operation
        void run();

        void do_read();

        void on_read(
                    boost::beast::error_code ec,
                    std::size_t bytes_transferred);

        void on_write(
                    boost::beast::error_code ec,
                    std::size_t bytes_transferred,
                    bool close);

        void do_close();

        // This function produces an HTTP response for the given
        // request. The type of the response object depends on the
        // contents of the request, so the interface requires the
        // caller to pass a generic lambda for receiving the response.
        void handle_request(
                boost::beast::http::request<boost::beast::http::string_body> req,
                send_lambda& send);

    };


    // Accepts incoming connections and launches the sessions
    class listener : public std::enable_shared_from_this<listener>
    {
        BeastHttpServer*                                server_;
        boost::asio::ip::tcp::acceptor                  acceptor_;
        boost::asio::ip::tcp::socket                    socket_;

    public:
        listener(
                BeastHttpServer* server,
                boost::asio::io_context& ioc,
                boost::asio::ip::tcp::endpoint endpoint);
        

        // Start accepting incoming connections
        void run();

        void stop();

        void do_accept();

        void on_accept(boost::beast::error_code ec);
    };

} /* namespace beast */

    /**
     * Provides an HTTP Server based on boost::beast to handle incoming requests and send back HTTP 1.1 valid responses.
     * Note that this class will always send HTTP-Status 200, even though an JSON-RPC Error might have occurred. Please
     * always check for the JSON-RPC Error Header.
     */
    class BeastHttpServer: public AbstractServerConnector
    {
        public:

            /**
             * @brief BeastHttpServer, constructor for the included BeastHttpServer
             * @param port on which the server is listening
             * @param enableSpecification - defines if the specification is returned in case of a GET request
             */
            BeastHttpServer(uint16_t port, uint8_t threads = 50);

            ~BeastHttpServer() 
            {}

            virtual bool StartListening();
            virtual bool StopListening();

            // TODO Do we need these two ?
            void SetUrlHandler(const std::string &url, IClientConnectionHandler *handler);
            IClientConnectionHandler* GetHandler(const std::string &url);

        private:
            std::string                                         address_;
            uint16_t                                            port_;
            uint8_t                                             threads_;
            bool                                                running_;
            boost::asio::io_context                             ioc_;
            std::vector<std::thread>                            threadVector_;

            std::map<std::string, IClientConnectionHandler*> urlhandler;

    };

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_BEASTHTTPSERVERCONNECTOR_H_ */

/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    beasthttpserver.cpp
 * @date    04.02.2019
 * @author  Javier Gutierrez (github: mr-j0nes)
 * @license See attached LICENSE.txt
 ************************************************************************/

#include <thread>

#include "beasthttpserver.h"

#include <jsonrpccpp/common/specificationparser.h>


using namespace jsonrpc;
using namespace std;


beast::serverSession::serverSession(BeastHttpServer* server, boost::asio::ip::tcp::socket&& socket)
    : server_(server)
#if BOOST_VERSION < 107000
    , socket_(std::move(socket))
    , strand_(socket_.get_executor())
#else
    , stream_(std::move(socket))
#endif
    , lambda_(*this)
{ }

// Start the asynchronous operation
void beast::serverSession::run()
{
    do_read();
}

void beast::serverSession::do_read()
{
    // Make the request empty before reading,
    // otherwise the operation behavior is undefined.
    req_ = {};

    // Read a request
    boost::beast::http::async_read(
#if BOOST_VERSION < 107000
            socket_, buffer_, req_,
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &serverSession::on_read,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2))
#else
            stream_, buffer_, req_,
            boost::beast::bind_front_handler(
                &serverSession::on_read,
                shared_from_this())
#endif
            );
}

void beast::serverSession::on_read(
    boost::beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if(ec == boost::beast::http::error::end_of_stream)
        return do_close();

    if(ec)
        throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR, "read :" + ec.message());

    // Send the response
    handle_request(std::move(req_), lambda_);
}

#if BOOST_VERSION < 107000
void beast::serverSession::on_write(
    boost::beast::error_code ec,
    std::size_t bytes_transferred,
    bool close)
#else
void beast::serverSession::on_write(
    bool close,
    boost::beast::error_code ec,
    std::size_t bytes_transferred)
#endif
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR, "write :" + ec.message());

    if(close)
    {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return do_close();
    }

    // We're done with the response so delete it
    res_ = nullptr;

    // Read another request
    do_read();
}

void beast::serverSession::do_close()
{
    // Send a TCP shutdown
    boost::beast::error_code ec;
#if BOOST_VERSION < 107000
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
#else
    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
#endif

    // At this point the connection is closed gracefully
}

void beast::serverSession::handle_request(
    boost::beast::http::request<boost::beast::http::string_body>&& req,
    beast::serverSession::send_lambda send)
{
    // Returns a bad request response
    auto const bad_request =
    [&req](boost::beast::string_view why)
    {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::bad_request, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Returns a method not allowed response
    auto const method_not_allowed =
    [&req](boost::beast::string_view why)
    {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::method_not_allowed, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error =
    [&req](boost::beast::string_view what)
    {
        boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::internal_server_error, req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if( req.method() == boost::beast::http::verb::get)
        return send(
                method_not_allowed("Unknown HTTP-method")
                );
    else if( req.method() != boost::beast::http::verb::post)
        return send(
                bad_request("Unknown HTTP-method")
                );




    std::string response;
    IClientConnectionHandler *handler =
        server_->GetHandler(string("/"));
    if (handler == NULL) {
        return send(server_error("No client connection handler found"));
    } else {

        handler->HandleRequest(req.body().data(), response);
        boost::beast::http::string_body::value_type body = response;


        auto const size = body.size();

        // Respond to POST request
        boost::beast::http::response<boost::beast::http::string_body> res{
            std::piecewise_construct,
                std::make_tuple(std::move(body)),
                std::make_tuple(boost::beast::http::status::ok, req.version())};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        // res.set(boost::beast::http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

}

beast::listener::listener(
    BeastHttpServer* server,
    boost::asio::io_context& ioc,
    boost::asio::ip::tcp::endpoint endpoint)
    : server_(server)
#if BOOST_VERSION < 107000
    , acceptor_(ioc)
    , socket_(ioc)
#else
    , ioc_(ioc)
    , acceptor_(boost::asio::make_strand(ioc))
#endif
{
    boost::beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
        throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR, "open :" + ec.message());
        return;
    }

    // Allow address reuse
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if(ec)
    {
        throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR, "set_option :" + ec.message());
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec)
    {
        throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR, "bind :" + ec.message());
        return;
    }

    // Start listening for connections
    acceptor_.listen(
        boost::asio::socket_base::max_listen_connections, ec);
    if(ec)
    {
        throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR, "listen :" + ec.message());
        return;
    }
}

// Start accepting incoming connections
void beast::listener::run()
{
#if BOOST_VERSION < 107000
    if(! acceptor_.is_open())
        return;
#else
#endif
    do_accept();
}

void beast::listener::do_accept()
{
    acceptor_.async_accept(
#if BOOST_VERSION < 107000
            socket_,
            std::bind(
                &listener::on_accept,
                shared_from_this(),
                std::placeholders::_1)
#else
            boost::asio::make_strand(ioc_),
            boost::beast::bind_front_handler(
                &listener::on_accept,
                shared_from_this())
#endif
            );
}

void beast::listener::on_accept(boost::beast::error_code ec
#if BOOST_VERSION < 107000
#else
        , boost::asio::ip::tcp::socket socket
#endif
        )
{
    if(ec)
    {
        throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR, "accept :" + ec.message());
    }
    else
    {
        // Create the session and run it
        std::make_shared<serverSession>(server_,
            std::move(
#if BOOST_VERSION < 107000
                socket_
#else
                socket
#endif
                ))->run();
    }

    // Accept another connection
    do_accept();
}

void beast::listener::stop()
{
    if(acceptor_.is_open())
    {
        acceptor_.close();
    }
}






BeastHttpServer::BeastHttpServer(uint16_t port, uint8_t threads)
    : AbstractServerConnector()
    , address_("127.0.0.1")
    , port_(port)
    , threads_(threads)
    , running_(false)
    , ioc_(threads)
{ }

IClientConnectionHandler *BeastHttpServer::GetHandler(const std::string &url) {
    if (AbstractServerConnector::GetHandler() != NULL)
        return AbstractServerConnector::GetHandler();
    map<string, IClientConnectionHandler *>::iterator it =
        this->urlhandler.find(url);
    if (it != this->urlhandler.end())
        return it->second;
    return NULL;
}

bool BeastHttpServer::StartListening() {
    if (! this->running_) {

        // TODO move ioc to listener ?

        // Create and launch a listening port
        std::make_shared<beast::listener>(
            this,
            ioc_,
            boost::asio::ip::tcp::endpoint{boost::asio::ip::make_address(address_), port_})->run();

        // Run the I/O service on the requested number of threads
        threadVector_.reserve(threads_);
        for(auto i = threads_; i > 0; --i)
        {
            threadVector_.emplace_back(
            [this]
            {
                this->ioc_.run();
            });
        }

        this->running_ = true;
    }
    return this->running_;
}

bool BeastHttpServer::StopListening() {
    ioc_.stop();

    for (auto& thread : threadVector_)
    {
        thread.join();
    }

    if (! ioc_.stopped())
        return false;
    return true;
}

void BeastHttpServer::SetUrlHandler(const string &url,
        IClientConnectionHandler *handler) {
    this->urlhandler[url] = handler;
    this->SetHandler(NULL);
}

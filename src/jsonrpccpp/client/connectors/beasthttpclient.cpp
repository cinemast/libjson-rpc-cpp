/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    beasthttpclient.cpp
 * @date    02.02.2019
 * @author  Javier Gutierrez (github mr-j0nes)
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "beasthttpclient.h"
#include <string>
#include <boost/beast/version.hpp>

#include <iostream>


using namespace jsonrpc;


beast::session::session(boost::asio::io_context& ioc, long timeout,
        std::function<void(const std::string&, uint16_t)> callback)
    : resolver_(ioc)
    , socket_(ioc)
    , timer_(ioc, (std::chrono::steady_clock::time_point::max)())
    , timeout_(timeout)
    , callback_(callback)
{}

//
void beast::session::on_timer(boost::beast::error_code ec)
{
    if(ec && ec != boost::asio::error::operation_aborted)
    {
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "timer :" + ec.message());
    }

    // Verify that the timer really expired since the deadline may have moved.
    if(timer_.expiry() <= std::chrono::steady_clock::now())
    {
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
        return;
    }

    // Wait on the timer
    timer_.async_wait(
                std::bind(
                    &beast::session::on_timer, // Executed always when "timer_.execute_after()"
                    shared_from_this(),
                    std::placeholders::_1));

}

// Start the asynchronous operation
void beast::session::run( const std::string& host, uint16_t port, const std::string& message)
{
    // Run the timer. The timer is operated
    // continuously, this simplifies the code.
    on_timer({});

    // Set up an HTTP POST message
    req_.version(11);
    req_.method(boost::beast::http::verb::post);
    req_.target("/");
    req_.set(boost::beast::http::field::host, host);
    req_.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    res_.set(boost::beast::http::field::content_type, "application/json");
    res_.set(boost::beast::http::field::accept, "application/json");
    res_.set(boost::beast::http::field::accept_charset, "utf-8");
    req_.body() = message;
    req_.prepare_payload(); // Sets the size

    // Set the timer
    timer_.expires_after(std::chrono::milliseconds(timeout_));

    // Look up the domain name
    resolver_.async_resolve(
        host,
        std::to_string(port),
        std::bind(
            &session::on_resolve,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
}

void beast::session::on_resolve(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::resolver::results_type results)
{
    if(ec)
    {
        timer_.expires_after(std::chrono::milliseconds(0));
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "resolve :" + ec.message());
    }

    // Set the timer
    timer_.expires_after(std::chrono::milliseconds(timeout_));

    // Make the connection on the IP address we get from a lookup
    boost::asio::async_connect(
        socket_,
        results.begin(),
        results.end(),
        std::bind(
            &session::on_connect,
            shared_from_this(),
            std::placeholders::_1));
}

void beast::session::on_connect(boost::beast::error_code ec)
{
    if(ec)
    {
        timer_.expires_after(std::chrono::milliseconds(0));
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "connect :" + ec.message());
    }

    // Write request to cout
    if (debug_)
        std::cout << req_ << std::endl;

    // Set the timer
    timer_.expires_after(std::chrono::milliseconds(timeout_));

    // Send the HTTP request to the remote host
    boost::beast::http::async_write(socket_, req_,
        std::bind(
            &session::on_write,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
}

void beast::session::on_write(
    boost::beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        timer_.expires_after(std::chrono::milliseconds(0));
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "write :" + ec.message());
    }
    
    // Set the timer
    timer_.expires_after(std::chrono::milliseconds(timeout_));

    // Receive the HTTP response
    boost::beast::http::async_read(socket_, buffer_, res_,
        std::bind(
            &session::on_read,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
}

void beast::session::on_read(
    boost::beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
    {
        timer_.expires_after(std::chrono::milliseconds(0));
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "read :" + ec.message());
    }

    // Write the message to standard out
    if (debug_)
        std::cout << res_ << std::endl;

    // Send response back to caller
    callback_(res_.body().data(), res_.result_int());

    // Gracefully close the socket
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes so don't bother reporting it.
    if(ec && ec != boost::beast::errc::not_connected)
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, "shutdown :" + ec.message());

    // If we get here then the connection is closed gracefully
    timer_.expires_after(std::chrono::milliseconds(0)); // releases the timer
}

void beast::session::setDebug(bool flag)
{
    debug_ = flag;
}

BeastHttpClient::BeastHttpClient(const std::string &host, uint16_t port) : 
    host_(host), 
    port_(port) ,
    timeout_(10000)
{ }

BeastHttpClient::~BeastHttpClient() 
{ }

void BeastHttpClient::ReceiveRPCMessage(const std::string& result, uint16_t statusCode)
{
    result_ = result;
    statusCode_ = statusCode;

}

void BeastHttpClient::SendRPCMessage(const std::string &message,
                                std::string& result) {

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // Launch the asynchronous operation
    beastSession_ = std::make_shared<beast::session>(ioc, timeout_,
            std::bind(&BeastHttpClient::ReceiveRPCMessage, this, std::placeholders::_1, std::placeholders::_2));

    beastSession_->setDebug(false);

    beastSession_->run(host_, port_, message);

    // Run the I/O service. The call will return when
    // the get operation is complete.
    ioc.run();

    result = std::move(result_);

  if (statusCode_ != 200) {
    throw JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR, result);
  }
}

//
void BeastHttpClient::SetTimeout(long timeout) { this->timeout_ = timeout; }

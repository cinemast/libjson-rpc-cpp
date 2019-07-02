/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    beasthttpclient.h
 * @date    02.02.2019
 * @author  Javier Gutierrez (github: mr-j0nes)
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_BEASTHTTPCLIENT_H_
#define JSONRPC_CPP_BEASTHTTPCLIENT_H_

#include "../iclientconnector.h"
#include <jsonrpccpp/common/exception.h>
#include <map>
#include <functional>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>


namespace jsonrpc {

namespace beast {

/**
 * @brief Boost::Beast asynchrounous HTTP client.
 */
class session : public std::enable_shared_from_this<session>
{
    boost::asio::ip::tcp::resolver                                      resolver_;
    boost::asio::ip::tcp::socket                                        socket_;
    boost::asio::steady_timer                                           timer_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
    boost::beast::http::request<boost::beast::http::string_body>        req_;
    boost::beast::http::response<boost::beast::http::string_body>       res_;

    long                                                                timeout_;
    std::function<void(const std::string &, uint16_t statusCode)>       callback_;
    bool                                                                debug_              {false};

public:
    explicit session(boost::asio::io_context& ioc, long timeout,
            std::function<void(const std::string&, uint16_t statusCode)> callback);

    void on_timer(boost::beast::error_code ec);

    void run(const std::string& host, uint16_t port, const std::string& message);

    void on_resolve( boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results);

    void on_connect(boost::beast::error_code ec);

    void on_write( boost::beast::error_code ec, std::size_t bytes_transferred);

    void on_read( boost::beast::error_code ec, std::size_t bytes_transferred);

    void setDebug(bool flag);
};
} /* namespace beast */



class BeastHttpClient : public IClientConnector {
public:
  BeastHttpClient(const std::string &host, uint16_t port);
  virtual ~BeastHttpClient();
  virtual void SendRPCMessage(const std::string &message, std::string &result);

  void SetHost(const std::string &host);
  void SetPort(uint16_t port);
  void SetTimeout(long timeout);

private:
  void ReceiveRPCMessage(const std::string &result, uint16_t statusCode);

private:
  std::map<std::string, std::string>    headers_;
  std::string                           host_;
  std::string                           result_;
  uint16_t                              port_;
  uint16_t                              statusCode_;
  std::shared_ptr<beast::session>       beastSession_;

  /**
   * @brief timeout for http request in milliseconds
   */
  long                                  timeout_;
};

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_BEASTHTTPCLIENT_H_ */

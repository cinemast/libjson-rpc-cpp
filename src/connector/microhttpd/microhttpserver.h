#pragma once

#include <microhttpd.h>
#include <map>
#include <sstream>
#include "../abstractserverconnector.h"

namespace jsonrpc {
  class MicroHttpServer;

  struct mhd_coninfo {
    struct MHD_PostProcessor *postprocessor;
    MHD_Connection *connection;
    std::stringstream request;
    MicroHttpServer *server;
    int code;
  };

  /**
   * This class provides an embedded HTTP Server, based on libmicrohttpd, to
   * handle incoming Requests and send HTTP 1.1 valid responses. Note that this
   * class will always send HTTP-Status 200, even though an JSON-RPC Error might
   * have occurred. Please always check for the JSON-RPC error field.
   */
  class MicroHttpServer : public AbstractServerConnector {
   public:
    /**
     * @brief MicroHttpServer, constructor for the included libmicrohttpd based HttpServer
     * @param port on which the server is listening
     */
    MicroHttpServer(int port, ConnectionHandlers handlers);
    virtual ~MicroHttpServer();

    /**
     * @brief enable TLS support of libmicrohttpd through gnutls.
     * @param sslcert defines the path to a SSL certificate
     * @param sslcert defines the path to a SSL key
     * @return true on success, false if one of the paths was invalid or libmicrohttpd does not support TLS
     */
    bool EnableTLS(const std::string &sslcert, const std::string &sslkey);

    virtual bool StartListening();
    virtual bool StopListening();

   protected:
    virtual bool SendResponse(const std::string &response, struct mhd_coninfo *client_connection = NULL);
    virtual bool SendOptionsResponse(struct mhd_coninfo *client_connection);

   private:
    int port;
    bool running;
    bool enableTLS;
    std::string sslcert;
    std::string sslkey;

    struct MHD_Daemon *daemon;

    static int callback(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                        const char *version, const char *upload_data, size_t *upload_data_size,
                        void **con_cls);
  };
}

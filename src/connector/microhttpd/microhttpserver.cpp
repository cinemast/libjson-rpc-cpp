#include "microhttpserver.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

using namespace jsonrpc;
using namespace std;

#define BUFFERSIZE 65536

std::string GetFileContent(const string &filename) {
  string target;
  ifstream config(filename.c_str());

  if (config) {
    config.open(filename.c_str(), ios::in);
    target.assign((std::istreambuf_iterator<char>(config)), (std::istreambuf_iterator<char>()));
  } else {
    target = "";
  }
  return target;
}

MicroHttpServer::MicroHttpServer(int port, IClientConnectionHandler& handler)
    : handler(handler),
      port(port),
      running(false),
      enableTLS(false),
      sslcert(""),
      sslkey(""),
      daemon(NULL) {}

MicroHttpServer::~MicroHttpServer() { this->StopListening(); }

bool MicroHttpServer::EnableTLS(const std::string &sslcert, const std::string &sslkey) {
  if (this->running || MHD_is_feature_supported(MHD_FEATURE_SSL) != MHD_YES) {
    return false;
  }

  this->sslcert = GetFileContent(sslcert);
  this->sslkey = GetFileContent(sslkey);

  if (this->sslcert != "" && this->sslkey != "") {
    this->enableTLS = true;
    return true;
  }

  return false;
}

bool MicroHttpServer::StartListening() {
  unsigned int mhd_flags = MHD_USE_THREAD_PER_CONNECTION;
  if (!this->running) {
    if (this->enableTLS) {
      mhd_flags |= MHD_USE_SSL;
      this->daemon = MHD_start_daemon(mhd_flags, this->port, NULL, NULL, MicroHttpServer::callback, this,
                                      MHD_OPTION_HTTPS_MEM_KEY, this->sslkey.c_str(),
                                      MHD_OPTION_HTTPS_MEM_CERT, this->sslcert.c_str(), MHD_OPTION_END);
    } else {
      this->daemon = MHD_start_daemon(mhd_flags, this->port, NULL, NULL, MicroHttpServer::callback, this,
                                      MHD_OPTION_END);
    }

    if (this->daemon != NULL) {
      this->running = true;
    }
    return this->running;
  }
  return false;
}

bool MicroHttpServer::StopListening() {
  if (this->running) {
    MHD_stop_daemon(this->daemon);
    this->running = false;
    return true;
  }
  return false;
}

bool MicroHttpServer::SendResponse(const string &response, struct mhd_coninfo *client_connection) {
  struct MHD_Response *result =
      MHD_create_response_from_buffer(response.size(), (void *)response.c_str(), MHD_RESPMEM_MUST_COPY);

  MHD_add_response_header(result, "Content-Type", "application/json");
  MHD_add_response_header(result, "Access-Control-Allow-Origin", "*");

  int ret = MHD_queue_response(client_connection->connection, client_connection->code, result);
  MHD_destroy_response(result);
  return ret == MHD_YES;
}

bool MicroHttpServer::SendOptionsResponse(struct mhd_coninfo *client_connection) {
  struct MHD_Response *result = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_MUST_COPY);

  MHD_add_response_header(result, "Allow", "POST, OPTIONS");
  MHD_add_response_header(result, "Access-Control-Allow-Origin", "*");
  MHD_add_response_header(result, "Access-Control-Allow-Headers", "origin, content-type, accept");
  MHD_add_response_header(result, "DAV", "1");

  int ret = MHD_queue_response(client_connection->connection, client_connection->code, result);
  MHD_destroy_response(result);
  return ret == MHD_YES;
}

int MicroHttpServer::callback(void *cls, MHD_Connection *connection, const char *url, const char *method,
                              const char *version, const char *upload_data, size_t *upload_data_size,
                              void **con_cls) {
  (void)version;
  (void)url;
  if (*con_cls == NULL) {
    struct mhd_coninfo *client_connection = new mhd_coninfo;
    client_connection->connection = connection;
    client_connection->server = static_cast<MicroHttpServer *>(cls);
    *con_cls = client_connection;
    return MHD_YES;
  }
  struct mhd_coninfo *client_connection = static_cast<struct mhd_coninfo *>(*con_cls);

  if (string("POST") == method) {
    if (*upload_data_size != 0) {
      client_connection->request.write(upload_data, *upload_data_size);
      *upload_data_size = 0;
      return MHD_YES;
    } else {
      string response;
      client_connection->server->handler.HandleRequest(client_connection->request.str(), response);
      client_connection->code = MHD_HTTP_OK;
      client_connection->server->SendResponse(response, client_connection);
    }
  } else if (string("OPTIONS") == method) {
    client_connection->code = MHD_HTTP_OK;
    client_connection->server->SendOptionsResponse(client_connection);
  } else {
    client_connection->code = MHD_HTTP_METHOD_NOT_ALLOWED;
    client_connection->server->SendResponse("Not allowed HTTP Method", client_connection);
  }
  delete client_connection;
  *con_cls = NULL;
  return MHD_YES;
}

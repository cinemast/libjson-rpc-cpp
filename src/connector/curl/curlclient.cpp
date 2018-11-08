
#include "curlclient.h"
#include <curl/curl.h>
#include <sstream>
#include <string>
#include "../../exception.h"

using namespace jsonrpc;
using namespace std;

// Rationale, see here: http://curl.haxx.se/libcurl/c/curl_global_init.html
class curl_initializer {
 public:
  curl_initializer() { curl_global_init(CURL_GLOBAL_ALL); }
  ~curl_initializer() { curl_global_cleanup(); }
};
static curl_initializer _curl_init = curl_initializer();

CurlClient::CurlClient(const string &url, long timeout, bool verifyTLS)
    : url(url), timeout(timeout), verifyTLS(verifyTLS) {
  this->AddHeader("Content-Type", "application/json");
  this->AddHeader("charsets", "utf-8");
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

string CurlClient::SendRPCMessage(const string &message) {
  CURL *curl = curl_easy_init();
  string response = "";

  struct curl_slist *headers = NULL;
  for (auto header : this->headers) {
    headers = curl_slist_append(headers, (header.first + ": " + header.second).c_str());
  }

  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, this->verifyTLS);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, this->verifyTLS); 

  CURLcode res = curl_easy_perform(curl);
  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

 if (res != CURLE_OK) {
    stringstream message;
    message << "libcurl error " << res << ", " << curl_easy_strerror(res);
    throw JsonRpcException(ExceptionCode::ERROR_CLIENT_CONNECTOR, message.str());
  } else if (http_code != 200) {
    stringstream message;
    message << "Received HTTP status code " << http_code << ": " << response;
    throw JsonRpcException(ExceptionCode::ERROR_CLIENT_CONNECTOR, message.str());
  }

  return response;
}

void CurlClient::AddHeader(const std::string &attr, const std::string &val) { this->headers[attr] = val; }
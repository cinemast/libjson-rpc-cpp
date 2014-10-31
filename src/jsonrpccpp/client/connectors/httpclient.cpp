/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    httpclient.cpp
 * @date    02.01.2013
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "httpclient.h"
#include <curl/curl.h>
#include <string>
#include <string.h>
#include <cstdlib>

#include <iostream>

using namespace jsonrpc;

/**
 * taken from http://stackoverflow.com/questions/2329571/c-libcurl-get-output-into-a-string
 */
struct string
{
    char *ptr;
    size_t len;
};

/**
  * Callback for libcurl
  */
static size_t writefunc(void *ptr, size_t size, size_t nmemb,
                        struct string *s)
{
    size_t new_len = s->len + size * nmemb;
    s->ptr = (char*) realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL)
    {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

void init_string(struct string *s)
{
    s->len = 0;
    s->ptr = (char*) malloc(s->len + 1);
    if (s->ptr == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

HttpClient::HttpClient(const std::string& url) throw(JsonRpcException)
    : url(url)
{
    this->timeout = 10000;
}

void HttpClient::SendRPCMessage(const std::string& message, std::string& result) throw (JsonRpcException)
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, ": libcurl initialization error");
    }

    curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);

    CURLcode res;

    struct string s;
    init_string(&s);

    struct curl_slist * headers = NULL;
    //Maybe to restrictive
    //headers = curl_slist_append(headers, "Accept: application/json");
    for (std::map<std::string, std::string>::iterator header = this->headers.begin(); header != this->headers.end(); ++header) {
        headers = curl_slist_append(headers, (header->first + ": " + header->second).c_str());
    }

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charsets: utf-8");

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);

    res = curl_easy_perform(curl);

    result = s.ptr;
    free(s.ptr);
    curl_slist_free_all(headers);
    if (res != CURLE_OK)
    {
        std::stringstream str;
        switch(res)
        {
            case 7: str << "Could not connect to " << this->url;
                break;
            case 28: str << "Operation timed out";
                break;
            default:
                str << "libcurl error: " << res;
                break;
        }
        curl_easy_cleanup(curl);
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, str.str());
    }

    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200)
    {
        throw JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR, result);
    }

    if (curl)
    {
        curl_easy_cleanup(curl);
    }
}

void HttpClient::SetUrl(const std::string& url)
{
    this->url = url;
}

void HttpClient::SetTimeout(long timeout)
{
    this->timeout = timeout;
}

void HttpClient::AddHeader(const std::string attr, const std::string val) {
    this->headers[attr] = val;
}

void HttpClient::RemoveHeader(const std::string attr) {
    this->headers.erase(attr);
}

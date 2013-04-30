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

using namespace std;

namespace jsonrpc
{
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
        : AbstractClientConnector(), url(url)
    {
        curl = curl_easy_init();
        if (!curl)
        {
            throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, ": libcurl initialization error");
        }

        curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    }
    
    HttpClient::~HttpClient()
    {
        if (curl)
        {
            curl_easy_cleanup(curl);
        }
    }

    void HttpClient::SendMessage(const std::string& message, std::string& result) throw (JsonRpcException)
    {
        CURLcode res;

        struct string s;
        init_string(&s);

        struct curl_slist * headers = NULL;
        //Maybe to restrictive
        //headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "charsets: utf-8");

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        result = s.ptr;
        free(s.ptr);
        curl_slist_free_all(headers);
        if (res != CURLE_OK)
        {
            stringstream str;
            if(res == 7)
            {
                str << ": Could not connect to " << this->url;
            }
            else
            {
                str << ": libcurl error: " << res;
            }
            throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, str.str());
        }
    }

    void HttpClient::SetUrl(const std::string& url)
    {
        this->url = url;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    }

} /* namespace jsonrpc */

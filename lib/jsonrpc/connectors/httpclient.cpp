/**
 * @file httpclient.cpp
 * @date 02.01.2013
 * @author Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @brief This class wraps libcurl.
 */

#include "httpclient.h"

#include <curl/curl.h>
#include <string>
#include <string.h>

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
    
    HttpClient::HttpClient(const std::string& url)
            : url(url)
    {
        curl = curl_easy_init();
        if (!curl)
        {
            //TODO: throw exception
            cerr << "error constructing httpclient" << endl;
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

    std::string HttpClient::SendMessage(const std::string& message)
    {
        std::string result = "";
        CURLcode res;

        struct string s;
        init_string(&s);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        res = curl_easy_perform(curl);

        result = s.ptr;
        free(s.ptr);
        if (res != CURLE_OK)
        {
            //TODO: throw exception
            cerr << "error in libcurl: " << res << endl;
        }

        return result;
    }

    void HttpClient::SetUrl(const std::string& url)
    {
       // this->url = url;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    }

} /* namespace jsonrpc */

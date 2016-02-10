/*
     This file is part of libmicrohttpd
     Copyright (C) 2007 Christian Grothoff

     libmicrohttpd is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 2, or (at your
     option) any later version.

     libmicrohttpd is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with libmicrohttpd; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
*/

/**
 * @file test_post.c
 * @brief  Testcase for libmicrohttpd POST operations using URL-encoding
 * @author Christian Grothoff
 */

#include "MHD_config.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef WINDOWS
#include <unistd.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif /* !WIN32_LEAN_AND_MEAN */
#include <windows.h>
#endif

#if defined(CPU_COUNT) && (CPU_COUNT+0) < 2
#undef CPU_COUNT
#endif
#if !defined(CPU_COUNT)
#define CPU_COUNT 2
#endif

#define POST_DATA "name=daniel&project=curl"

static int oneone;

struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};


static void
completed_cb (void *cls,
	      struct MHD_Connection *connection,
	      void **con_cls,
	      enum MHD_RequestTerminationCode toe)
{
  struct MHD_PostProcessor *pp = *con_cls;

  if (NULL != pp)
    MHD_destroy_post_processor (pp);   
  *con_cls = NULL;
}


static size_t
copyBuffer (void *ptr, size_t size, size_t nmemb, void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->pos + size * nmemb > cbc->size)
    return 0;                   /* overflow */
  memcpy (&cbc->buf[cbc->pos], ptr, size * nmemb);
  cbc->pos += size * nmemb;
  return size * nmemb;
}


/**
 * Note that this post_iterator is not perfect
 * in that it fails to support incremental processing.
 * (to be fixed in the future)
 */
static int
post_iterator (void *cls,
               enum MHD_ValueKind kind,
               const char *key,
               const char *filename,
               const char *content_type,
               const char *transfer_encoding,
               const char *value, uint64_t off, size_t size)
{
  int *eok = cls;

  if ((0 == strcmp (key, "name")) &&
      (size == strlen ("daniel")) && (0 == strncmp (value, "daniel", size)))
    (*eok) |= 1;
  if ((0 == strcmp (key, "project")) &&
      (size == strlen ("curl")) && (0 == strncmp (value, "curl", size)))
    (*eok) |= 2;
  return MHD_YES;
}


static int
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **unused)
{
  static int eok;
  struct MHD_Response *response;
  struct MHD_PostProcessor *pp;
  int ret;

  if (0 != strcmp ("POST", method))
    {
      printf ("METHOD: %s\n", method);
      return MHD_NO;            /* unexpected method */
    }
  pp = *unused;
  if (pp == NULL)
    {
      eok = 0;
      pp = MHD_create_post_processor (connection, 1024, &post_iterator, &eok);
      *unused = pp;
    }
  MHD_post_process (pp, upload_data, *upload_data_size);
  if ((eok == 3) && (0 == *upload_data_size))
    {
      response = MHD_create_response_from_buffer (strlen (url),
						  (void *) url,
						  MHD_RESPMEM_MUST_COPY);
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
      MHD_destroy_post_processor (pp);
      *unused = NULL;
      return ret;
    }
  *upload_data_size = 0;
  return MHD_YES;
}


static int
testInternalPost ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
                        1080, NULL, NULL, &ahc_echo, NULL, 
			MHD_OPTION_NOTIFY_COMPLETED, &completed_cb, NULL,			
			MHD_OPTION_END);
  if (d == NULL)
    return 1;
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1:1080/hello_world");
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_POSTFIELDS, POST_DATA);
  curl_easy_setopt (c, CURLOPT_POSTFIELDSIZE, strlen (POST_DATA));
  curl_easy_setopt (c, CURLOPT_POST, 1L);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  // NOTE: use of CONNECTTIMEOUT without also
  //   setting NOSIGNAL results in really weird
  //   crashes on my system!
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1);
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
    {
      fprintf (stderr,
               "curl_easy_perform failed: `%s'\n",
               curl_easy_strerror (errornum));
      curl_easy_cleanup (c);
      MHD_stop_daemon (d);
      return 2;
    }
  curl_easy_cleanup (c);
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
    return 4;
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
    return 8;
  return 0;
}

static int
testMultithreadedPost ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
                        1081, NULL, NULL, &ahc_echo, NULL, 
			MHD_OPTION_NOTIFY_COMPLETED, &completed_cb, NULL,			
			MHD_OPTION_END);
  if (d == NULL)
    return 16;
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1:1081/hello_world");
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_POSTFIELDS, POST_DATA);
  curl_easy_setopt (c, CURLOPT_POSTFIELDSIZE, strlen (POST_DATA));
  curl_easy_setopt (c, CURLOPT_POST, 1L);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  // NOTE: use of CONNECTTIMEOUT without also
  //   setting NOSIGNAL results in really weird
  //   crashes on my system!
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1);
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
    {
      fprintf (stderr,
               "curl_easy_perform failed: `%s'\n",
               curl_easy_strerror (errornum));
      curl_easy_cleanup (c);
      MHD_stop_daemon (d);
      return 32;
    }
  curl_easy_cleanup (c);
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
    return 64;
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
    return 128;
  return 0;
}

static int
testMultithreadedPoolPost ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
                        1081, NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_THREAD_POOL_SIZE, CPU_COUNT,
			MHD_OPTION_NOTIFY_COMPLETED, &completed_cb, NULL,			
			MHD_OPTION_END);
  if (d == NULL)
    return 16;
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1:1081/hello_world");
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_POSTFIELDS, POST_DATA);
  curl_easy_setopt (c, CURLOPT_POSTFIELDSIZE, strlen (POST_DATA));
  curl_easy_setopt (c, CURLOPT_POST, 1L);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  // NOTE: use of CONNECTTIMEOUT without also
  //   setting NOSIGNAL results in really weird
  //   crashes on my system!
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1);
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
    {
      fprintf (stderr,
               "curl_easy_perform failed: `%s'\n",
               curl_easy_strerror (errornum));
      curl_easy_cleanup (c);
      MHD_stop_daemon (d);
      return 32;
    }
  curl_easy_cleanup (c);
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
    return 64;
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
    return 128;
  return 0;
}

static int
testExternalPost ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLM *multi;
  CURLMcode mret;
  fd_set rs;
  fd_set ws;
  fd_set es;
  MHD_socket max;
  int running;
  struct CURLMsg *msg;
  time_t start;
  struct timeval tv;

  multi = NULL;
  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_DEBUG,
                        1082, NULL, NULL, &ahc_echo, NULL, 
			MHD_OPTION_NOTIFY_COMPLETED, &completed_cb, NULL,			
			MHD_OPTION_END);
  if (d == NULL)
    return 256;
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1:1082/hello_world");
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_POSTFIELDS, POST_DATA);
  curl_easy_setopt (c, CURLOPT_POSTFIELDSIZE, strlen (POST_DATA));
  curl_easy_setopt (c, CURLOPT_POST, 1L);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  // NOTE: use of CONNECTTIMEOUT without also
  //   setting NOSIGNAL results in really weird
  //   crashes on my system!
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1);


  multi = curl_multi_init ();
  if (multi == NULL)
    {
      curl_easy_cleanup (c);
      MHD_stop_daemon (d);
      return 512;
    }
  mret = curl_multi_add_handle (multi, c);
  if (mret != CURLM_OK)
    {
      curl_multi_cleanup (multi);
      curl_easy_cleanup (c);
      MHD_stop_daemon (d);
      return 1024;
    }
  start = time (NULL);
  while ((time (NULL) - start < 5) && (multi != NULL))
    {
      max = 0;
      FD_ZERO (&rs);
      FD_ZERO (&ws);
      FD_ZERO (&es);
      curl_multi_perform (multi, &running);
      mret = curl_multi_fdset (multi, &rs, &ws, &es, &max);
      if (mret != CURLM_OK)
        {
          curl_multi_remove_handle (multi, c);
          curl_multi_cleanup (multi);
          curl_easy_cleanup (c);
          MHD_stop_daemon (d);
          return 2048;
        }
      if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &max))
        {
          curl_multi_remove_handle (multi, c);
          curl_multi_cleanup (multi);
          curl_easy_cleanup (c);
          MHD_stop_daemon (d);
          return 4096;
        }
      tv.tv_sec = 0;
      tv.tv_usec = 1000;
      select (max + 1, &rs, &ws, &es, &tv);
      curl_multi_perform (multi, &running);
      if (running == 0)
        {
          msg = curl_multi_info_read (multi, &running);
          if (msg == NULL)
            break;
          if (msg->msg == CURLMSG_DONE)
            {
              if (msg->data.result != CURLE_OK)
                printf ("%s failed at %s:%d: `%s'\n",
                        "curl_multi_perform",
                        __FILE__,
                        __LINE__, curl_easy_strerror (msg->data.result));
              curl_multi_remove_handle (multi, c);
              curl_multi_cleanup (multi);
              curl_easy_cleanup (c);
              c = NULL;
              multi = NULL;
            }
        }
      MHD_run (d);
    }
  if (multi != NULL)
    {
      curl_multi_remove_handle (multi, c);
      curl_easy_cleanup (c);
      curl_multi_cleanup (multi);
    }
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
    return 8192;
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
    return 16384;
  return 0;
}


static int
ahc_cancel (void *cls,
	    struct MHD_Connection *connection,
	    const char *url,
	    const char *method,
	    const char *version,
	    const char *upload_data, size_t *upload_data_size,
	    void **unused)
{
  struct MHD_Response *response;
  int ret;

  if (0 != strcmp ("POST", method))
    {
      fprintf (stderr,
	       "Unexpected method `%s'\n", method);
      return MHD_NO; 
    }

  if (*unused == NULL)
    {
      *unused = "wibble";
      /* We don't want the body. Send a 500. */
      response = MHD_create_response_from_buffer (0, NULL, 
						  MHD_RESPMEM_PERSISTENT);
      ret = MHD_queue_response(connection, 500, response);
      if (ret != MHD_YES)
	fprintf(stderr, "Failed to queue response\n");
      MHD_destroy_response(response);
      return ret;
    }
  else
    {
      fprintf(stderr, 
	      "In ahc_cancel again. This should not happen.\n");
      return MHD_NO;
    }
}

struct CRBC
{
  const char *buffer;
  size_t size;
  size_t pos;
};


static size_t 
readBuffer(void *p, size_t size, size_t nmemb, void *opaque)
{
  struct CRBC *data = opaque;
  size_t required = size * nmemb;
  size_t left = data->size - data->pos;
  
  if (required > left)
    required = left;
  
  memcpy(p, data->buffer + data->pos, required);
  data->pos += required;
  
  return required/size;
}


static size_t 
slowReadBuffer(void *p, size_t size, size_t nmemb, void *opaque)
{
  sleep(1);
  return readBuffer(p, size, nmemb, opaque);
}


#define FLAG_EXPECT_CONTINUE 1
#define FLAG_CHUNKED 2
#define FLAG_FORM_DATA 4
#define FLAG_SLOW_READ 8
#define FLAG_COUNT 16


static int
testMultithreadedPostCancelPart(int flags)
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;
  struct curl_slist *headers = NULL;
  long response_code;
  CURLcode cc;
  int result = 0;
  struct CRBC crbc;

  /* Don't test features that aren't available with HTTP/1.0 in
   * HTTP/1.0 mode. */
  if (!oneone && (flags & (FLAG_EXPECT_CONTINUE | FLAG_CHUNKED)))
    return 0;

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
                        1081, NULL, NULL, &ahc_cancel, NULL, 
			MHD_OPTION_END);
  if (d == NULL)
    return 32768;

  crbc.buffer = "Test content";
  crbc.size = strlen(crbc.buffer);
  crbc.pos = 0;
  
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1:1081/hello_world");
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_READFUNCTION, (flags & FLAG_SLOW_READ) ? &slowReadBuffer : &readBuffer);
  curl_easy_setopt (c, CURLOPT_READDATA, &crbc);
  curl_easy_setopt (c, CURLOPT_POSTFIELDS, NULL);
  curl_easy_setopt (c, CURLOPT_POSTFIELDSIZE, crbc.size);
  curl_easy_setopt (c, CURLOPT_POST, 1L);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  // NOTE: use of CONNECTTIMEOUT without also
  //   setting NOSIGNAL results in really weird
  //   crashes on my system!
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1);

  if (flags & FLAG_CHUNKED)
      headers = curl_slist_append(headers, "Transfer-Encoding: chunked");
  if (!(flags & FLAG_FORM_DATA))
  headers = curl_slist_append(headers, "Content-Type: application/octet-stream");
  if (flags & FLAG_EXPECT_CONTINUE)
      headers = curl_slist_append(headers, "Expect: 100-Continue");
  curl_easy_setopt(c, CURLOPT_HTTPHEADER, headers);
  
  if (CURLE_HTTP_RETURNED_ERROR != (errornum = curl_easy_perform (c)))
    {
      fprintf (stderr,
               "flibbet curl_easy_perform didn't fail as expected: `%s' %d\n",
               curl_easy_strerror (errornum), errornum);
      curl_easy_cleanup (c);
      MHD_stop_daemon (d);
      curl_slist_free_all(headers);
      return 65536;
    }
  
  if (CURLE_OK != (cc = curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &response_code)))
    {
      fprintf(stderr, "curl_easy_getinfo failed: '%s'\n", curl_easy_strerror(errornum));
      result = 65536;
    }
  
  if (!result && (response_code != 500))
    {
      fprintf(stderr, "Unexpected response code: %ld\n", response_code);
      result = 131072;
    }
  
  if (!result && (cbc.pos != 0))
    result = 262144;

  curl_easy_cleanup (c);
  MHD_stop_daemon (d);
  curl_slist_free_all(headers);
  return result;
}


static int
testMultithreadedPostCancel()
{
  int result = 0;
  int flags;
  for(flags = 0; flags < FLAG_COUNT; ++flags)
    result |= testMultithreadedPostCancelPart(flags);  
  return result;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;

  oneone = (NULL != strrchr (argv[0], (int) '/')) ?
    (NULL != strstr (strrchr (argv[0], (int) '/'), "11")) : 0;
  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;
  errorCount += testMultithreadedPostCancel ();
  errorCount += testInternalPost ();
  errorCount += testMultithreadedPost ();
  errorCount += testMultithreadedPoolPost ();
  errorCount += testExternalPost ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  curl_global_cleanup ();
  return errorCount != 0;       /* 0 == pass */
}

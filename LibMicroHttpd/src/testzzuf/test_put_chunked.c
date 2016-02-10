/*
     This file is part of libmicrohttpd
     Copyright (C) 2007, 2008 Christian Grothoff

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
 * @file daemontest_put_chunked.c
 * @brief Testcase for libmicrohttpd PUT operations with chunked encoding
 *        for the upload data
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

#include "socat.c"

struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};

static size_t
putBuffer (void *stream, size_t size, size_t nmemb, void *ptr)
{
  unsigned int *pos = ptr;
  unsigned int wrt;

  wrt = size * nmemb;
  if (wrt > 8 - (*pos))
    wrt = 8 - (*pos);
  if (wrt > 4)
    wrt = 4;                    /* only send half at first => force multiple chunks! */
  memcpy (stream, &("Hello123"[*pos]), wrt);
  (*pos) += wrt;
  return wrt;
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

static int
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **unused)
{
  int *done = cls;
  struct MHD_Response *response;
  int ret;
  int have;

  if (0 != strcmp ("PUT", method))
    return MHD_NO;              /* unexpected method */
  if ((*done) < 8)
    {
      have = *upload_data_size;
      if (have + *done > 8)
        {
          return MHD_NO;
        }
      if (0 == memcmp (upload_data, &"Hello123"[*done], have))
        {
          *done += have;
          *upload_data_size = 0;
        }
      else
        {
          return MHD_NO;
        }
#if 0
      fprintf (stderr, "Not ready for response: %u/%u\n", *done, 8);
#endif
      return MHD_YES;
    }
  response = MHD_create_response_from_buffer (strlen (url),
					      (void *) url,
					      MHD_RESPMEM_MUST_COPY);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;
}


static int
testInternalPut ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  unsigned int pos = 0;
  int done_flag = 0;
  int i;

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG,
                        11080,
                        NULL, NULL, &ahc_echo, &done_flag, MHD_OPTION_END);
  if (d == NULL)
    return 1;
  zzuf_socat_start ();
  for (i = 0; i < LOOP_COUNT; i++)
    {
      fprintf (stderr, ".");
      c = curl_easy_init ();
      curl_easy_setopt (c, CURLOPT_URL, "http://localhost:11080/hello_world");
      curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
      curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
      curl_easy_setopt (c, CURLOPT_READFUNCTION, &putBuffer);
      curl_easy_setopt (c, CURLOPT_READDATA, &pos);
      curl_easy_setopt (c, CURLOPT_UPLOAD, 1L);
      /*
         // by not giving the file size, we force chunking!
         curl_easy_setopt (c, CURLOPT_INFILESIZE_LARGE, (curl_off_t) 8L);
       */
      curl_easy_setopt (c, CURLOPT_FAILONERROR, 1);
      curl_easy_setopt (c, CURLOPT_TIMEOUT_MS, CURL_TIMEOUT);
      curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
      curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT_MS, CURL_TIMEOUT);
      // NOTE: use of CONNECTTIMEOUT without also
      //   setting NOSIGNAL results in really weird
      //   crashes on my system!
      curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1);
      curl_easy_perform (c);
      curl_easy_cleanup (c);
    }
  fprintf (stderr, "\n");
  zzuf_socat_stop ();
  MHD_stop_daemon (d);
  return 0;
}

static int
testMultithreadedPut ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  unsigned int pos = 0;
  int done_flag = 0;
  CURLcode errornum;

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG,
                        11081,
                        NULL, NULL, &ahc_echo, &done_flag, MHD_OPTION_END);
  if (d == NULL)
    return 16;
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://localhost:11081/hello_world");
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_READFUNCTION, &putBuffer);
  curl_easy_setopt (c, CURLOPT_READDATA, &pos);
  curl_easy_setopt (c, CURLOPT_UPLOAD, 1L);
  /*
     // by not giving the file size, we force chunking!
     curl_easy_setopt (c, CURLOPT_INFILESIZE_LARGE, (curl_off_t) 8L);
   */
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 15L);
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
testExternalPut ()
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
  int max;
  int running;
  time_t start;
  struct timeval tv;
  unsigned int pos = 0;
  int done_flag = 0;
  int i;

  multi = NULL;
  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_DEBUG,
                        11082,
                        NULL, NULL, &ahc_echo, &done_flag, MHD_OPTION_END);
  if (d == NULL)
    return 256;

  multi = curl_multi_init ();
  if (multi == NULL)
    {
      MHD_stop_daemon (d);
      return 512;
    }
  zzuf_socat_start ();
  for (i = 0; i < LOOP_COUNT; i++)
    {
      fprintf (stderr, ".");
      c = curl_easy_init ();
      curl_easy_setopt (c, CURLOPT_URL, "http://localhost:11082/hello_world");
      curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
      curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
      curl_easy_setopt (c, CURLOPT_READFUNCTION, &putBuffer);
      curl_easy_setopt (c, CURLOPT_READDATA, &pos);
      curl_easy_setopt (c, CURLOPT_UPLOAD, 1L);
      /*
         // by not giving the file size, we force chunking!
         curl_easy_setopt (c, CURLOPT_INFILESIZE_LARGE, (curl_off_t) 8L);
       */
      curl_easy_setopt (c, CURLOPT_FAILONERROR, 1);
      curl_easy_setopt (c, CURLOPT_TIMEOUT_MS, CURL_TIMEOUT);
      curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
      curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT_MS, CURL_TIMEOUT);
      // NOTE: use of CONNECTTIMEOUT without also
      //   setting NOSIGNAL results in really weird
      //   crashes on my system!
      curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1);


      mret = curl_multi_add_handle (multi, c);
      if (mret != CURLM_OK)
        {
          curl_multi_cleanup (multi);
          curl_easy_cleanup (c);
          zzuf_socat_stop ();
          MHD_stop_daemon (d);
          return 1024;
        }
      start = time (NULL);
      while ((time (NULL) - start < 5) && (c != NULL))
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
              zzuf_socat_stop ();
              MHD_stop_daemon (d);
              return 2048;
            }
          if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &max))
            {
              curl_multi_remove_handle (multi, c);
              curl_multi_cleanup (multi);
              curl_easy_cleanup (c);
              zzuf_socat_stop ();
              MHD_stop_daemon (d);
              return 4096;
            }
          tv.tv_sec = 0;
          tv.tv_usec = 1000;
          select (max + 1, &rs, &ws, &es, &tv);
          curl_multi_perform (multi, &running);
          if (running == 0)
            {
              curl_multi_info_read (multi, &running);
              curl_multi_remove_handle (multi, c);
              curl_easy_cleanup (c);
              c = NULL;
            }
          MHD_run (d);
        }
      if (c != NULL)
        {
          curl_multi_remove_handle (multi, c);
          curl_easy_cleanup (c);
        }
    }
  fprintf (stderr, "\n");
  curl_multi_cleanup (multi);
  zzuf_socat_stop ();
  MHD_stop_daemon (d);
  return 0;
}



int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;
  errorCount += testInternalPut ();
  errorCount += testMultithreadedPut ();
  errorCount += testExternalPut ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  curl_global_cleanup ();
  return errorCount != 0;       /* 0 == pass */
}

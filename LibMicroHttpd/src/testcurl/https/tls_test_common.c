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
 * @file tls_test_common.c
 * @brief  Common tls test functions
 * @author Sagie Amir
 */
#include "tls_test_common.h"
#include "tls_test_keys.h"


int curl_check_version (const char *req_version, ...);

FILE *
setup_ca_cert ()
{
  FILE *cert_fd;

  if (NULL == (cert_fd = fopen (ca_cert_file_name, "wb+")))
    {
      fprintf (stderr, "Error: failed to open `%s': %s\n",
               ca_cert_file_name, strerror (errno));
      return NULL;
    }
  if (fwrite (ca_cert_pem, sizeof (char), strlen (ca_cert_pem) + 1, cert_fd)
      != strlen (ca_cert_pem) + 1)
    {
      fprintf (stderr, "Error: failed to write `%s. %s'\n",
               ca_cert_file_name, strerror (errno));
      fclose (cert_fd);
      return NULL;
    }
  if (fflush (cert_fd))
    {
      fprintf (stderr, "Error: failed to flush ca cert file stream. %s\n",
               strerror (errno));
      fclose (cert_fd);
      return NULL;
    }
  return cert_fd;
}


/*
 * test HTTPS transfer
 */
int
test_daemon_get (void *cls,
		 const char *cipher_suite, int proto_version,
		 int port,
		 int ver_peer)
{
  CURL *c;
  struct CBC cbc;
  CURLcode errornum;
  char url[255];
  size_t len;

  len = strlen (test_data);
  if (NULL == (cbc.buf = malloc (sizeof (char) * len)))
    {
      fprintf (stderr, MHD_E_MEM);
      return -1;
    }
  cbc.size = len;
  cbc.pos = 0;

  /* construct url - this might use doc_path */
  gen_test_file_url (url, port);

  c = curl_easy_init ();
#if DEBUG_HTTPS_TEST
  curl_easy_setopt (c, CURLOPT_VERBOSE, 1);
#endif
  curl_easy_setopt (c, CURLOPT_URL, url);
  curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 10L);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 10L);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_FILE, &cbc);

  /* TLS options */
  curl_easy_setopt (c, CURLOPT_SSLVERSION, proto_version);
  curl_easy_setopt (c, CURLOPT_SSL_CIPHER_LIST, cipher_suite);

  /* perform peer authentication */
  /* TODO merge into send_curl_req */
  curl_easy_setopt (c, CURLOPT_SSL_VERIFYPEER, ver_peer);
  curl_easy_setopt (c, CURLOPT_CAINFO, ca_cert_file_name);
  curl_easy_setopt (c, CURLOPT_SSL_VERIFYHOST, 0);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1);
  
  /* NOTE: use of CONNECTTIMEOUT without also
     setting NOSIGNAL results in really weird
     crashes on my system! */
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1);
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
    {
      fprintf (stderr, "curl_easy_perform failed: `%s'\n",
               curl_easy_strerror (errornum));
      curl_easy_cleanup (c);
      free (cbc.buf);
      return errornum;
    }

  curl_easy_cleanup (c);

  if (memcmp (cbc.buf, test_data, len) != 0)
    {
      fprintf (stderr, "Error: local file & received file differ.\n");
      free (cbc.buf);
      return -1;
    }

  free (cbc.buf);
  return 0;
}


void
print_test_result (int test_outcome, char *test_name)
{
#if 0
  if (test_outcome != 0)
    fprintf (stderr, "running test: %s [fail]\n", test_name);
  else
    fprintf (stdout, "running test: %s [pass]\n", test_name);
#endif
}

size_t
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
 *  HTTP access handler call back
 */
int
http_ahc (void *cls, struct MHD_Connection *connection,
          const char *url, const char *method, const char *upload_data,
          const char *version, size_t *upload_data_size, void **ptr)
{
  static int aptr;
  struct MHD_Response *response;
  int ret;

  if (0 != strcmp (method, MHD_HTTP_METHOD_GET))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *ptr)
    {
      /* do never respond on first call */
      *ptr = &aptr;
      return MHD_YES;
    }
  *ptr = NULL;                  /* reset when done */
  response = MHD_create_response_from_buffer (strlen (test_data),
					      (void *) test_data,
					      MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;
}

/* HTTP access handler call back */
int
http_dummy_ahc (void *cls, struct MHD_Connection *connection,
                const char *url, const char *method, const char *upload_data,
                const char *version, size_t *upload_data_size,
                void **ptr)
{
  return 0;
}

/**
 * send a test http request to the daemon
 * @param url
 * @param cbc - may be null
 * @param cipher_suite
 * @param proto_version
 * @return
 */
/* TODO have test wrap consider a NULL cbc */
int
send_curl_req (char *url, struct CBC * cbc, const char *cipher_suite,
               int proto_version)
{
  CURL *c;
  CURLcode errornum;
  c = curl_easy_init ();
#if DEBUG_HTTPS_TEST
  curl_easy_setopt (c, CURLOPT_VERBOSE, CURL_VERBOS_LEVEL);
#endif
  curl_easy_setopt (c, CURLOPT_URL, url);
  curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 60L);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 60L);

  if (cbc != NULL)
    {
      curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
      curl_easy_setopt (c, CURLOPT_FILE, cbc);
    }

  /* TLS options */
  curl_easy_setopt (c, CURLOPT_SSLVERSION, proto_version);
  curl_easy_setopt (c, CURLOPT_SSL_CIPHER_LIST, cipher_suite);

  /* currently skip any peer authentication */
  curl_easy_setopt (c, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt (c, CURLOPT_SSL_VERIFYHOST, 0);

  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1);

  /* NOTE: use of CONNECTTIMEOUT without also
     setting NOSIGNAL results in really weird
     crashes on my system! */
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1);
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
    {
      fprintf (stderr, "curl_easy_perform failed: `%s'\n",
               curl_easy_strerror (errornum));
      curl_easy_cleanup (c);
      return errornum;
    }
  curl_easy_cleanup (c);

  return CURLE_OK;
}


/**
 * compile test file url pointing to the current running directory path
 *
 * @param url - char buffer into which the url is compiled
 * @param port port to use for the test
 * @return -1 on error
 */
int
gen_test_file_url (char *url, int port)
{
  int ret = 0;
  char *doc_path;
  size_t doc_path_len;
  /* setup test file path, url */
  doc_path_len = PATH_MAX > 4096 ? 4096 : PATH_MAX;
  if (NULL == (doc_path = malloc (doc_path_len)))
    {
      fprintf (stderr, MHD_E_MEM);
      return -1;
    }
  if (getcwd (doc_path, doc_path_len) == NULL)
    {
      fprintf (stderr, "Error: failed to get working directory. %s\n",
               strerror (errno));
      ret = -1;
    }
#ifdef WINDOWS
  {
    int i;
    for (i = 0; i < doc_path_len; i++)
    {
      if (doc_path[i] == 0)
        break;
      if (doc_path[i] == '\\')
      {
        doc_path[i] = '/';
      }
      if (doc_path[i] != ':')
        continue;
      if (i == 0)
        break;
      doc_path[i] = doc_path[i - 1];
      doc_path[i - 1] = '/';
    }
  }
#endif
  /* construct url - this might use doc_path */
  if (sprintf (url, "%s:%d%s/%s", "https://127.0.0.1", port,
               doc_path, "urlpath") < 0)
    ret = -1;

  free (doc_path);
  return ret;
}

/**
 * test HTTPS file transfer
 */
int
test_https_transfer (void *cls, const char *cipher_suite, int proto_version)
{
  int len;
  int ret = 0;
  struct CBC cbc;
  char url[255];

  len = strlen (test_data);
  if (NULL == (cbc.buf = malloc (sizeof (char) * len)))
    {
      fprintf (stderr, MHD_E_MEM);
      return -1;
    }
  cbc.size = len;
  cbc.pos = 0;

  if (gen_test_file_url (url, DEAMON_TEST_PORT))
    {
      ret = -1;
      goto cleanup;
    }

  if (CURLE_OK != send_curl_req (url, &cbc, cipher_suite, proto_version))
    {
      ret = -1;
      goto cleanup;
    }

  /* compare test file & daemon responce */
  if ( (len != strlen (test_data)) ||
       (memcmp (cbc.buf, 
		test_data, 
		len) != 0) )
    {
      fprintf (stderr, "Error: local file & received file differ.\n");
      ret = -1;
    }
cleanup:
  free (cbc.buf);
  return ret;
}

/**
 * setup test case
 *
 * @param d
 * @param daemon_flags
 * @param arg_list
 * @return
 */
int
setup_testcase (struct MHD_Daemon **d, int daemon_flags, va_list arg_list)
{
  *d = MHD_start_daemon_va (daemon_flags, DEAMON_TEST_PORT,
                            NULL, NULL, &http_ahc, NULL, arg_list);

  if (*d == NULL)
    {
      fprintf (stderr, MHD_E_SERVER_INIT);
      return -1;
    }

  return 0;
}

void
teardown_testcase (struct MHD_Daemon *d)
{
  MHD_stop_daemon (d);
}

int
setup_session (gnutls_session_t * session,
               gnutls_datum_t * key,
               gnutls_datum_t * cert, 
	       gnutls_certificate_credentials_t * xcred)
{
  int ret;
  const char *err_pos;

  gnutls_certificate_allocate_credentials (xcred);
  key->size = strlen (srv_key_pem) + 1;
  key->data = malloc (key->size);
  if (NULL == key->data) 
     {
       gnutls_certificate_free_credentials (*xcred);
	return -1;
     }
  memcpy (key->data, srv_key_pem, key->size);
  cert->size = strlen (srv_self_signed_cert_pem) + 1;
  cert->data = malloc (cert->size);
  if (NULL == cert->data)
    {
        gnutls_certificate_free_credentials (*xcred);
	free (key->data); 
	return -1;
    }
  memcpy (cert->data, srv_self_signed_cert_pem, cert->size);
  gnutls_certificate_set_x509_key_mem (*xcred, cert, key,
				       GNUTLS_X509_FMT_PEM);
  gnutls_init (session, GNUTLS_CLIENT);
  ret = gnutls_priority_set_direct (*session,
				    "NORMAL", &err_pos);
  if (ret < 0)
    {
       gnutls_deinit (*session);
       gnutls_certificate_free_credentials (*xcred);
       free (key->data);
       return -1;
    }
  gnutls_credentials_set (*session, 
			  GNUTLS_CRD_CERTIFICATE, 
			  *xcred);
  return 0;
}

int
teardown_session (gnutls_session_t session,
                  gnutls_datum_t * key,
                  gnutls_datum_t * cert,
                  gnutls_certificate_credentials_t xcred)
{
  free (key->data);
  key->data = NULL;
  key->size = 0;
  free (cert->data);
  cert->data = NULL;
  cert->size = 0;
  gnutls_deinit (session);
  gnutls_certificate_free_credentials (xcred);
  return 0;
}

/* TODO test_wrap: change sig to (setup_func, test, va_list test_arg) */
int
test_wrap (const char *test_name, int
           (*test_function) (void * cls, const char *cipher_suite,
                             int proto_version), void * cls,
           int daemon_flags, const char *cipher_suite, int proto_version, ...)
{
  int ret;
  va_list arg_list;
  struct MHD_Daemon *d;

  va_start (arg_list, proto_version);
  if (setup_testcase (&d, daemon_flags, arg_list) != 0)
    {
      va_end (arg_list);
      fprintf (stderr, "Failed to setup testcase %s\n", test_name);
      return -1;
    }
#if 0
  fprintf (stdout, "running test: %s ", test_name);
#endif
  ret = test_function (NULL, cipher_suite, proto_version);
#if 0
  if (ret == 0)
    {
      fprintf (stdout, "[pass]\n");
    }
  else
    {
      fprintf (stdout, "[fail]\n");
    }
#endif
  teardown_testcase (d);
  va_end (arg_list);
  return ret;
}

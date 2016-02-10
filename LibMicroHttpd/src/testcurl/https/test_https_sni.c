/*
  This file is part of libmicrohttpd
  Copyright (C) 2013 Christian Grothoff

  libmicrohttpd is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3, or (at your
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
 * @file test_https_sni.c
 * @brief  Testcase for libmicrohttpd HTTPS with SNI operations
 * @author Christian Grothoff
 */
#include "platform.h"
#include "microhttpd.h"
#include <limits.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <gcrypt.h>
#include "tls_test_common.h"
#include <gnutls/gnutls.h>

/* This test only works with GnuTLS >= 3.0 */
#if GNUTLS_VERSION_MAJOR >= 3

#include <gnutls/abstract.h>

/**
 * A hostname, server key and certificate.
 */
struct Hosts
{
  struct Hosts *next;
  const char *hostname;
  gnutls_pcert_st pcrt;
  gnutls_privkey_t key;
};


/**
 * Linked list of supported TLDs and respective certificates.
 */
static struct Hosts *hosts;

/* Load the certificate and the private key.
 * (This code is largely taken from GnuTLS).
 */
static void
load_keys(const char *hostname,
          const char *CERT_FILE,
          const char *KEY_FILE)
{
  int ret;
  gnutls_datum_t data;
  struct Hosts *host;

  host = malloc (sizeof (struct Hosts));
  if (NULL == host)
    abort ();
  host->hostname = hostname;
  host->next = hosts;
  hosts = host;

  ret = gnutls_load_file (CERT_FILE, &data);
  if (ret < 0)
    {
      fprintf (stderr,
               "*** Error loading certificate file %s.\n",
               CERT_FILE);
      exit (1);
    }
  ret =
    gnutls_pcert_import_x509_raw (&host->pcrt, &data, GNUTLS_X509_FMT_PEM,
                                  0);
  if (ret < 0)
    {
      fprintf (stderr,
               "*** Error loading certificate file: %s\n",
               gnutls_strerror (ret));
      exit (1);
    }
  gnutls_free (data.data);

  ret = gnutls_load_file (KEY_FILE, &data);
  if (ret < 0)
    {
      fprintf (stderr,
               "*** Error loading key file %s.\n",
               KEY_FILE);
      exit (1);
    }

  gnutls_privkey_init (&host->key);
  ret =
    gnutls_privkey_import_x509_raw (host->key,
                                    &data, GNUTLS_X509_FMT_PEM,
                                    NULL, 0);
  if (ret < 0)
    {
      fprintf (stderr,
               "*** Error loading key file: %s\n",
               gnutls_strerror (ret));
      exit (1);
    }
  gnutls_free (data.data);
}



/**
 * @param session the session we are giving a cert for
 * @param req_ca_dn NULL on server side
 * @param nreqs length of req_ca_dn, and thus 0 on server side
 * @param pk_algos NULL on server side
 * @param pk_algos_length 0 on server side
 * @param pcert list of certificates (to be set)
 * @param pcert_length length of pcert (to be set)
 * @param pkey the private key (to be set)
 */
static int
sni_callback (gnutls_session_t session,
              const gnutls_datum_t* req_ca_dn,
              int nreqs,
              const gnutls_pk_algorithm_t* pk_algos,
              int pk_algos_length,
              gnutls_pcert_st** pcert,
              unsigned int *pcert_length,
              gnutls_privkey_t * pkey)
{
  char name[256];
  size_t name_len;
  struct Hosts *host;
  unsigned int type;

  name_len = sizeof (name);
  if (GNUTLS_E_SUCCESS !=
      gnutls_server_name_get (session,
                              name,
                              &name_len,
                              &type,
                              0 /* index */))
    return -1;
  for (host = hosts; NULL != host; host = host->next)
    if (0 == strncmp (name, host->hostname, name_len))
      break;
  if (NULL == host)
    {
      fprintf (stderr,
               "Need certificate for %.*s\n",
               (int) name_len,
               name);
      return -1;
    }
#if 0
  fprintf (stderr,
           "Returning certificate for %.*s\n",
           (int) name_len,
           name);
#endif
  *pkey = host->key;
  *pcert_length = 1;
  *pcert = &host->pcrt;
  return 0;
}


/* perform a HTTP GET request via SSL/TLS */
static int
do_get (const char *url)
{
  CURL *c;
  struct CBC cbc;
  CURLcode errornum;
  size_t len;
  struct curl_slist *dns_info;

  len = strlen (test_data);
  if (NULL == (cbc.buf = malloc (sizeof (char) * len)))
    {
      fprintf (stderr, MHD_E_MEM);
      return -1;
    }
  cbc.size = len;
  cbc.pos = 0;

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

  /* perform peer authentication */
  /* TODO merge into send_curl_req */
  curl_easy_setopt (c, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt (c, CURLOPT_SSL_VERIFYHOST, 2);
  dns_info = curl_slist_append (NULL, "host1:4233:127.0.0.1");
  dns_info = curl_slist_append (dns_info, "host2:4233:127.0.0.1");
  curl_easy_setopt (c, CURLOPT_RESOLVE, dns_info);
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
      curl_slist_free_all (dns_info);
      return errornum;
    }

  curl_easy_cleanup (c);
  curl_slist_free_all (dns_info);
  if (memcmp (cbc.buf, test_data, len) != 0)
    {
      fprintf (stderr, "Error: local file & received file differ.\n");
      free (cbc.buf);
      return -1;
    }

  free (cbc.buf);
  return 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int error_count = 0;
  struct MHD_Daemon *d;

  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif
  if (0 != curl_global_init (CURL_GLOBAL_ALL))
    {
      fprintf (stderr, "Error: %s\n", strerror (errno));
      return -1;
    }
  load_keys ("host1", ABS_SRCDIR "/host1.crt", ABS_SRCDIR "/host1.key");
  load_keys ("host2", ABS_SRCDIR "/host2.crt", ABS_SRCDIR "/host2.key");
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL | MHD_USE_DEBUG,
                        4233,
                        NULL, NULL,
                        &http_ahc, NULL,
                        MHD_OPTION_HTTPS_CERT_CALLBACK, &sni_callback,
                        MHD_OPTION_END);
  if (d == NULL)
    {
      fprintf (stderr, MHD_E_SERVER_INIT);
      return -1;
    }
  error_count += do_get ("https://host1:4233/");
  error_count += do_get ("https://host2:4233/");

  MHD_stop_daemon (d);
  curl_global_cleanup ();
  return error_count != 0;
}


#else

int main ()
{
  fprintf (stderr,
           "SNI not supported by GnuTLS < 3.0\n");
  return 0;
}
#endif

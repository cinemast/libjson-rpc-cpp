/*
  This file is part of libmicrohttpd
  Copyright (C) 2007 Christian Grothoff

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
 * @file test_https_get.c
 * @brief  Testcase for libmicrohttpd HTTPS GET operations
 * @author Sagie Amir
 */

#include "platform.h"
#include "microhttpd.h"
#include <limits.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <gcrypt.h>
#include "tls_test_common.h"

extern const char srv_key_pem[];
extern const char srv_self_signed_cert_pem[];
extern const char srv_signed_cert_pem[];
extern const char srv_signed_key_pem[];


static int
test_cipher_option (FILE * test_fd,
		    const char *cipher_suite,
		    int proto_version)
{

  int ret;
  struct MHD_Daemon *d;
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL |
                        MHD_USE_DEBUG, 4233,
                        NULL, NULL, &http_ahc, NULL,
                        MHD_OPTION_HTTPS_MEM_KEY, srv_key_pem,
                        MHD_OPTION_HTTPS_MEM_CERT, srv_self_signed_cert_pem,
                        MHD_OPTION_END);

  if (d == NULL)
    {
      fprintf (stderr, MHD_E_SERVER_INIT);
      return -1;
    }

  ret = test_https_transfer (test_fd, cipher_suite, proto_version);

  MHD_stop_daemon (d);
  return ret;
}


/* perform a HTTP GET request via SSL/TLS */
static int
test_secure_get (FILE * test_fd,
		 const char *cipher_suite,
		 int proto_version)
{
  int ret;
  struct MHD_Daemon *d;

  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION | MHD_USE_SSL |
                        MHD_USE_DEBUG, 4233,
                        NULL, NULL, &http_ahc, NULL,
                        MHD_OPTION_HTTPS_MEM_KEY, srv_signed_key_pem,
                        MHD_OPTION_HTTPS_MEM_CERT, srv_signed_cert_pem,
                        MHD_OPTION_END);

  if (d == NULL)
    {
      fprintf (stderr, MHD_E_SERVER_INIT);
      return -1;
    }

  ret = test_https_transfer (test_fd, cipher_suite, proto_version);

  MHD_stop_daemon (d);
  return ret;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  const char *aes256_sha_tlsv1   = "AES256-SHA";
  const char *des_cbc3_sha_tlsv1 = "DES-CBC3-SHA";

  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif
  if (0 != curl_global_init (CURL_GLOBAL_ALL))
    {
      fprintf (stderr, "Error: %s\n", strerror (errno));
      return -1;
    }

  if (curl_uses_nss_ssl() == 0)
    {
      aes256_sha_tlsv1 = "rsa_aes_256_sha";
      des_cbc3_sha_tlsv1 = "rsa_aes_128_sha";
    }

  errorCount +=
    test_secure_get (NULL, aes256_sha_tlsv1, CURL_SSLVERSION_TLSv1);
  errorCount +=
    test_cipher_option (NULL, des_cbc3_sha_tlsv1, CURL_SSLVERSION_TLSv1);
  print_test_result (errorCount, argv[0]);

  curl_global_cleanup ();

  return errorCount != 0;
}

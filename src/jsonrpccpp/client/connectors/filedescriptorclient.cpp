/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    filedescriptorclient.cpp
 * @date    26.10.2016
 * @author  Jean-Daniel Michaud <jean.daniel.michaud@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "filedescriptorclient.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cstdlib>
#include <cstdio>

#define BUFFER_SIZE 64
#ifndef DELIMITER_CHAR
#define DELIMITER_CHAR char(0x0A)
#endif //DELIMITER_CHAR
#define MEX_ERR_MSG 255

using namespace jsonrpc;
using namespace std;

FileDescriptorClient::FileDescriptorClient(int inputfd, int outputfd) :
  inputfd(inputfd), outputfd(outputfd)
{
}

FileDescriptorClient::~FileDescriptorClient()
{
}

void FileDescriptorClient::SendRPCMessage(const std::string& message,
  std::string& result) throw (JsonRpcException)
{
  bool fullyWritten = false;
  string toSend = message;
  do
  {
    ssize_t byteWritten = write(outputfd, toSend.c_str(), toSend.size());
    if (byteWritten < 1) {
      char errorstr[MEX_ERR_MSG];
      if (strerror_r(errno, errorstr, MEX_ERR_MSG) == 0)
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR, std::string(errorstr));
      else
        throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR,
          "Unknown error occured while writing to the output file descriptor");
    }

    if (byteWritten < (ssize_t) toSend.size())
    {
      unsigned long len = toSend.size() - byteWritten;
      toSend = toSend.substr(byteWritten + sizeof(char), len);
    }
    else
      fullyWritten = true;
  } while(!fullyWritten);

  if (!IsReadable(inputfd))
    throw JsonRpcException(Errors::ERROR_CLIENT_CONNECTOR,
      "The input file descriptor is not readable");

  ssize_t nbytes = 0;
  char buffer[BUFFER_SIZE];
  do
  {
    nbytes = read(inputfd, buffer, BUFFER_SIZE);
    result.append(buffer, nbytes);
  } while(result.find(DELIMITER_CHAR) == string::npos);
}

bool FileDescriptorClient::IsReadable(int fd)
{
  int o_accmode = 0;
  int ret = fcntl(fd, F_GETFL, &o_accmode);
  if (ret == -1)
    return false;
  return ((o_accmode & O_ACCMODE) == O_RDONLY ||
    (o_accmode & O_ACCMODE) == O_RDWR);
}


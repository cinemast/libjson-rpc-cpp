/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    filedescriptorserver.h
 * @date    25.10.2016
 * @author  Jean-Daniel Michaud <jean.daniel.michaud@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "filedescriptorserver.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

using namespace jsonrpc;
using namespace std;

#define BUFFER_SIZE 1024
#ifndef DELIMITER_CHAR
#define DELIMITER_CHAR char(0x0A)
#endif

FileDescriptorServer::FileDescriptorServer(int inputfd, int outputfd) :
  running(false), inputfd(inputfd), outputfd(outputfd)
{
}

bool FileDescriptorServer::StartListening()
{
  if(this->running)
    return false;

  if (!IsReadable(inputfd) || !IsWritable(outputfd))
    return false;

  this->running = true;
  int ret = pthread_create(&(this->listenning_thread), NULL, FileDescriptorServer::LaunchLoop, this);
  if(ret != 0)
  {
    pthread_detach(this->listenning_thread);
  }
  this->running = static_cast<bool>(ret == 0);

  return this->running;
}

bool FileDescriptorServer::StopListening()
{
  if (!this->running)
    return false;

  this->running = false;
  pthread_join(this->listenning_thread, NULL);
  return !(this->running);
}

bool FileDescriptorServer::SendResponse(const string& response, void* addInfo)
{
  if (!IsWritable(outputfd))
    return false;

  string toSend = response;
  // If the DELIMITER_CHAR was not append, do it now
  if (toSend.find(DELIMITER_CHAR) == string::npos)
    toSend.append(1, DELIMITER_CHAR);

  if (DELIMITER_CHAR != '\n')
    toSend = toSend.substr(0, toSend.find_last_of('\n')) + DELIMITER_CHAR;

  ssize_t result = 0;
  ssize_t nbytes = toSend.size();
  do
  {
    result = write(outputfd, &(toSend.c_str()[toSend.size() - nbytes]), toSend.size() - result);
    nbytes -= result;
  } while (result && nbytes); // While we are still writing and there is still to write
  return result;
}

void* FileDescriptorServer::LaunchLoop(void *p_data)
{
  pthread_detach(pthread_self());
  FileDescriptorServer *instance = reinterpret_cast<FileDescriptorServer*>(p_data);;
  instance->ListenLoop();
  return NULL;
}

void FileDescriptorServer::ListenLoop()
{
  while (this->running)
  {
    char buffer[BUFFER_SIZE];
    ssize_t nbytes;
    string request;
    do
    { //The client sends its json formatted request and a delimiter request.
      nbytes = read(inputfd, buffer, BUFFER_SIZE);
      if (nbytes == 0) {
        // File closed
        this->running = false;
        break;
      }
      request.append(buffer, nbytes);
    } while (request.find(DELIMITER_CHAR) == string::npos);
    if (nbytes) {
      this->OnRequest(request, NULL);
    }
  }
}

bool FileDescriptorServer::IsReadable(int fd)
{
  int o_accmode = 0;
  int ret = fcntl(fd, F_GETFL, &o_accmode);
  if (ret == -1)
  if (ret == -1 )
    return ret;
  return ((o_accmode & O_ACCMODE) == O_RDONLY ||
    (o_accmode & O_ACCMODE) == O_RDWR);
}

bool FileDescriptorServer::IsWritable(int fd)
{
  int ret = fcntl(fd, F_GETFL);
  if (ret == -1)
    return ret;
  return ((ret & O_WRONLY) || (ret & O_RDWR));
}


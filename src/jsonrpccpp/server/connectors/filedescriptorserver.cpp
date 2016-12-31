/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    filedescriptorserver.h
 * @date    25.10.2016
 * @author  Jean-Daniel Michaud <jean.daniel.michaud@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "filedescriptorserver.h"
#include <sys/select.h>
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
#define READ_TIMEOUT 0.2 // Set timeout in seconds

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

void FileDescriptorServer::Wait() const {
  pthread_join(this->listenning_thread, NULL);
}

bool FileDescriptorServer::SendResponse(const string& response, void* addInfo)
{
  (void)addInfo; // Suppress warning
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
  return result != 0;
}

void* FileDescriptorServer::LaunchLoop(void *p_data)
{
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
    {
      // Wait for something to be read.
      // Interrupt after a timeout to check it we should still wait or just stop.
      int wait_ret = 0;
      if ((wait_ret = this->WaitForRead()) > 0 && FD_ISSET(inputfd, &read_fds))
      {
        this->running = (nbytes = read(inputfd, buffer, BUFFER_SIZE)) != 0;
        request.append(buffer, nbytes);
      } else {
        if (wait_ret < 0) {
          // File closed
          this->running = false;
          break;
        }
      }
    } while (this->running && request.find(DELIMITER_CHAR) == string::npos);
    if (this->running) { // False if either the input fd was closed or someone interrupted us with ::StopListening.
      this->OnRequest(request, NULL);
    }
  }
}

int FileDescriptorServer::WaitForRead() {
  // Has to be reset after every call, as POSIX allow the value to be modifiable by the system.
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&except_fds);
  FD_SET(inputfd, &read_fds);
  timeout.tv_sec = 0;
  timeout.tv_usec = (suseconds_t) (READ_TIMEOUT * 1000000);
  // Wait for something to read
  return select(inputfd + 1, &read_fds, &write_fds, &except_fds, &timeout);
}

bool FileDescriptorServer::IsReadable(int fd)
{
  int o_accmode = 0;
  int ret = fcntl(fd, F_GETFL, &o_accmode);
  if (ret == -1)
    return false;
  return ((o_accmode & O_ACCMODE) == O_RDONLY ||
    (o_accmode & O_ACCMODE) == O_RDWR);
}

bool FileDescriptorServer::IsWritable(int fd)
{
  int ret = fcntl(fd, F_GETFL);
  if (ret == -1)
    return false;
  return ((ret & O_WRONLY) || (ret & O_RDWR));
}


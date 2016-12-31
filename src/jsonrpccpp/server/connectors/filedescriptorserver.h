/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    filedescriptorserver.h
 * @date    25.10.2016
 * @author  Jean-Daniel Michaud <jean.daniel.michaud@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_FILEDESCRIPTORSERVERCONNECTOR_H_
#define JSONRPC_CPP_FILEDESCRIPTORSERVERCONNECTOR_H_

#include <string>
#include <pthread.h>
#include <sys/select.h>

#include "../abstractserverconnector.h"

namespace jsonrpc
{
  /**
   * This class is the file descriptor implementation of an AbstractServerConnector.
   * It uses the POSIX file API and POSIX thread API to performs its job.
   * Each client request is handled in a new thread.
   */
  class FileDescriptorServer: public AbstractServerConnector
  {
    public:
      /**
       * AbstractServerConnector
       * @param inputfd The file descriptor already open for us to read
       * @param outputfd The file descriptor already open for us to write
       */
      FileDescriptorServer(int inputfd, int outputfd);
      /**
       * This method launches the listening loop that will handle client connections.
       * @return true if the file is readable, false otherwise.
       */
      bool StartListening();
      /**
       * This method stops the listening loop that will handle client connections.
       * @return True if successful, false otherwise of if not listening.
       */
      bool StopListening();
      /**
       * This method sends the result of the RPC Call over the output file
       * @param response The response to send to the client
       * @param addInfo Additionnal parameters
       * @return A boolean that indicates the success or the failure of the operation.
       */
      bool SendResponse(const std::string& response, void* addInfo = NULL);

      /**
       * This method blocks the caller as long as the server is listening to its input.
       */
      void Wait() const;

  private:
      bool running;
      int inputfd;
      int outputfd;

      // For select operation
      fd_set read_fds;
      fd_set write_fds;
      fd_set except_fds;
      struct timeval timeout;

      pthread_t listenning_thread;

      static void* LaunchLoop(void *p_data);
      void ListenLoop();
      bool IsReadable(int fd);
      bool IsWritable(int fd);

      int WaitForRead();
  };
}

#endif /* JSONRPC_CPP_FILEDESCRIPTORSERVERCONNECTOR_H_ */


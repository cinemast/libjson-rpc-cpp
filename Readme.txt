***     jsonrpccpp modifications      ***

original source: https://github.com/cinemast/libjson-rpc-cpp

implemented features:
- TcpClient and TcpServer are new connectors based on Tcp-Sockets
- they provide bidirectional communication with a callback for context-switch
- the library can therefore be used asynchronously by using rpcrequests with no return (e.g. notifications)
- those two connectors lack the normal use of rpcrequests with return, the available httpconnectors can be used
  as such, but they are not bidirectional
- client- and serverstubs for tcp connectors can be generated with the libraries stubgenerator, but afterwards require some minor modifications
  as the tcp-server needs a connection-id to send rpc-requests, this feature is not provided by the generated headers
- new classes: TcpServer, TcpServerCallback, TcpClient, TcpClientCallback, TcpConnection,
  IServerConnector (similar to IClientConnector), Server (similar to Client)

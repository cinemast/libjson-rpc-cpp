# Examples

## File descriptors

This example demonstrate the use of simple file descriptor to connect
processes with jsonrpc.

First create a FIFO:

```
mkfifo FOO
```

Then launch the client and the server by connecting them to each other
this way:
```
filedescriptorclientsample < PIPE | filedescriptorserversample > PIPE
```

The way the standard output of the client is connected to the standard
input of the server, and /vice versa/.

You can even launch the server in standalone and enter json-rpc
compliant json string in the standard input to test it.

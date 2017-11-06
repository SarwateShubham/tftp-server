# File Transfer Server


## Usage Instructions:

First copy the files server_tftp.c & makefile into a common folder.
Next, run the command make on the terminal.
Start the server by the command : 
```bash
$ make
$ ./server localhost <port no.>
```
Usage will be displayed to help run the code.
Start the client code by the commands: 

```bash
$ tftp 
tftp> connect <ip address> <port no.> 
tftp> get <filename> 
```

Usage will be displayed to help run the code.

## Architecture
The timeout is 10 seconds with 10 retransmission attempts allowed after which the client will be disconnected from the server.The code is compatible with IPv4 and IPv6 addresses. The code is built upon Beej’s guide and UNIX Network Programming guide as primary references. 


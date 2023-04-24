all: UDPEchoClient-Timeout UDPEchoServer
UDPEchoClient-Timeout: UDPEchoClient-Timeout.c DieWithError.c
	gcc UDPEchoClient-Timeout.c DieWithError.c -o UDPEchoClient-Timeout
UDPEchoServer: UDPEchoServer.c DieWithError.c
	gcc UDPEchoServer.c DieWithError.c -o UDPEchoServer

all: UDPEchoClient UDPEchoServer-SIGIO
UDPEchoClient: UDPEchoClient.c DieWithError.c
	gcc UDPEchoClient.c DieWithError.c -o UDPEchoClient
UDPEchoServer-SIGIO: UDPEchoServer-SIGIO.c DieWithError.c
	gcc UDPEchoServer-SIGIO.c DieWithError.c -o UDPEchoServer-SIGIO

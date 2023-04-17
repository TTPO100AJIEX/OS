all: TCPEchoClientLoop TCPEchoServer-ForkN
TCPEchoClientLoop: TCPEchoClientLoop.c DieWithError.c
	gcc TCPEchoClientLoop.c DieWithError.c -o TCPEchoClientLoop
TCPEchoServer-ForkN: TCPEchoServer-ForkN.c DieWithError.c
	gcc TCPEchoServer.h TCPEchoServer-ForkN.c DieWithError.c \
	HandleTCPClient.c AcceptTCPConnection.c CreateTCPServerSocket.c \
	-o TCPEchoServer-ForkN

all: TCPEchoClientLoop TCPEchoServer-Thread
TCPEchoClientLoop: TCPEchoClientLoop.c DieWithError.c
	gcc TCPEchoClientLoop.c DieWithError.c -o TCPEchoClientLoop
TCPEchoServer-Thread: TCPEchoServer.h TCPEchoServer-Thread.c DieWithError.c
	gcc TCPEchoServer-Thread.c DieWithError.c HandleTCPClient.c \
	CreateTCPServerSocket.c AcceptTCPConnection.c \
	-o TCPEchoServer-Thread -pthread

all: TCPEchoClientLoop TCPEchoServer
TCPEchoClientLoop: TCPEchoClientLoop.c DieWithError.c
	gcc TCPEchoClientLoop.c DieWithError.c -o TCPEchoClientLoop
TCPEchoServer: TCPEchoServer.c DieWithError.c
	gcc TCPEchoServer.c DieWithError.c HandleTCPClient.c -o TCPEchoServer

gcc_options = -O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=gnu17 -Werror -Wall -Wextra


compile_visitor:
	mkdir -p ./bin
	gcc $(gcc_options) ./src/visitor/index.c ./src/protocol.c -o ./bin/visitor.exe

visitor: compile_visitor
	./bin/visitor.exe 127.0.0.1 8000 m 3


compile_hotel:
	mkdir -p ./bin
	gcc $(gcc_options) ./src/hotel/index.c ./src/hotel/rooms/rooms.c ./src/protocol.c -o ./bin/hotel.exe

hotel: compile_hotel
	./bin/hotel.exe 8000
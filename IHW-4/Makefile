gcc_options = -O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=gnu17 -Werror -Wall -Wextra


compile_visitor:
	mkdir -p ./bin
	gcc $(gcc_options) ./src/visitor/index.c ./src/protocol.c -o ./bin/visitor.exe
visitor: compile_visitor
	./bin/visitor.exe 127.0.0.1 8000 m 3


compile_hotel:
	mkdir -p ./bin
	gcc $(gcc_options) ./src/hotel/index.c ./src/protocol.c ./src/hotel/rooms/rooms.c ./src/hotel/log/log.c -o ./bin/hotel.exe
hotel: compile_hotel
	./bin/hotel.exe 8000 225.0.0.0 9000


compile_logger:
	mkdir -p ./bin
	gcc $(gcc_options) ./src/logger/index.c ./src/protocol.c -o ./bin/logger.exe
logger: compile_logger
	./bin/logger.exe 225.0.0.0 9000


compile_all: compile_hotel compile_visitor compile_logger

test_small: compile_all
	node testing/run.mjs small 8000 225.0.0.0 9000
test_big: compile_all
	node testing/run.mjs big 8000 225.0.0.0 9000
test: test_small test_big
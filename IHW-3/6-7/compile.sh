cd "$(dirname "$0")"

options="-O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=gnu17 -Werror -Wall -Wextra"
mkdir -p ./bin

gcc $options ./src/hotel/index.c ./src/hotel/rooms/rooms.c ./src/hotel/sem/sem.c ./src/hotel/shm/shm.c ./src/hotel/log/log.c -o ./bin/hotel.exe -lpthread -lrt
gcc $options ./src/visitor/index.c ./src/visitor/log/log.c -o ./bin/visitor.exe -lpthread -lrt
gcc $options ./src/logger/index.c -o ./bin/logger.exe -lpthread -lrt
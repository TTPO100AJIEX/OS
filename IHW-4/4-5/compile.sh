cd "$(dirname "$0")"

options="-O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=gnu17 -Werror -Wall -Wextra"
mkdir -p ./bin

gcc $options ./src/hotel/index.c ./src/hotel/rooms/rooms.c ./src/hotel/log/log.c ./src/hotel/utils/sem/sem.c ./src/hotel/utils/shm/shm.c -o ./bin/hotel.exe
gcc $options ./src/visitor/index.c -o ./bin/visitor.exe
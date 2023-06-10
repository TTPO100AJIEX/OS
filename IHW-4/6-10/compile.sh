cd "$(dirname "$0")"

options="-O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=gnu17 -Werror -Wall -Wextra"
mkdir -p ./bin

gcc $options ./src/hotel/index.c ./src/hotel/rooms/rooms.c -o ./bin/hotel.exe
gcc $options ./src/visitor/index.c -o ./bin/visitor.exe
# gcc $options ./src/logger/index.c -o ./bin/logger.exe
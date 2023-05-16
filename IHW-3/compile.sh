options="-O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=gnu17 -Werror -Wall -Wextra"
mkdir -p ./$1/bin

gcc $options ./$1/src/hotel/index.c ./$1/src/hotel/rooms/rooms.c ./$1/src/hotel/sem/sem.c ./$1/src/hotel/shm/shm.c ./$1/src/hotel/log/log.c -o ./$1/bin/hotel.exe -lpthread -lrt
gcc $options ./$1/src/visitor/index.c ./$1/src/visitor/log/log.c -o ./$1/bin/visitor.exe -lpthread -lrt
options="-O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=gnu17 -Werror -Wall -Wextra"
gcc $options src/hotel/index.c src/hotel/rooms/rooms.c src/hotel/sem/sem.c src/hotel/shm/shm.c -o hotel.exe -lpthread
./hotel.exe 8000
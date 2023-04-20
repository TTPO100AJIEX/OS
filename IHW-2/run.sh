options="-O2 -fsanitize=address,undefined,leak -fno-sanitize-recover=all -std=c17 -Werror -Wall -Wextra"
# gcc $options 4/src/hotel/hotel.c 4/src/visitor/visitor.c 4/src/index.c -o 4/bin/index.exe -lpthread -lrt
gcc 4/src/utils/utils.c 4/src/log/log.c 4/src/hotel/hotel.c 4/src/visitor/visitor.c 4/src/index.c -o 4/bin/index.exe -lpthread -lrt
./4/bin/index.exe 4/output/output.txt
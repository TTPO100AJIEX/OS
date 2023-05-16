options="-O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=gnu17 -Werror -Wall -Wextra"
gcc $options src/visitor/index.c src/visitor/log/log.c -o visitor.exe -lpthread -lrt
./visitor.exe 127.0.0.1 7000 m 2
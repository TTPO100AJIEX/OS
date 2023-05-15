options="-O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=gnu17 -Werror -Wall -Wextra"
gcc $options src/visitor/index.c -o visitor.exe -lpthread
./visitor.exe
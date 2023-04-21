options="-O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=gnu17 -Werror -Wall -Wextra"
mkdir -p ./$1/bin
gcc $options $1/src/utils/utils.c $1/src/log/log.c $1/src/hotel/hotel.c $1/src/visitor/visitor.c $1/src/index.c -o $1/bin/index.exe -lpthread -lrt
cat small_test.txt | ./$1/bin/index.exe $1/output/small.txt
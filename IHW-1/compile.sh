options="-O2 -fsanitize=address,undefined -fno-sanitize-recover=all -std=c17 -Werror -Wsign-compare"

if [ $1 -lt 8 ]
then
    gcc -Ilib -DCHUNK_SIZE=5000 $options lib/pipes/solve.c lib/pipes/transfer.c $1/index.c -o $1/bin/index.exe
fi

if [ $1 -eq 8 ] || [ $1 -eq 9 ]
then
    if [ $1 -eq 8 ]
    then
        chunksize=5000
    fi
    if [ $1 -eq 9 ]
    then
        chunksize=200
    fi

    gcc -Ilib -DCHUNK_SIZE=$chunksize $options lib/pipes/solve.c lib/pipes/transfer.c $1/io.c -o $1/bin/io.exe
    gcc -Ilib -DCHUNK_SIZE=$chunksize $options lib/pipes/solve.c lib/pipes/transfer.c $1/solver.c -o $1/bin/solver.exe
fi

if [ $1 -eq 10 ]
then
    gcc -Ilib -DCHUNK_SIZE=200 $options lib/messages/input.c lib/messages/solve.c lib/messages/output.c $1/io.c -o $1/bin/io.exe
    gcc -Ilib -DCHUNK_SIZE=200 $options lib/messages/input.c lib/messages/solve.c lib/messages/output.c $1/solver.c -o $1/bin/solver.exe
fi

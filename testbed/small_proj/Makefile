AFL_PATH=$(realpath ../aflpp)
CC=../../lscov/build/lscov-clang

export AFL_USE_ASAN
export AFL_USE_UBSAN

all:
	#$(CC) main.c 
	AFL_PATH=$(AFL_PATH) $(CC) main.c -c -emit-llvm -o main.bc
	llvm-dis main.bc

clean:
	rm -f main.bc main.ll a.out

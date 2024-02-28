AFL_PATH=$(realpath ../aflpp)
CC=../../lscov/build/lscov-clang

export AFL_USE_ASAN
export AFL_USE_UBSAN

all:
	$(CC) main.c #-o3 -c -emit-llvm -o main.bc
	#llvm-dis main.bc

clean:
	rm -f main.bc main.ll a.out

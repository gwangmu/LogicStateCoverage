CC=../../lscov/build/lscov-clang

all:
	$(CC) main.c -c -emit-llvm -o main.bc
	llvm-dis main.bc

clean:
	rm -f main.bc main.ll a.out

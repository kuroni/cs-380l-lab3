#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024768
#define JUMP 16384

int fib[SIZE];

int main(int argc, char** argv) {
    // fib = (int*)malloc(SIZE * sizeof(int))
#define FIB(i) fib[(i) * JUMP]
    FIB(0) = FIB(1) = 1;
    for (int i = 2; i * JUMP < SIZE; i++) {
        FIB(i) = FIB(i - 1) + FIB(i - 2);
    }
    printf("%d\n", FIB(SIZE / JUMP));
}

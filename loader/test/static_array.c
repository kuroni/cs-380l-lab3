#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024768

int fib[SIZE];

int main(int argc, char** argv) {
    // fib = (int*)malloc(SIZE * sizeof(int));
    fib[0] = fib[1] = 1;
    for (int i = 2; i < SIZE; i++) {
        fib[i] = fib[i - 1] + fib[i - 2];
    }
    printf("%d\n", fib[SIZE - 1]);
}

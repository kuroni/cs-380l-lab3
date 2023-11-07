#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int size = atoi(argv[1]);
    int* fib = (int*)malloc(size * sizeof(int));
    fib[0] = fib[1] = 1;
    for (int i = 2; i < size; i++) {
        fib[i] = fib[i - 1] + fib[i - 2];
    }
    printf("%d\n", fib[size - 1]);
}

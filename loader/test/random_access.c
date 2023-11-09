#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 268435456
#define COUNT 1024

int fib[SIZE];

int position() {
    return (rand() * rand()) & (SIZE - 1);
}

int main(int argc, char** argv) {
    srand(time(NULL));
    int a = position(), b = position();
    fib[a] = fib[b] = 1;
    for (int i = 2; i < COUNT; i++) {
        int c = position();
        fib[c] = fib[a] + fib[b];
        a = b; b = c;
    }
    printf("%d\n", fib[b]);
}

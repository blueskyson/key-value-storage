#include "bloom.h"
#include "bench.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
int main() {
    bloom b;
    unsigned long long *num = new unsigned long long;
    double f = 0;
    int all = 200000;
    srand(time(NULL));
    for (unsigned long long i = 0; i < all; i++) {
        unsigned long long t = rand() % 10000000;
        b.bloom_add(&t);
    }
    for (unsigned long long i = all; i < all*2; i++) {
        unsigned long long t = (rand() % 10000000) + 10000000;
        if (b.bloom_get(&t)) {
            f++;
        }
    }
    printf("\nfalse positive: %lf\n", f / all);
}
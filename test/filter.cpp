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
    int all = 100000;
    srand(time(NULL));
    for (unsigned long long i = 0; i < all; i++) {
        *num = rand();
        if (b.bloom_get(num)) {
            f++;
        }
        b.bloom_add(num);
    }
    printf("\nfalse positive: %lf\n", f / all);
}
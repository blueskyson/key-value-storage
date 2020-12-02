#include "bench.h"
#include "kvs.h"

/* turn a char array into unsigned long long */
unsigned long long fast_atoull(const char *str)
{
    unsigned long long val = 0;
    while (*str) {
        val = (val << 1) + (val << 3) + (*(str++) - 48);
    }
    return val;
}

int main(int argc, char *argv[]) {
    double start, end; //
    start = tvgetf();  //
    DataBase database(argv[1]);

}
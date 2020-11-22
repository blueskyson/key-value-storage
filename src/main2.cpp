#include <stdio.h>
#include <string.h>
#include <fcntl.h>     //open
#include <sys/mman.h>  //mmap
#include <sys/stat.h>
#include <stdlib.h>

#include "bench.h"
#include "kvs.h"

int main(int argc, char *argv[])
{
    char *in_file = argv[1];
    char *out_file = new char[15];
    unsigned long long idx = strlen(in_file) - 5;
    while (idx != 0 && in_file[idx - 1] != '/') {
        idx--;
    }
    int idx2 = 0;
    while (in_file[idx] != '.') {
        out_file[idx2++] = in_file[idx++];
    }
    memcpy(out_file + idx2, ".output\0", 8);

    double start, end; //
    start = tvgetf();  //
    FILE *input = fopen(in_file, "r");
    if (!input) {
        puts("cannot open input file");
        return 0;
    }
    char insbuff[5];
    char keybuff[20];
    char valbuff[129];
    idx = 0;
    while (fscanf(input, "%s", insbuff) != EOF) {
        if (strcmp(insbuff, "GET") == 0) {
            fscanf(input, "%s", keybuff);
        } else {
            fscanf(input, "%s", keybuff);
            fscanf(input, "%s", valbuff);
        }
    }
    end = tvgetf();
    printf("\ntime: %lf\n", end - start);
    return 0;
}
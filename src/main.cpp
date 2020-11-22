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
    struct stat s;
    int fd = open(in_file, O_RDONLY);
    if (fd < 0) {
        puts("cannot open iniput file");
        return 0;
    }
    int status = fstat(fd, &s);
    if (status < 0) {
        puts("cannot get status of iniput file");
        return 0;
    }
    
    char *input_map = (char*) mmap(0, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    // char str[16];
    // sprintf(str, "%ld", s.st_size);
    char insbuff[5];
    char keybuff[20];
    char valbuff[129];
    
    const char endchar = (char) 0x0a;
    idx = 0;
    while (idx < s.st_size) {
        unsigned buffidx = 0;
        while (input_map[idx] != ' ') {
            insbuff[buffidx++] = input_map[idx++];
        }
        insbuff[buffidx] = '\0';
        //puts(insbuff);
        buffidx = 0;
        while (input_map[idx] != endchar)
            idx++;
        idx++;
    }

    end = tvgetf();
    printf("\ntime: %lf\n", end - start);
    return 0;
}
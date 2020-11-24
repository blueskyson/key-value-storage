#include <stdio.h>
#include <string.h>
#include <fcntl.h>     //open
#include <sys/mman.h>  //mmap
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "bench.h"
#include "kvs.h"

unsigned long long  fast_atoull(const char *str)
{
    unsigned long long val = 0;
    while(*str)
    {
        val = (val << 1) + (val << 3) + (*(str++) - 48);
    }
    return val;
}

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
    char *input_map = (char*) mmap(0, s.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    const char ENDCHAR = (char) 0x0a;
    input_map[s.st_size] = ENDCHAR;

    char insbuff;
    char keybuff1[20], keybuff2[20];
    char valbuff[129];
    const char PUT = 'P';
    const char GET = 'G';
    const char SCAN = 'S';

    SkipList memtable(4);
    idx = 0;
    while (idx < s.st_size) {
        insbuff = input_map[idx++];
        while (input_map[idx] != ' ') {
            idx++;
        }
        idx++;
        int kidx1 = 0, kidx2 = 0;
        switch (insbuff) {
            char *pEnd;
            case PUT:
                while (input_map[idx] != ' ') {
                    keybuff1[kidx1++] = input_map[idx++];
                }
                idx++;
                keybuff1[kidx1] = '\0';
                for (int i = 0; i < 128; i++, idx++) {
                    valbuff[i] = input_map[idx];
                }
                valbuff[128] = '\0';
                idx++;
                //strtoull(keybuff1, &pEnd, 10);
                memtable.put(fast_atoull(keybuff1), valbuff);
            break;

            case GET:
                while (input_map[idx] != ENDCHAR) {
                    keybuff1[kidx1++] = input_map[idx++];
                }
                idx++;
                keybuff1[kidx1] = '\0';
            break;

            case SCAN:
                while (input_map[idx] != ' ') {
                    keybuff1[kidx1++] = input_map[idx++];
                }
                idx++;
                keybuff1[kidx1] = '\0';

                while (input_map[idx] != ENDCHAR) {
                    keybuff2[kidx2++] = input_map[idx++];
                }
                idx++;
                keybuff2[kidx2] = '\0';
            break;
        }
    }

    munmap(input_map, s.st_size);
    close(fd);

    end = tvgetf();
    printf("\ntime: %lf\n", end - start);
    return 0;
}
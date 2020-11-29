#include "bench.h"
#include "kvs.h"

/* turn a char array into unsigned long long */
unsigned long long  fast_atoull(const char *str)
{
    unsigned long long val = 0;
    while (*str) {
        val = (val << 1) + (val << 3) + (*(str++) - 48);
    }
    return val;
}

int main(int argc, char *argv[])
{
    double start, end; //
    start = tvgetf();  //
    DataBase database(argv[1]);

    /* open input file */
    struct stat s;
    int fd = open(argv[1], O_RDONLY);
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
    input_map[s.st_size] = ENDCHAR;

    char insbuff;
    char keybuff1[20], keybuff2[20];
    char valbuff[129];
    const char PUT = 'P';
    const char GET = 'G';
    const char SCAN = 'S';

    unsigned long long idx = 0;
    int debug = 168; //

    while (idx < s.st_size && debug) {
        debug--; //
        insbuff = input_map[idx++];
        while (input_map[idx] != ' ') {
            idx++;
        }
        idx++;
        int kidx1 = 0, kidx2 = 0;
        switch (insbuff) {
            char* output_str;
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
                database.put(fast_atoull(keybuff1), valbuff);
            break;

            case GET:
                while (input_map[idx] != ENDCHAR) {
                    keybuff1[kidx1++] = input_map[idx++];
                }
                idx++;
                keybuff1[kidx1] = '\0';
                database.get(fast_atoull(keybuff1));
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
                database.scan(fast_atoull(keybuff1), fast_atoull(keybuff2));
            break;
        }
    }
    database.save_output();
    //database.show();
    database.mem_to_disk();
    database.meta_to_disk();
    munmap(input_map, s.st_size);
    close(fd);

    end = tvgetf();
    printf("\ntime: %lf\n", end - start);
    return 0;
}
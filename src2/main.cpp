#include "bench.h"
#include "kvs.h"

int main(int argc, char *argv[])
{
    double start, end; //
    start = tvgetf();  //
    DataBase database(argv[1]);
    
    /* read meta from disk */
    if (access("storage/0", F_OK) == 0) {
        database.meta_from_disk();
        database.mem_from_disk();
    }

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

    /* read instructions */
    int debug = 2;
    while (idx < s.st_size && debug) {
        debug--;
        insbuff = input_map[idx++];
        while (input_map[idx] != ' ') {
            idx++;
        }
        idx++;
        int kidx1 = 0, kidx2 = 0;
        switch (insbuff) {
            case PUT:
                /* get key */
                while (input_map[idx] != ' ') {
                    keybuff1[kidx1++] = input_map[idx++];
                }
                idx++;
                keybuff1[kidx1] = '\0';

                /* get value */
                for (int i = 0; i < 128; i++, idx++) {
                    valbuff[i] = input_map[idx];
                }
                valbuff[128] = '\0';
                idx++;
                database.put2(fast_atoull(keybuff1), valbuff);
            break;
        }
        getchar();
    }
    return 0;
}
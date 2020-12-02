#include "bench.h"
#include "kvs.h"
#include <pthread.h>

int main(int argc, char *argv[])
{
    double start, end; //
    start = tvgetf();  //
    DataBase database(argv[1]);
    
    /* read meta from disk */
    if (access("storage/0", F_OK) == 0) {
        database.Meta.get_meta();
        database.get_data();
        database.Meta.show();
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
    int debug = 0;
    while (idx < s.st_size && debug < 162) {
        // char c;
        // scanf("%c", &c);
        // if (c == 'a') {
        //     database.Meta.show_pages();
        // }
        
        debug++;
        //printf("ins: %d\n", debug);
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
                if (database.key_num == MAXKEYS) {
                    database.flush_data();
                    //database.Meta.show();
                }
                //printf("ins: %3d, put %s\n", debug, keybuff1);
                break;
            
            case GET:
                while (input_map[idx] != ENDCHAR) {
                    keybuff1[kidx1++] = input_map[idx++];
                }
                idx++;
                keybuff1[kidx1] = '\0';
                char *str = database.get2(fast_atoull(keybuff1));
                if (str != nullptr)
                    printf("ins: %3d, %128s\n", debug, str);
                else 
                    printf("ins: %3d, EMPTY\n", debug);
            break;
        }
        //database.show();
    }
    // database.show();
    database.flush_data();
    database.Meta.flush_meta();
    return 0;
}
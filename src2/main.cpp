#include "bench.h"
#include "kvs.h"
#include <deque>
using namespace std;

struct out_data {
    bool *feeding;
    deque<char*> *queue;
    char *in_name;
};

void *print_routine(void *args) {
    out_data *shared = (out_data*) args;
    char out_file[30];
    int ii = strlen(shared->in_name) - 5;
    while (ii != 0 && shared->in_name[ii - 1] != '/') {
        ii--;
    }
    int idx2 = 0;
    while (shared->in_name[ii] != '.') {
        out_file[idx2++] = shared->in_name[ii++];
    }
    memcpy(out_file + idx2, ".output\0", 8);
    FILE *fp = fopen(out_file, "wb");
    unsigned write = 0;
    while(1) {
        if ((*shared->queue).empty()) {
            if (*shared->feeding == false) {
                fclose(fp);
                pthread_exit(NULL);
            }
            continue;
        }
        if ((*shared->queue).front() == nullptr) {
            //puts("EMPTY");
            fwrite("\nEMPTY", 6, 1, fp);
            (*shared->queue).pop_front();
        } else {
            if (write)
                fwrite("\n", 1, 1, fp);
            //puts((*shared->queue).front());
            fwrite((*shared->queue).front(), 128, 1, fp);
            delete[] (*shared->queue).front();
            (*shared->queue).pop_front();
            write++;
        }
    }
}

int main(int argc, char *argv[])
{
    double start, end; //
    start = tvgetf();  //
    DataBase database;
    
    /* read meta from disk */
    if (access("storage/0", F_OK) == 0) {
        database.Meta.get_meta();
        database.get_data();
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

    /* init */
    char insbuff;
    char keybuff1[20], keybuff2[20];
    char valbuff[129];
    const char PUT = 'P';
    const char GET = 'G';
    const char SCAN = 'S';
    unsigned long long idx = 0;
    pthread_t print_thread;
    out_data out;
    out.feeding = new bool(true);
    out.queue = new deque<char*>;
    out.in_name = argv[1];

    pthread_create(&print_thread, NULL, print_routine, &out);
    /* read instructions */
    int debug = 0;
    while (idx < s.st_size && debug < 169) {
        char c;
        // if (debug > 160) {
        //     scanf("%c", &c);
        //     if (c == 'a') {
        //         database.Meta.show_pages();
        //     }
        // }
        debug++;
        insbuff = input_map[idx++];
        while (input_map[idx] != ' ') {
            idx++;
        }
        idx++;
        int kidx1 = 0, kidx2 = 0;
        switch (insbuff) {
            char *str;
            char **line;
            unsigned long long key1, key2;
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
                }
                break;
            
            case GET:
                while (input_map[idx] != ENDCHAR) {
                    keybuff1[kidx1++] = input_map[idx++];
                }
                idx++;
                keybuff1[kidx1] = '\0';
                str = database.get2(fast_atoull(keybuff1));
                /* print routine */
                out.queue->push_back(str);
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
                key1 = fast_atoull(keybuff1);
                key2 = fast_atoull(keybuff2);
                
                line = database.scan2(key1, key2);
                for (int i = 0; i <= key2 - key1; i++) {
                    /* print routine */
                    out.queue->push_back(line[i]);
                }
                //delete[] line;
                break;
        }
    }
    *out.feeding = false;
    database.flush_data();
    database.Meta.flush_meta();
    pthread_join(print_thread, NULL);
    return 0;
}
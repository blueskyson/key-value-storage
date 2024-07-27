#include <string.h>
#include <unistd.h>
#include "lib/kvs.h"

int main(int argc, char *argv[])
{
    Kvs *kvs = NULL;
    if (init_kvs(&kvs) == -1) {
        destruct_kvs(kvs);
        return -1;
    }

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input> <output>\n", argv[0]);
        return 1;
    }

    FILE *input_file = fopen(argv[1], "r");
    if (!input_file) {
        perror("fopen");
        return 1;
    }

    FILE *output_file = fopen(argv[2], "w");
    if (!output_file) {
        perror("fopen");
        return 1;
    }

    /* read instructions */
    char instruction;
    while (fscanf(input_file, " %c", &instruction) == 1) {
        unsigned long long key;
        char value[129];
        switch (instruction) {
            /* PUT */
            case 'P':
                if (fscanf(input_file, "%llu %s", &key, value) != 2) {
                    fprintf(stderr, "Error reading PUT\n");
                    break;
                }
                kvs->put(kvs, key, value);
                break;
            
            /* GET */
            case 'G':
                if (fscanf(input_file, "%llu", &key) != 1) {
                    fprintf(stderr, "Error reading GET\n");
                    break;
                }
                char *val = kvs->get(kvs, key);
                fprintf(output_file, "%llu %.*s\n", key, 128, val);
                free(val);
                break;

            default:
                fprintf(stderr, "Unknown instruction: %c\n", instruction);
        }
    }

    fclose(input_file);
    fclose(output_file);
    kvs->save_to_disk(kvs);
    destruct_kvs(kvs);
    return 0;
}
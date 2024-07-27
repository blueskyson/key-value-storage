#include <stdbool.h>

typedef struct bloomfilter Bloomfilter;
typedef void (*bloomfilter_add_func_t)(Bloomfilter *self, unsigned long long key);
typedef bool (*bloomfilter_get_func_t)(Bloomfilter *self, unsigned long long key);

int init_bloomfilter(Bloomfilter **self);

struct bloomfilter {
    unsigned int size, complement;
    unsigned char *table;
    bloomfilter_add_func_t add;
    bloomfilter_get_func_t get;
};

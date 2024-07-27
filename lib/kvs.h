#include <stdio.h>
#include <stdlib.h>

#include "Metadata.h"
#include "Skiplist.h"

/* forward declaration */
typedef struct kvs Kvs;
typedef int (*put_func_t)(Kvs *self, unsigned long long key, char *value);
typedef char* (*get_func_t)(Kvs *self, unsigned long long key);
typedef void (*save_func_t)(Kvs *self);

int init_kvs(Kvs **self);
void destruct_kvs(Kvs *self);

struct kvs {
    int keyCount, pageCount;
    MetadataManager *metadataManager;
    Skiplist *skiplist;
    Bloomfilter *bloomfilter;
    put_func_t put;
    get_func_t get;
    save_func_t save_to_disk;
};

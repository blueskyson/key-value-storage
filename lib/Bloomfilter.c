#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Bloomfilter.h"

static unsigned int djb2(const void *_str)
{
    const char *str = (const char*)_str;
    unsigned int hash = 5381;
    char c;
    for (int i = 0; i < 8; i++) {
        c = str[i];
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static unsigned int jenkins(const void *_str)
{
    const char *key = (const char*)_str;
    unsigned int hash = 0;
    for (int i = 0; i < 8; i++) {
        hash += *key;
        hash += (hash << 10);
        hash ^= (hash >> 6);
        key++;
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

void bloomfilter_add_impl(Bloomfilter *self, unsigned long long key)
{
    unsigned int hash;
    hash = djb2(&key);
    self->table[hash >> self->complement] |= 0x80 >> (hash & 7);
    hash = jenkins(&key);
    self->table[hash >> self->complement] |= 0x80 >> (hash & 7);
}

bool bloomfilter_get_impl(Bloomfilter *self, unsigned long long key)
{
    unsigned int hash;
    hash = djb2(&key);
    if (!(self->table[hash >> self->complement] & (0x80 >> (hash & 7)))) {
        return false;
    }
    hash = jenkins(&key);
    if (!(self->table[hash >> self->complement] & (0x80 >> (hash & 7)))) {
        return false;
    }
    return true;
}

int init_bloomfilter(Bloomfilter **self)
{
    if (NULL == (*self = malloc(sizeof(Bloomfilter))))
        return -1;
    (*self)->add = bloomfilter_add_impl;
    (*self)->get = bloomfilter_get_impl;
    (*self)->size = 131072;     // 2^17
    (*self)->complement = 15;   // 32 - log2(2^17)
    (*self)->table = malloc((*self)->size * sizeof(unsigned char));
    if ((*self)->table == NULL) {
        return -1;
    }

    memset((*self)->table, 0, (*self)->size);
    return 0;
};
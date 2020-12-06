#include <stdio.h>
#include <string.h>
class bloom {
public:
    unsigned int size, complement;
    unsigned char *table;
    bloom() {
        size = 1 << 18;
        complement = 14; // 32 - log2(size)
        table = new unsigned char[size];
        memset(table, 0, size);
    }

    unsigned int djb2(const void *_str)
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

    unsigned int jenkins(const void *_str)
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

    void bloom_add(unsigned long long *k) {
        unsigned int hash;
        hash = djb2(k);
        table[hash >> complement] |= 0x80 >> (hash & 7);
        hash = jenkins(k);
        table[hash >> complement] |= 0x80 >> (hash & 7);
    }

    bool bloom_get(unsigned long long *k) {
        unsigned int hash;
        hash = djb2(k);
        if (!(table[hash >> complement] & (0x80 >> (hash & 7)))) {
            return false;
        }
        hash = jenkins(k);
        if (!(table[hash >> complement] & (0x80 >> (hash & 7)))) {
            return false;
        }
        return true;
    }

    void show() {
        for (unsigned int i = 0; i < size; i++) {
            if (table[i] & 0x01)
                putchar('1');
            else
                putchar('0');
            if (table[i] & 0x02)
                putchar('1');
            else
                putchar('0');
            if (table[i] & 0x04)
                putchar('1');
            else
                putchar('0');
            if (table[i] & 0x08)
                putchar('1');
            else
                putchar('0');
            if (table[i] & 0x10)
                putchar('1');
            else
                putchar('0');
            if (table[i] & 0x20)
                putchar('1');
            else
                putchar('0');
            if (table[i] & 0x40)
                putchar('1');
            else
                putchar('0');
            if (table[i] & 0x80)
                putchar('1');
            else
                putchar('0');
        }
    }
};
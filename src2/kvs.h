#ifndef STDIO_STDLIB_TIME_STRING
#define STDIO_STDLIB_TIME_STRING
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#endif

#include <fcntl.h>     //open
#include <sys/mman.h>  //mmap
#include <sys/stat.h>
#include <unistd.h>
#include "bloom.h"

#define MAXTEXT 1000000
#define FULLTEXT 999800
#define MAXKEYS 10
const char ENDCHAR = (char) 0x0a;

struct node {
    unsigned long long *key;
    char *value;
    node *right[4];
};

struct meta {
    unsigned long long start, end;
    int key_num, page;
    bloomfilter* bloom;
    meta* next;
};

class MetaData {
public:
    meta *head, *tail, *not_full;
    int page_num;

    MetaData() {
        head = nullptr;
        tail = nullptr;
        not_full = nullptr;
        page_num = 1;
    }

    void get_meta() {
        meta *cur = head, *prev_tail;
        struct stat s;
        int fd = open("storage/0", O_RDONLY);
        int status = fstat(fd, &s);
        char *map = (char*) mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
        unsigned long long idx = 0;
        while (idx < s.st_size) {
            meta *m = new meta;
            memcpy(&(m->start), (map + idx), 8);
            memcpy(&(m->end), (map + idx + 8), 8);
            memcpy(&(m->key_num), (map + idx + 16), 4);
            m->bloom = new bloomfilter;
            memcpy(m->bloom->table, (map + idx + 20), 131072);
            m->next = nullptr;
            m->page = page_num++;
            printf("%d,\n", m->page);
            if (head == nullptr) {
                head = m;
                cur = m;
            } else {
                cur->next = m;
                cur = cur->next;
            }
            prev_tail = tail;
            tail = cur;
            idx += 131092;
        }

        if (tail != nullptr && tail->key_num != MAXKEYS) {
            not_full = tail;
            tail = prev_tail;
            if (tail != nullptr)
                tail->next = nullptr;
            else
                head = nullptr;
        }
    }

    void flush_meta() {
        meta* p = head;
        if (!p)
            return;    
        FILE *fp = fopen("storage/0", "wb");
        for (;p != nullptr; p = p->next) {
            fwrite(&(p->start), 8, 1, fp);
            fwrite(&(p->end), 8, 1, fp);
            fwrite(&(p->key_num), 4, 1, fp);
            fwrite(p->bloom->table, p->bloom->size, 1, fp);
        }
        fclose(fp);
    }

    void show() {
        for (meta *m = head; m != nullptr; m = m->next) {
            printf("start: %llu, end: %llu, key_num: %d, page: %d\n",
                    m->start, m->end, m->key_num, m->page);
        }
    }

    void show_pages() {
        if (head == nullptr) {
            puts("no pages");
            return;
        }
        for (meta *m = head; m != nullptr; m = m->next) {
            char file_str[15];
            sprintf(file_str, "storage/%d", m->page);
            struct stat s;
            int fd = open(file_str, O_RDONLY);
            int status = fstat(fd, &s);
            char *map = (char*) mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
            unsigned idx = 0;
            printf("page %d:\n", m->page);
            while (idx < s.st_size) {
                unsigned long long* tmp = (unsigned long long*)(map + idx);
                char s[128];
                memcpy(s, map + (idx + 8), 128);
                printf("%llu %s\n", *tmp, s);
                idx += 136;
            }
            munmap(map, s.st_size);
            close(fd);
            puts("");
        }
    }

    void show_not_full() {
        if (!not_full) {
            puts("not_null is nullptr");
            return;
        }
        printf("not full start: %llu, end: %llu, key_num: %d, page: %d\n",
            not_full->start, not_full->end, not_full->key_num, not_full->page);
    }
};


class DataBase {
public:
    MetaData Meta;
    node *head;
    bloomfilter* bloom;
    char* in_file, *out_file, *txtbuff;
    int key_num, page_num;
    size_t text_size;

    DataBase(char* in_name)
    {
        srand(time(NULL));
        bloom = new bloomfilter;
        key_num = 0;  // key number in skiplist
        page_num = 1; // page index of current skiplist

        /* create skip list */
        head = new node;
        for (int i = 0; i < 4; i++) {
            head->key = nullptr;
            head->right[i] = nullptr;
        }
        
        /* generate output filename */
        in_file = strdup(in_name);
        out_file = new char[16];
        int ii = strlen(in_file) - 5;
        while (ii != 0 && in_file[ii - 1] != '/') {
            ii--;
        }
        int idx2 = 0;
        while (in_file[ii] != '.') {
            out_file[idx2++] = in_file[ii++];
        }
        memcpy(out_file + idx2, ".output\0", 8);
        
        text_size = 0;
        txtbuff = new char[MAXTEXT];
    }

    void get_data() {
        if (Meta.not_full == nullptr || Meta.not_full->key_num == MAXKEYS) {
            return;
        }
        key_num = Meta.not_full->key_num;
        page_num = Meta.not_full->page;
        char fname[15];
        sprintf(fname, "storage/%d", page_num);
        FILE *fp = fopen(fname, "rb");
        node* cursor[4];
        cursor[3] = head;
        cursor[2] = head;
        cursor[1] = head;
        cursor[0] = head;

        for (int i = 0; i < key_num; i++) {
            /* level 0 */
            node* new_node = new node;
            new_node->key = new unsigned long long;
            new_node->value = new char[128];
            fread(new_node->key, 8, 1, fp);
            fread(new_node->value, 128, 1, fp);
            cursor[0]->right[0] = new_node;
            cursor[0] = new_node;
            bloom->bloom_add(new_node->key);

            /* other levels */
            new_node->right[0] = nullptr;
            new_node->right[1] = nullptr;
            new_node->right[2] = nullptr;
            new_node->right[3] = nullptr;
            short cnt = 1;
            while (cnt < 4 && (rand() & 1)) {
                cursor[cnt]->right[cnt] = new_node;
                cursor[cnt] = new_node;
                cnt++;
            }
        }
        fclose(fp);
        remove(fname);
        remove("storage/0");
    }

    void put2(unsigned long long k, char* v)
    {
        /* check if the key is in skiplist */
        int level = 3, searchflag = 0;
        node* cursor[4];
        cursor[3] = head;
        if (bloom->bloom_get(&k)) {
            while(level) {
                if (cursor[level]->right[level] == nullptr || k < *(cursor[level]->right[level]->key)) {
                    cursor[level-1] = cursor[level];
                    level--;
                } else {
                    cursor[level] = cursor[level]->right[level];
                }
            }
            while (cursor[0]->right[0] != nullptr && k >= *(cursor[0]->right[0]->key))
                cursor[0] = cursor[0]->right[0];
            
            /* the key is actually in skiplist */
            if (cursor[0]->key && *(cursor[0]->key) == k) {
                strcpy(cursor[0]->value, v);
                return;
            }
            /* false positive occurs */
            searchflag = 1;
        }

        /* check if the key is in disk */
        for (meta* m = Meta.head; m != nullptr; m = m->next) {
            if (k >= m->start && k <= m->end && m->bloom->bloom_get(&k)) {
                char file_str[15];
                sprintf(file_str, "storage/%d", m->page);
                struct stat s;
                int fd = open(file_str, O_RDWR);
                int status = fstat(fd, &s);
                char *map = (char*) mmap(0, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                unsigned idx = 0;
                while (idx < s.st_size) {
                    unsigned long long* tmp = (unsigned long long*)(map + idx);
                    if (*tmp == k) {
                        memcpy(map + (idx + 8), v, 128);
                        munmap(map, s.st_size);
                        close(fd);
                        return;
                    }
                    idx += 136;
                }
                munmap(map, s.st_size);
                close(fd);
            }
        }

        /* key in neither disk nor skiplist */
        /* then put it into skiplist*/
        if (!searchflag) {
            while(level) {
                if (cursor[level]->right[level] == nullptr || k < *(cursor[level]->right[level]->key)) {
                    cursor[level-1] = cursor[level];
                    level--;
                } else {
                    cursor[level] = cursor[level]->right[level];
                }
            }
            while (cursor[0]->right[0] != nullptr && k > *(cursor[0]->right[0]->key))
                cursor[0] = cursor[0]->right[0];            
        }

        unsigned long long *new_k = new unsigned long long(k);
        char* new_v = strdup(v);
        bloom->bloom_add(&k);
        key_num++;

        /* level 0 */
        node* new_node = new node;
        new_node->key = new_k;
        new_node->value = new_v;
        new_node->right[0] = cursor[0]->right[0];
        cursor[0]->right[0] = new_node;
        cursor[0] = new_node;
         
        /* other levels */
        new_node->right[1] = nullptr;
        new_node->right[2] = nullptr;
        new_node->right[3] = nullptr;
        short cnt = 1;
        while (cnt < 4 && (rand() & 1)) {
            new_node->right[cnt] = cursor[cnt]->right[cnt];
            cursor[cnt]->right[cnt] = new_node;
            cnt++;
        }
    }
    
    /* flush skiplist to disk and add meta data */
    void flush_data() {
        node* p = head->right[0];
        if (p == nullptr) {
            return;
        }

        meta *m = new meta;
        m->bloom = bloom;
        m->start = *(p->key);
        char fname[15];
        sprintf(fname, "storage/%d", page_num);
        FILE *fp = fopen(fname, "wb");
        for (; p->right[0] != nullptr;) {
            fwrite(p->key, 8, 1, fp);
            fwrite(p->value, 128, 1, fp);
            node* del = p;
            p = p->right[0];
            delete del->key;
            delete[] del->value;
            delete del;
        }
        fwrite(p->key, 8, 1, fp);
        fwrite(p->value, 128, 1, fp);
        m->end = *(p->key);
        m->key_num = key_num;
        m->page = page_num;
        m->next = nullptr;
        if (Meta.tail != nullptr) {
            Meta.tail->next = m;
            Meta.tail = m;
        } else {
            Meta.head = m;
            Meta.tail = m;
        }    
        delete p->key;
        delete[] p->value;
        delete p;

        head->right[0] = nullptr;
        head->right[1] = nullptr;
        head->right[2] = nullptr;
        head->right[3] = nullptr;
        key_num = 0;
        page_num++;
        bloom = new bloomfilter;
        fclose(fp);
    }

    char* get2(unsigned long long k) {
        /* check if the key is in skiplist */
        int level = 3, searchflag = 0;
        node* cur = head;
        if (bloom->bloom_get(&k)) {
            while(level) {
                if (cur->right[level] == nullptr || k < *(cur->right[level]->key)) {
                    level--;
                } else {
                    cur = cur->right[level];
                }
            }
            while (cur->right[0] != nullptr && k >= *(cur->right[0]->key))
                cur = cur->right[0];
            
            /* the key is actually in skiplist */
            if (cur->key && *(cur->key) == k) {
                return cur->value;
            }
            /* false positive occurs */
            searchflag = 1;
        }

        /* check if the key is in disk */
        for (meta* m = Meta.head; m != nullptr; m = m->next) {
            if (k >= m->start && k <= m->end && m->bloom->bloom_get(&k)) {
                char file_str[15];
                sprintf(file_str, "storage/%d", m->page);
                struct stat s;
                int fd = open(file_str, O_RDONLY);
                int status = fstat(fd, &s);
                char *map = (char*) mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
                unsigned idx = 0;
                while (idx < s.st_size) {
                    unsigned long long* tmp = (unsigned long long*)(map + idx);
                    if (*tmp == k) {
                        char *ret = new char[128];
                        memcpy(ret, map + (idx + 8), 128);
                        munmap(map, s.st_size);
                        close(fd);
                        return ret;
                    }
                    idx += 136;
                }
                munmap(map, s.st_size);
                close(fd);
            }
        }

        /* key in neither disk nor skiplist */
        return nullptr;
    }

    void scan(unsigned long long k1, unsigned long long k2)
    {
        /* searching */
        int level = 3;
        node* cur;
        cur = head;
        while (level) {
            if (cur->right[level] == nullptr || k1 < *(cur->right[level]->key)) {
                level--;
            } else {
                cur = cur->right[level];
            }
        }
        while (cur->right[0] != nullptr && k1 >= *(cur->right[0]->key)) {
            cur = cur->right[0];
        }

        if (cur->key == nullptr) {
            if (cur->right[0] == nullptr) {
                for (; k1 <= k2; k1++) {
                    memcpy(txtbuff + text_size, "\nEMPTY", 6);
                    text_size += 6;
                }
                return;
            }
            cur = cur->right[0];
        } else if (k1 > *(cur->key)) {
            cur = cur->right[0];
        }

        for (; k1 <= k2; k1++) {
            if (k1 == *(cur->key)) {
                txtbuff[text_size++] = '\n';
                memcpy(txtbuff + text_size, cur->value, 128);
                text_size += 128;
                cur = cur->right[0];
            } else {
                memcpy(txtbuff + text_size, "\nEMPTY", 6);
                text_size += 6;
            }
        }
    }

    void save_output()
    {
        FILE *fp = fopen(out_file, "w");
        fwrite(txtbuff + 1, 1, text_size - 1, fp);
        fclose(fp);
        text_size = 0;
    }

    void show()
    {
        if (head->right[0] == nullptr) {
            puts("null");
            return;
        }
        node* cursor = head->right[0];
        printf("key num: %d, page num: %d\n", key_num, page_num);
        while(cursor != nullptr) {
            printf ("%20llu %s\n", *(cursor->key), cursor->value);
            cursor = cursor->right[0];
        }
    }
};
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
#include <pthread.h>
#include "bloom.h"

#define MAXKEYS 100000
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

struct child_data {
    char **line;
    unsigned long long start, end;
    int page;
};

void *scan_child(void *args)
{
    child_data* cd = (child_data*) args;
    char file_str[15];
    sprintf(file_str, "storage/%d", cd->page);
    struct stat s;
    int fd = open(file_str, O_RDONLY);
    int status = fstat(fd, &s);
    int key_n = 0;
    char *map = (char*) mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
    unsigned idx = 0;
    while (key_n < MAXKEYS) {
        unsigned long long* tmp = (unsigned long long*)(map + idx);
        if (cd->start <= *tmp ) {
            break;
        }
        idx += 136;
        key_n++;
    }

    while (key_n < MAXKEYS) {
        unsigned long long* tmp = (unsigned long long*)(map + idx);
        if (*tmp > cd->end) {
            break;
            
        }
        cd->line[*tmp - cd->start] = new char[128];
        memcpy(cd->line[*tmp - cd->start], map + (idx + 8), 128);
        idx += 136;
        key_n++;
    }
    munmap(map, s.st_size);
    close(fd);
    pthread_exit(NULL);
}

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
    int key_num, page_num;
    size_t text_size;

    DataBase()
    {
        srand(time(NULL));
        bloom = new bloomfilter;
        key_num = 0;  // key number in skiplist
        page_num = 1; // page index of current skiplist

        /* create skip list */
        head = new node;
        head->key = nullptr;
        for (int i = 0; i < 4; i++) {
            head->right[i] = nullptr;
        }
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
        char* new_v = new char[128];
        memcpy(new_v, v, 128);
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
                char *ret = new char[128];
                memcpy(ret, cur->value, 128);
                return ret;
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

    char** scan2(unsigned long long k1, unsigned long long k2)
    {
        char **line = new char*[k2 - k1 + 1];
        for (int i = 0; i <= k2 - k1; i++) {
            line[i] = nullptr;
        }
        /* multithread to scan from disk */
        child_data cdata[page_num];
        pthread_t thread[page_num];
        meta* p = Meta.head;
        for (int i = 0; p != nullptr; p = p->next, i++) {
            if (k1 > p->end || k2 < p->start) {
                // not in this page
                cdata[i].page = 0; 
                continue;
            }
            cdata[i].line = line;
            cdata[i].start = k1;
            cdata[i].end = k2;
            cdata[i].page = p->page;
            pthread_create(&thread[i], NULL, scan_child, &cdata[i]);
        }

        /* scan from skiplist */
        
        /* empty list */
        if (head->right[0] == nullptr || *(head->right[0]->key) > k2) {
            for (int i = 0; i < page_num - 1; i++) {
                if (cdata[i].page)
                    pthread_join(thread[i], NULL);
            }
            return line;
        }
        //show();
        int level = 3;
        node* cur = head;
        while(level) {
            if (cur->right[level] == nullptr || k1 < *(cur->right[level]->key)) {
                level--;
            } else {
                cur = cur->right[level];
            }
        }
        while (cur->right[0] != nullptr && k1 >= *(cur->right[0]->key))
            cur = cur->right[0];
        //printf("%llu, %llu\n", *(cur->key), k1);
        
        if (!cur->key) {
            cur = cur->right[0];
        }
        if (k1 <= *(cur->key)) {
            while (cur != nullptr && *(cur->key) <= k2) {
                char *val = new char[128];
                memcpy(val, cur->value, 128);
                line[*(cur->key) - k1] = val;
                cur = cur->right[0];
            }
        }

        for (int i = 0; i < page_num - 1; i++) {
            if (cdata[i].page)
                pthread_join(thread[i], NULL);
        }
        return line;
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
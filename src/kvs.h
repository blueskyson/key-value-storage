#include <time.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>     //open
#include <sys/mman.h>  //mmap
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXTEXT 1000000
#define FULLTEXT 999800
const char ENDCHAR = (char) 0x0a;

class DataBase {
public:
    struct node {
        unsigned long long *key;
        char *value;
        node *right[4];
    };
    struct meta {
        unsigned long long start, end;
        meta* next;
    };

    node *head;
    meta *meta_head;
    char* in_file, *out_file, *txtbuff;
    int out_fd;
    size_t text_size;

    DataBase(char* in_name)
    {
        srand(time(NULL));

        /* create skip list */
        meta_head = nullptr;
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

    void put(unsigned long long k, char* v)
    {
        /* searching */
        int level = 3;
        node* cursor[4];
        cursor[3] = head;

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
        
        /* if the same key, then replace the value */
        if (cursor[0]->key && *(cursor[0]->key) == k) {
            strcpy(cursor[0]->value, v);
            return;
        }

        unsigned long long *new_k = new unsigned long long(k);
        char* new_v = strdup(v);

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

    void get(unsigned long long k)
    {
        //printf("search for: %llu\n", k);
        //show();
        /* searching */
        int level = 3;
        node* cur = head;
        while (level) {
            if (cur->right[level] == nullptr || k < *(cur->right[level]->key)) {
                level--;
            } else {
                cur = cur->right[level];
                //printf("%d %llu\n", level, *(cur->key));
            }
        }
        while (cur->right[0] != nullptr && k >= *(cur->right[0]->key)) {
            cur = cur->right[0];
            //printf("%d %llu\n", level, *(cur->key));
        }

        if (cur->key == nullptr || *(cur->key) != k) {
            memcpy(txtbuff + text_size, "\nEMPTY", 6);
            text_size += 6;
        } else {
            txtbuff[text_size++] = '\n';
            memcpy(txtbuff + text_size, cur->value, 128);
            text_size += 128;
        }
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

    void show()
    {
        node* cursor = head->right[0];
        while(cursor != nullptr) {
            printf ("%20llu  X", *(cursor->key));
            if (cursor->right[1])
                printf (" X");
            if (cursor->right[2])
                printf (" X");
            if (cursor->right[3])
                printf (" X");
            cursor = cursor->right[0];
            puts("");
        }
        puts("");
    }

    void save() {
        FILE *fp = fopen(out_file, "w");
        fwrite(txtbuff + 1, 1, text_size - 1, fp);
        fclose(fp);
    }

    void mem_to_disk() {
        node* p = head->right[0];
        if (!p) {
            return;    
        }

        meta *m;
        if (meta_head == nullptr) {
            meta_head = new meta;
            meta_head->next = nullptr;
            m = meta_head;
        } else {
            for (m = meta_head; m->next != nullptr; m = m->next);
            m->next = new meta;
            m = m->next;
            m->next = nullptr;
        }

        m->start = *(p->key);
        FILE *fp = fopen("storage/1", "wb");
        for (; p->right[0]; p = p->right[0]) {
            fwrite(p->key, 8, 1, fp);
            fwrite(p->value, 1, 128, fp);
        }
        m->end = *(p->key);
        fclose(fp);
        
    }

    void meta_to_disk() {
        meta* p = meta_head;
        if (!p) {
            return;    
        }
        FILE *fp = fopen("storage/0", "wb");
        for (;p != nullptr; p = p->next) {
            fwrite(&(p->start), 8, 1, fp);
            fwrite(&(p->end), 8, 1, fp);
        }
        fclose(fp);
    }

    void get_from_disk(unsigned long long k) {
        
    }
};
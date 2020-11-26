#include <time.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>     //open
#include <sys/mman.h>  //mmap
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXTEXT 1000000
#define FULLTEXE 999800
const char ENDCHAR = (char) 0x0a;

class DataBase {
public:
    struct node {
        unsigned long long *key;
        char *value;
        node *right;
        node *down;
    };
    short level_num;
    node **head;
    char* in_file, *out_file, *txtbuff;
    int out_fd;
    size_t text_size;

    DataBase(int l, char* in_name)
    {
        srand(time(NULL));

        /* create skip list */
        level_num = l;
        head = new node*[4];
        head[0] = new node;
        head[0]->key = nullptr;
        head[0]->right = nullptr;
        head[0]->down = nullptr;
        for (int i = 1; i < level_num; i++) {
            head[i] = new node;
            head[i]->key = nullptr;
            head[i]->right = nullptr;
            head[i]->down = head[i - 1];
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
        int level = level_num - 1;
        node* cursor[level_num];
        cursor[level] = head[level];
        while(level) {
            if (cursor[level]->right == nullptr || k < *(cursor[level]->right->key)) {
                cursor[level-1] = cursor[level]->down;
                level--;
            } else {
                cursor[level] = cursor[level]->right;
            }
        }
        while (cursor[0]->right != nullptr && k >= *(cursor[level]->right->key))
            cursor[0] = cursor[0]->right;
        
        /* if the same key, then replace the value */
        if (cursor[0]->key && *(cursor[0]->key) == k) {
            strcpy(cursor[0]->value, v);
            return;
        }

        unsigned long long *new_k = new unsigned long long(k);
        char* new_v = strdup(v);

        /* level 0 */
        node* new_node0 = new node;
        new_node0->key = new_k;
        new_node0->value = new_v;
        new_node0->down = nullptr;
        new_node0->right = cursor[0]->right;
        cursor[0]->right = new_node0;
        cursor[0] = new_node0;
         
        /* other levels */
        short cnt = 1;
        while (cnt < level_num && (rand() & 1)) {
            node* new_node = new node;
            new_node->key = new_k;
            new_node->value = new_v;
            new_node->down = cursor[cnt - 1];
            new_node->right = cursor[cnt]->right;
            cursor[cnt]->right = new_node;
            cursor[cnt] = new_node;
            cnt++;
        }
    }

    void get(unsigned long long k)
    {
        //printf("search for: %llu\n", k);
        //show();
        /* searching */
        int level = level_num - 1;
        node* cur;
        cur = head[level];
        while (level) {
            if (cur->right == nullptr || k < *(cur->right->key)) {
                cur = cur->down;
                level--;
            } else {
                cur = cur->right;
                //printf("%d %llu\n", level, *(cur->key));
            }
        }
        while (cur->right != nullptr && k >= *(cur->right->key)) {
            cur = cur->right;
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
        int level = level_num - 1;
        node* cur;
        cur = head[level];
        while (level) {
            if (cur->right == nullptr || k1 < *(cur->right->key)) {
                cur = cur->down;
                level--;
            } else {
                cur = cur->right;
            }
        }
        while (cur->right != nullptr && k1 >= *(cur->right->key)) {
            cur = cur->right;
        }

        if (cur->key == nullptr) {
            if (cur->right == nullptr) {
                for (; k1 <= k2; k1++) {
                    memcpy(txtbuff + text_size, "\nEMPTY", 6);
                    text_size += 6;
                }
                return;
            }
            cur = cur->right;
        } else if (k1 > *(cur->key)) {
            cur = cur->right;
        }

        for (; k1 <= k2; k1++) {
            if (k1 == *(cur->key)) {
                txtbuff[text_size++] = '\n';
                memcpy(txtbuff + text_size, cur->value, 128);
                text_size += 128;
                cur = cur->right;
            } else {
                memcpy(txtbuff + text_size, "\nEMPTY", 6);
                text_size += 6;
            }
        }
    }

    void show()
    {
        node* cursor[level_num];
        for (int i = 0; i < level_num; i++) {
            cursor[i] = head[i]->right;
        }
        while(cursor[0] != nullptr) {
            printf ("%20llu  X ", *(cursor[0]->key));
            for (int i = 1; i < level_num; i++) {
                if (cursor[i] && *(cursor[i]->key) == *(cursor[0]->key)) {
                    putchar('X');
                    putchar(' ');
                    cursor[i] = cursor[i]->right;
                }
            }
            cursor[0] = cursor[0]->right;
            puts("");
        }
        puts("");
    }

    void save() {
        int fd = open(out_file, O_RDWR | O_CREAT, (mode_t)0600);
        lseek(fd, text_size - 1, SEEK_SET);
        write(fd, &ENDCHAR, 1);
        char *map = (char*) mmap(0, text_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        memcpy(map, txtbuff + 1, text_size - 1);
        msync(map, text_size, MS_SYNC);
        munmap(map, text_size);
        close(fd);
    }

    void save2() {
        FILE *fp = fopen(out_file, "w");
        fwrite(txtbuff + 1, 1, text_size - 1, fp);
        fclose(fp);
    }
};
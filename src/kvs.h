#include <time.h>
#include <stdlib.h>
#include <string.h>
#define random() (rand() & 1)

class SkipList {
public:
    struct node {
        unsigned long long *key;
        char *value;
        node *right;
        node *down;
    };
    short level_num;
    node **head;

    SkipList(int l = 4)
    {
        puts("haha");
        level_num = l;
        head = new node*[4];
        head[0] = new node;
        head[0]->right = nullptr;
        head[0]->down = nullptr;
        for (int i = 1; i < level_num; i++) {
            head[i] = new node;
            head[i]->right = nullptr;
            head[i]->down = head[i - 1];
        }
        
        srand(time(NULL));
    }
    
    int randval();
    void put(unsigned long long k, char* v) {
        /* searching */
        int level = level_num - 1;
        node* cursor[level_num];
        cursor[level] = head[level];
        while(level) {
            printf("%d ", level);
            if (cursor[level]->right == nullptr || k < *(cursor[level]->right->key)) {
                cursor[level-1] = cursor[level]->down;
            } else {
                cursor[level] = cursor[level]->right;
            }
            level--;
        }

        // /* if the same key, then replace the value */
        // if (*(cursor[0]->key) == k) {
        //     strcpy(cursor[0]->value, v);
        //     return;
        // }

        // unsigned long long *new_k = new unsigned long long(k);
        // char* new_v = strdup(v);

        // /* level 0 */
        // {
        //     node* new_node = new node;
        //     new_node->key = new_k;
        //     new_node->value = new_v;
        //     new_node->down = nullptr;
        //     new_node->right = cursor[0]->right;
        //     cursor[0]->right = new_node;
        //     cursor[0] = new_node;
        // } 
        
        // /* other levels */
        // short cnt = 1;
        // while (randval() && cnt < level_num) {
        //     node* new_node = new node;
        //     new_node->key = new_k;
        //     new_node->value = new_v;
        //     new_node->down = cursor[cnt - 1];
        //     new_node->right = cursor[cnt]->right;
        //     cursor[cnt]->right = new_node;
        //     cursor[cnt] = new_node;
        // }
    }

    char* get(unsigned long long k);
    char** scan(unsigned long long k1, unsigned long long k2);
    void show() {

    }
};

int SkipList::randval()
{
    return rand() & 1;
}
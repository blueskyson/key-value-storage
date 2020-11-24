#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define random() (rand() & 1)

class SkipList {
public:
    struct node {
        uint64_t *key;
        char *value;
        node *right;
        node *down;
    };
    short level_num;
    node **head;

    SkipList(int l = 4)
    {
        level_num = l;
        head = new node*[4];
        for (int i = level_num - 1; i > 0; i--) {
            head[i] = new node;
            head[i]->right = nullptr;
            head[i]->down = head[i - 1];
        }
        head[0] = new node;
        head[0]->right = nullptr;
        head[0]->down = nullptr;
        srand(time(NULL));
    }
    
    int randval();
    void put(uint64_t k, char* v) {
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

        /* if the same key, then replace the value */
        if (*(cursor[0]->key) == k) {
            strcpy(cursor[0]->value, v);
            return;
        }

        uint64_t *new_k = new uint64_t(k);
        char* new_v = strdup(v);

        /* level 0 */
        {
            node* new_node = new node;
            new_node->key = new_k;
            new_node->value = new_v;
            new_node->down = nullptr;
            new_node->right = cursor[0]->right;
            cursor[0]->right = new_node;
            cursor[0] = new_node;
        } 
        
        /* other levels */
        short cnt = 1;
        while (randval() && cnt < level_num) {
            node* new_node = new node;
            new_node->key = new_k;
            new_node->value = new_v;
            new_node->down = cursor[cnt - 1];
            new_node->right = cursor[cnt]->right;
            cursor[cnt]->right = new_node;
            cursor[cnt] = new_node;
        }
    }

    char* get(uint64_t k);
    char** scan(uint64_t k1, uint64_t k2);
    void show() {

    }
};

int SkipList::randval()
{
    return rand() & 1;
}
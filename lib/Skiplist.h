typedef struct skiplist Skiplist;
typedef struct skiplist_node SkiplistNode;
typedef SkiplistNode* (*skiplist_find_func_t)(Skiplist *self, unsigned long long key);
typedef int (*skiplist_insert_func_t)(Skiplist *self, unsigned long long key, char *value);

int init_skiplist(Skiplist **self);
void destruct_skiplist(Skiplist *self);

struct skiplist_node {
    unsigned long long *key;
    char *value;
    SkiplistNode *right[4];
};

struct skiplist {
    SkiplistNode *head;
    unsigned int rand_mask[4];
    skiplist_find_func_t find;
    skiplist_insert_func_t insert;
};
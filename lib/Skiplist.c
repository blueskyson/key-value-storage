#include <stdlib.h>
#include <string.h>
#include "Skiplist.h"

SkiplistNode* skiplist_find_impl(Skiplist* self, unsigned long long key)
{
    int level = 3;
    SkiplistNode* cursor[4];
    cursor[3] = self->head;
    while(level) {
        if (cursor[level]->right[level] == NULL || key < *(cursor[level]->right[level]->key)) {
            cursor[level-1] = cursor[level];
            level--;
        } else {
            cursor[level] = cursor[level]->right[level];
        }
    }

    while (cursor[0]->right[0] != NULL && key >= *(cursor[0]->right[0]->key))
        cursor[0] = cursor[0]->right[0];
    
    /* the key is actually in skiplist */
    if (cursor[0]->key && *(cursor[0]->key) == key) {
        return cursor[0];
    }

    return NULL;
}

int skiplist_insert_impl(Skiplist* self, unsigned long long key, char *value) {
    int level = 3;
    SkiplistNode* cursor[4];
    cursor[3] = self->head;
    while (level) {
        if (cursor[level]->right[level] == NULL || key < *(cursor[level]->right[level]->key)) {
            cursor[level-1] = cursor[level];
            level--;
        } else {
            cursor[level] = cursor[level]->right[level];
        }
    }
    while (cursor[0]->right[0] != NULL && key > *(cursor[0]->right[0]->key))
        cursor[0] = cursor[0]->right[0];

    SkiplistNode *newNode = malloc(sizeof(SkiplistNode));
    if (newNode == NULL)
        return -1;
    newNode->key = malloc(sizeof(unsigned long long));
    newNode->value = malloc(128 * sizeof(char));
    *(newNode->key) = key;
    memcpy(newNode->value, value, 128);
    newNode->right[0] = cursor[0]->right[0];
    cursor[0]->right[0] = newNode;
    cursor[0] = newNode;
    newNode->right[1] = NULL;
    newNode->right[2] = NULL;
    newNode->right[3] = NULL;
    short cnt = 1;
    while (cnt < 4 && !(rand() & self->rand_mask[cnt])) {
        newNode->right[cnt] = cursor[cnt]->right[cnt];
        cursor[cnt]->right[cnt] = newNode;
        cnt++;
    }
}

int init_skiplist(Skiplist **self)
{
    if (NULL == (*self = malloc(sizeof(Skiplist))))
        return -1;

    (*self)->head = malloc(sizeof(SkiplistNode));
    if ((*self)->head == NULL) {
        return -1;
    }

    (*self)->head->key = NULL;
    for (int i = 0; i < 4; i++) {
        (*self)->head->right[i] = NULL;
    }

    (*self)->find = skiplist_find_impl;
    (*self)->insert = skiplist_insert_impl;
    (*self)->rand_mask[0] = 0;
    (*self)->rand_mask[1] = 15;
    (*self)->rand_mask[2] = 7;
    (*self)->rand_mask[3] = 15;
    return 0;
}
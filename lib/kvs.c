#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kvs.h"
#define MAXKEYS 100000

void save_latest_page(Kvs *self);
void save_metadata(Kvs *self);

void logErr(char *msg)
{
    fprintf(stderr, "Error: %s\n", msg);
}

int put_impl(Kvs *self, unsigned long long key, char *value)
{
    if (self->bloomfilter->get(self->bloomfilter, key))
    {
        SkiplistNode *node = self->skiplist->find(self->skiplist, key);
        if (node != NULL)
        {
            memcpy(node->value, value, 128);
            return 0;
        }
    }

    self->skiplist->insert(self->skiplist, key, value);
    self->bloomfilter->add(self->bloomfilter, key);
    self->keyCount++;

    /* Create a new page when current page is full */
    if (self->keyCount == MAXKEYS) {
        save_latest_page(self);
    }
}

char *get_impl(Kvs *self, unsigned long long key)
{
    char *value = malloc(129 * sizeof(char));

    /* check if the key is in skiplist */
    if (self->bloomfilter->get(self->bloomfilter, key))
    {
        SkiplistNode *node = self->skiplist->find(self->skiplist, key);
        if (node != NULL)
        {
            memcpy(value, node->value, 128);
            value[128] = '\0';
            return value;
        }
    }

    /* check if the key is in disk */
    for (MetadataNode* m = self->metadataManager->head; m != NULL; m = m->next) {
        if (key >= m->start && key <= m->end && m->bloomfilter->get(m->bloomfilter, key)) {
            char file_str[15];
            sprintf(file_str, "storage/%d", m->pageNum);
            struct stat s;
            int fd = open(file_str, O_RDONLY);
            int status = fstat(fd, &s);
            char *map = (char*) mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
            unsigned idx = 0;
            
            /* binary search */
            int low = 0, high = MAXKEYS - 1;
            while (low <= high) {
                int mid = (low + high) / 2;
                int idx = (mid << 7) + (mid << 3); // mid * 136
                unsigned long long* tmp = (unsigned long long*)(map + idx);
                if (*tmp < key) {
                    low = mid + 1;
                } else if (*tmp > key) {
                    high = mid - 1;
                } else {
                    memcpy(value, map + (idx + 8), 128);
                    munmap(map, s.st_size);
                    close(fd);
                    return value;
                }
            }
            munmap(map, s.st_size);
            close(fd);
        }
    }

    memcpy(value, "EMPTY", 6);
    return value;
}

void save_latest_page(Kvs *self)
{
    SkiplistNode *p = self->skiplist->head->right[0];
    if (p == NULL) {
        return;
    }

    MetadataNode *m = malloc(sizeof(MetadataNode));
    if (m == NULL)
        return;

    m->bloomfilter = self->bloomfilter;
    self->bloomfilter = malloc(sizeof(Bloomfilter));
    if (self->bloomfilter == NULL || init_bloomfilter(&(self->bloomfilter)) == -1)
        return;

    m->start = *(p->key);
    char fname[15];
    sprintf(fname, "storage/%d", self->pageCount);
    FILE *fp = fopen(fname, "wb");
    for (; p->right[0] != NULL;)
    {
        fwrite(p->key, 8, 1, fp);
        fwrite(p->value, 128, 1, fp);
        SkiplistNode *del = p;
        p = p->right[0];
        free(del->key);
        free(del->value);
        free(del);
    }
    fwrite(p->key, 8, 1, fp);
    fwrite(p->value, 128, 1, fp);
    m->end = *(p->key);
    m->keyCount = self->keyCount;
    m->pageNum = self->pageCount;
    m->next = NULL;
    if (self->metadataManager->tail != NULL)
    {
        self->metadataManager->tail->next = m;
        self->metadataManager->tail = m;
    }
    else
    {
        self->metadataManager->head = m;
        self->metadataManager->tail = m;
    }
    free(p->key);
    free(p->value);
    free(p);

    self->skiplist->head->right[0] = NULL;
    self->skiplist->head->right[1] = NULL;
    self->skiplist->head->right[2] = NULL;
    self->skiplist->head->right[3] = NULL;
    self->keyCount = 0;
    self->pageCount++;
    fclose(fp);
}

void save_metadata(Kvs *self)
{
    MetadataNode *p = self->metadataManager->head;
    if (!p)
        return;
    FILE *fp = fopen("storage/metadata", "wb");
    for (; p != NULL; p = p->next)
    {
        fwrite(&(p->start), 8, 1, fp);
        fwrite(&(p->end), 8, 1, fp);
        fwrite(&(p->keyCount), 4, 1, fp);
        fwrite(p->bloomfilter->table, p->bloomfilter->size, 1, fp);
    }
    fclose(fp);
}

void save_impl(Kvs *self)
{
    save_latest_page(self);
    save_metadata(self);
}

int init_kvs(Kvs **self)
{
    /* create storage folder */
    struct stat dir;
    if (stat("storage", &dir) != 0) {
        if (mkdir("storage", 0775) == -1) {
            logErr("fail to create storage directory");
            return -1;
        }
    }

    if (NULL == (*self = malloc(sizeof(Kvs))))
        return -1;

    (*self)->metadataManager = malloc(sizeof(MetadataManager));
    if ((*self)->metadataManager == NULL || init_metadata_manager(&((*self)->metadataManager)) == -1)
        return -1;

    (*self)->bloomfilter = malloc(sizeof(Bloomfilter));
    if ((*self)->bloomfilter == NULL || init_bloomfilter(&((*self)->bloomfilter)) == -1)
        return -1;

    (*self)->skiplist = malloc(sizeof(Skiplist));
    if ((*self)->skiplist == NULL || init_skiplist(&((*self)->skiplist)) == -1)
        return -1;

    (*self)->put = put_impl;
    (*self)->get = get_impl;
    (*self)->save_to_disk = save_impl;
    (*self)->pageCount = 0;
    (*self)->keyCount = 0;

    /* load metadata */
    if (access("storage/metadata", F_OK) != 0) {
        return 0;
    }
    MetadataNode *cur = (*self)->metadataManager->head, *prev_tail;
    struct stat s;
    int fd = open("storage/metadata", O_RDONLY);
    int status = fstat(fd, &s);
    char *map = (char *)mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
    unsigned long long idx = 0;
    while (idx < s.st_size) {
        MetadataNode *m = malloc(sizeof(MetadataNode));
        if (m == NULL)
            return -1;
        m->bloomfilter = malloc(sizeof(Bloomfilter));
        if (m->bloomfilter == NULL || init_bloomfilter(&(m->bloomfilter)) == -1)
            return -1;
        memcpy(&(m->start), (map + idx), 8);
        memcpy(&(m->end), (map + idx + 8), 8);
        memcpy(&(m->keyCount), (map + idx + 16), 4);
        memcpy(m->bloomfilter->table, (map + idx + 20), m->bloomfilter->size);
        m->next = NULL;
        m->pageNum = (*self)->pageCount++;
        if ((*self)->metadataManager->head == NULL)
        {
            (*self)->metadataManager->head = m;
            cur = m;
        }
        else
        {
            cur->next = m;
            cur = cur->next;
        }
        prev_tail = (*self)->metadataManager->tail;
        (*self)->metadataManager->tail = cur;
        idx += (m->bloomfilter->size + 20); // Metadata Size: 131072 + 8 + 8 + 4
    }

    /* last page is full */
    if ((*self)->metadataManager->tail == NULL || (*self)->metadataManager->tail->keyCount == MAXKEYS) {
        return 0;
    }

    /* remove last page from metadata and store it in skiplist */
    MetadataNode* m = (*self)->metadataManager->tail;
    (*self)->metadataManager->tail = prev_tail;
    if ((*self)->metadataManager->tail != NULL)
        (*self)->metadataManager->tail->next = NULL;
    else
        (*self)->metadataManager->head = NULL;

    char fname[15];
    SkiplistNode *cursor[4];
    cursor[3] = (*self)->skiplist->head;
    cursor[2] = (*self)->skiplist->head;
    cursor[1] = (*self)->skiplist->head;
    cursor[0] = (*self)->skiplist->head;

    (*self)->keyCount = m->keyCount;
    (*self)->pageCount = m->pageNum;
    sprintf(fname, "storage/%d", (*self)->pageCount);
    FILE *fp = fopen(fname, "rb");
    for (int i = 0; i < (*self)->keyCount; i++) {
        /* level 0 */
        SkiplistNode *newNode = malloc(sizeof(SkiplistNode));
        newNode->key = malloc(sizeof(unsigned long long));
        newNode->value = malloc(128 * sizeof(char));
        fread(newNode->key, 8, 1, fp);
        fread(newNode->value, 128, 1, fp);
        cursor[0]->right[0] = newNode;
        cursor[0] = newNode;
        (*self)->bloomfilter->add((*self)->bloomfilter, *(newNode->key));

        /* other levels */
        newNode->right[0] = NULL;
        newNode->right[1] = NULL;
        newNode->right[2] = NULL;
        newNode->right[3] = NULL;
        short cnt = 1;
        while (cnt < 4 && !(rand() & (*self)->skiplist->rand_mask[cnt]))
        {
            cursor[cnt]->right[cnt] = newNode;
            cursor[cnt] = newNode;
            cnt++;
        }
    }
    fclose(fp);
    return 0;
};
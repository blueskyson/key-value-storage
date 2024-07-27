#include "Bloomfilter.h"

typedef struct metadata_node MetadataNode;
typedef struct metadata_manager MetadataManager;

int init_metadata_manager(MetadataManager **self);
void destruct_metadata_manager(MetadataManager *self);

struct metadata_node {
    unsigned long long start, end;
    int keyCount, pageNum;
    Bloomfilter* bloomfilter;
    MetadataNode* next;
};

struct metadata_manager {
    MetadataNode *head, *tail;
    int pageCount;
};
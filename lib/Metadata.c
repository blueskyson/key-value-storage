#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Metadata.h"

int init_metadata_manager(MetadataManager **self)
{
    if (NULL == (*self = malloc(sizeof(MetadataManager))))
        return -1;

    (*self)->head = NULL;
    (*self)->tail = NULL;
    (*self)->pageCount = 0;
    return 0;
}
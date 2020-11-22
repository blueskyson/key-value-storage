#include <sys/stat.h>
#include <sys/mman.h>  //mmap
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>     //open
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>    //close
#include "bench.h"

void check(int test, const char* message, ...)
{
    if (test) {
        va_list args;
        va_start(args, message);
        vfprintf(stderr, message, args);
        va_end(args);
        fprintf(stderr, "\n");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    int fd;
    struct stat s;
    int status;
    size_t size;
    const char* file_name = "../input.txt";
    const char* mapped;
    int i;
    double start, end;

    start = tvgetf();
    fd = open(file_name, O_RDONLY);
    //check(fd < 0, "open %s failed: %s", file_name, strerror(errno));

    status = fstat(fd, &s);
    //check(status < 0, "stat %s failed: %s", file_name, strerror(errno));
    size = s.st_size;

    mapped = (const char*) mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    //check(mapped == MAP_FAILED, "mmap %s failed: %s", file_name, strerror(errno));
    end = tvgetf();
    printf("\ntime: %lf\n", end - start);
    close(fd);
    while(1);
    return 0;
}
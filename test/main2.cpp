#include <stdio.h>
#include <string.h>
#include "bench.h"
#define MAXCHAR 128

int main() {
    const char* file_name = "../input.txt";
    char buffer[MAXCHAR];
    char **line = new char*[100];
    int linenum = 0;
    double start, end;

    start = tvgetf();
    FILE* fp = fopen(file_name, "r");
    while (fgets(buffer, MAXCHAR, fp) != NULL) {
        //puts(buffer);
    }
    end = tvgetf();
    printf("\ntime: %lf\n", end - start);
    fclose(fp);
    while(1);
    return 0;
}
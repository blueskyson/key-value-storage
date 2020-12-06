#include "bloom.h"
#include <string>
using namespace std;
int main(int argc, char *argv[]) {
    FILE *fp = fopen(argv[1], "wb");
    unsigned long long lines = fast_atoull(argv[2]);
    //srand(1);
    //int t = rand();
    unsigned long long offset = (unsigned long long)0;
    char first_val[129];
    first_val[128] = '\0';
    memset(first_val, '-', 128);
    memcpy(first_val, to_string(offset).c_str(), to_string(offset).size());
    string first_line = "PUT " + to_string(offset) + " " + string(first_val);
    fwrite(first_line.c_str(), first_line.size(), 1, fp);
    for (unsigned long long k = offset + 1; k < lines + offset; k++) {
        char v[129];
        memset(v, '-', 128);
        v[128] = '\0';
        memcpy(v, to_string(k).c_str(), to_string(k).size());
        string l = "\nPUT " + to_string(k) + " " + string(v);
        fwrite(l.c_str(), l.size(), 1, fp);
    }
    fclose(fp);
    return 0;
}
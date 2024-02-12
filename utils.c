#include "utils.h"

void* _ma(size_t size) {
    void* p = NULL; do p = malloc(size); while (!p);
    return p;
}
void* _ca(size_t size) {
    void* p = NULL; do p = calloc(size, 1); while (!p);
    return p;
}
void* _re(void* p, size_t size) {
    void* o = NULL; do o = realloc(p, size); while (!o);
    return o;
}



char** SplitString(char* string, char* p) {
    int count = 4, k = 0, len = strlen(string);

    char a[len], * b, ** A = _ma(count * _s(char*));
    strcpy(a, string);

    while (strstr(a, p) != NULL) {
        b = strstr(a, p);
        int l = b - a;
        if (l > 0) A[k++] = memcpy(_ca(l + 1), a, l);
        strcpy(a, b + 1);

        if (k == count) A = _re(A, (count *= 2) * _s(char*));
    }

    A[k++] = strcpy(_ca(strlen(a) + 1), a);
    A[k++] = NULL;
    return (char**)_re(A, k * _s(char*));
}
uint8_t ReadBit(uint8_t x, uint8_t bit) { return (x & (1 << bit)) >> bit; }


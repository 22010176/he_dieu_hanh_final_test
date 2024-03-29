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

    char a[len], * b, ** A = _ca(count * sizeof(char*));
    strcpy(a, string);

    while (strstr(a, p) != NULL) {
        b = strstr(a, p);
        int l = b - a;
        if (l > 0) A[k++] = memcpy(_ca(l + 1), a, l);
        strcpy(a, b + 1);
        if (k + 2 == count) A = _re(A, (count *= 2) * sizeof(char*));
    }

    if (strlen(a) > 0) A[k++] = strcpy(_ca(strlen(a) + 1), a);
    A[k++] = NULL;

    return (char**)_re(A, k * sizeof(char*));
}
char** SplitStringExceptLast(char* string, char* p) {
    int count = 4, k = 0, len = strlen(string);

    char a[len], * b, ** A = _ca(count * sizeof(char*));
    strcpy(a, string);

    while (strstr(a, p) != NULL) {
        b = strstr(a, p);
        int l = b - a;
        if (l > 0) A[k++] = memcpy(_ca(l + 1), a, l);
        strcpy(a, b + 1);
        if (k + 2 == count) A = _re(A, (count *= 2) * sizeof(char*));
    }

    // if (strlen(a) > 0) A[k++] = strcpy(_ca(strlen(a) + 1), a);
    A[k++] = NULL;

    return (char**)_re(A, k * sizeof(char*));
}
uint32_t SizeStringArr(char** string) {
    uint32_t size = 0;
    for (int i = 0; string[i] != NULL;++i) size += strlen(string[i]) + 1;
    return size;
}
char* JoinString(char* result, char** string, char* add) {
    for (int i = 0; string[i] != NULL; ++i) {
        strcat(result, string[i]);
        strcat(result, add);
    }
    return result;
}
char* JoinStringExceptLast(char* result, char** string, char* add) {
    for (int i = 0; string[i + 1] != NULL; ++i) {
        strcat(result, string[i]);
        strcat(result, add);
    }
    return result;
}
char* GetFileName(char** path) {
    for (int i = 0; path[i] != NULL; ++i) if (path[i + 1] == NULL)
        return strcpy(_ca(strlen(path[i]) + 1), path[i]);
    return "";
}
uint8_t ReadBit(uint8_t x, uint8_t bit) { return (x & (1 << bit)) >> bit; }
void _Print(char* address, size_t size) {
    if (size == 0) return;
    printf("\n");
    for (int i = 0; i < size;++i) {
        for (int j = 0; j < 8; printf("%d", ReadBit(address[i], j++)));
        printf((i + 1) % 8 != 0 ? " " : "\n");
    }
    printf("\n");
}
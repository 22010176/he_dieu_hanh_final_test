#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
// #include <stdbool.h>

#define _s fflush(stdin); scanf
#define _rand(x) rand() % x + 1
#define _randRange(start, end, step) _rand((int)((end - start) / step)) * step + start
#define _for(start, end, step) for (uint32_t i = start; i < end; i += step)
#define FreeM(x) for (int i = 0; x[i] != NULL; free(x[i++])); free(x)

// char             1 byte              8   bit     (*)
// int              4 byte              32  bit
// long long        8 byte              64  bit
// float            4 byte              32  bit
// double           8 byte              64  bit
// unsigned         no negative
unsigned char* data = NULL; // 4KB * 16 (1 line) * 4 = 256KB

// Name                 Chunk
// Super                0       
// Inodes Bitmap        1
// Data Bitmap          2           
// Inodes               3 - 7
// Data                 8 - 63

// Setup parameter              
// Use store and read it in Super later
const unsigned int diskSize = 4 * 1024 * 16 * 4;            // byte (256KB)
const unsigned int chunkSize = 4096;                        // byte (4KB)
const unsigned int blockSize = 256;                           // byte
const unsigned int inodeStartAddress = (3 * chunkSize) / blockSize;
const unsigned int inodeNumber = 80;
const unsigned int rootInode = 0;

typedef struct Inode Inode;
struct Inode {
    unsigned int inode_number;                           // Inode's number        
    unsigned int link;                                   // Count links
    unsigned int blocks[15];                             // Data's address (12 direct pointer - 3 indirect)
    // 4 + 4 + 4 * 15 = 68 byte

    // Need to fill up 256 byte => Add more 144 byte (256 - 68) = 188
    unsigned char others[188];                           // for storing addition stuff;
};

void* _m(size_t size);
void* _r(void* p, size_t size);

char** SplitString(char* string, char* p);

int MappingAddress(int address);
int GetAddressFromChunk(int chunk);
int CalcMemorySize(double size);

int CheckFreeChunk(int chunk, int number);
int GetFreeChunk(int address, int size);
void FreeChunk(int chunk, int number);

int Mkdir(char* path);

void WriteData(int address, char* dat, int size);

unsigned int ReadBit(char x, int bit);
char* ReadData(void* dest, int address, int size);
void PrintData(char* data, int size);

void ReadWriteTest() {
    int ss = sizeof(int);
    int size = diskSize / ss, dd[size];
    for (int i = 0; i < size; dd[i++] = rand() * rand() * rand());

    WriteData(GetAddressFromChunk(0), (char*)&dd, size * ss);

    char d[CalcMemorySize(size * ss)];
    ReadData(d, GetAddressFromChunk(0), size * ss);

    int* a = (int*)d;
    for (int i = 0; i < size; i++) {
        if (a[i] == dd[i]) continue;
        printf("f: %10lld  %10lld\n", a[i], dd[i]);
    }
}

void Initialize() {
    srand(time(NULL));
    data = _m(diskSize);



}

void CleanUp() {
    free(data);
}

int main() {
    Initialize();


    CleanUp();

    return 0;
}
void* _m(size_t size) {
    void* p = NULL;
    do p = calloc(size, 1); while (!p);
    return p;
}

void* _r(void* p, size_t size) {
    void* o = NULL;
    do o = realloc(p, size); while (!o);

    return o;
}
char** SplitString(char* string, char* p) {
    int count = 4, k = 0, len = strlen(string);
    char a[len], * b;
    strcpy(a, string);
    char** A = _m(count * sizeof(void*));

    while (strstr(a, p) != NULL) {
        b = strstr(a, p);
        int l = b - a;
        A[k++] = memcpy(_m(l + 1), a, l);
        strcpy(a, b + 1);
        if (k == count) A = _r(A, (count *= 2) * sizeof(char*));
    }

    A[k++] = strcpy(_m(strlen(a)), a);
    A[k++] = NULL;
    return (char**)_r(A, k * sizeof(void*));
}

int MappingAddress(int address) { return address * blockSize; }
int GetAddressFromChunk(int chunk) { return (chunk * chunkSize) / blockSize; }

int CreateDirectory(int InodeNumber, char** path) {

}
int Mkdir(char* path) {
    char** p = SplitString(path, "p");
    for (int i = 0; p[i] != NULL; i++) {


    }

    FreeM(p);
    return 1;
}
int CalcMemorySize(double size) { return (int)ceil(size / blockSize) * blockSize; }

void WriteData(int address, char* dat, int size) {
    char* dst = &data[MappingAddress(address)];
    memcpy(dst, dat, CalcMemorySize(size));
}

unsigned int ReadBit(char x, int bit) { return (x & (1 << bit)) >> bit; }
char* ReadData(void* dest, int address, int size) { return memcpy(dest, data + MappingAddress(address), CalcMemorySize(size)); }
void PrintData(char* data, int size) {
    for (int i = 0, c = 0; i < CalcMemorySize(size);++i, ++c) {
        for (int j = 0; j < 8; printf("%d", ReadBit(data[i], ++j)));
        printf((c + 1) % 8 != 0 ? " " : "\n");
    }
}
int CheckFreeChunk(int chunk, int number) {
    char* a = &data[MappingAddress(GetAddressFromChunk(chunk))];
    return ReadBit(a[(int)(number / 8)], number % 8);
}

int GetFreeChunk(int chunk, int size) {
    char* a = &data[MappingAddress(GetAddressFromChunk(chunk))];
    char data = 1;
    for (int i = 0; i < (size / 8); ++i) {
        for (int j = 0; j < 8; ++j) {
            if (ReadBit(a[i], j)) continue;                             // If Bitmap is 1, continue
            a[i] |= (1 << j);                                           // Write to Bitmap
            return i + j;
        }
    }
    return -1;
}

void FreeChunk(int chunk, int number) {
    char* d = &data[MappingAddress(GetAddressFromChunk(chunk))];
    if (!ReadBit(d[(number / 8)], number % 8)) return;

    d[(number / 8)] -= pow(2, number % 8);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
// #include <stdbool.h>

#define _s                                              fflush(stdin); scanf
#define _rand(x)                                        rand() % x + 1
#define _randRange(start, end, step)                    _rand((int)((end - start) / step)) * step + start
#define FreeM(x)                                        for (int i = 0; x[i] != NULL; free(x[i++])); free(x)

#define DIRECTORY                                       1
#define FILE                                            2
// #define END_OF_ARRAY                                    UINT_MAX
// char             1 byte              8   bit     (*)
// int              4 byte              32  bit
// long long        8 byte              64  bit
// float            4 byte              32  bit
// double           8 byte              64  bit
// unsigned         no negative
unsigned char* data = NULL; // 4KB * 16 (1 line) * 4 = 256KB

typedef struct Bitmap Bitmap;
struct Bitmap { unsigned int size, chunk, address; };

typedef struct Inode Inode;
struct Inode {
    unsigned int inode_number;                              // Inode's number        
    unsigned int type;                                      // File type (1 = D, 2 = F)
    unsigned int size;                                      // Size of the file
    unsigned int link;                                      // Count links
    unsigned int blocks[12];                                // Data's address (12 direct pointer)
    // 4 + 4 + 4 * 12 + 4 + 4 = 64 byte
};

typedef struct InodeTable InodeTable;
struct InodeTable { char name[28]; unsigned int inode_number; };

// Name                 Chunk
// Super                0       
// Inodes Bitmap        1
// Data Bitmap          2           
// Inodes               3 - 7
// Data                 8 - 63

// Setup parameter              
// Use store and read it in Super later
const unsigned int diskSize = 4 * 1024 * 16 * 4;                    // byte (256KB)
const unsigned int chunkSize = 4096;                                // byte (4KB)

const Bitmap inodeBitmap = { .size = (chunkSize * 5) / sizeof(Inode), .chunk = 1, .address = 3 * chunkSize };
const Bitmap dataBitmap = { .size = 56,.chunk = 2, .address = 8 * chunkSize };

const unsigned int inodeLinks = sizeof(Inode) / sizeof(unsigned int) - 4;
unsigned int rootInode;

void* _m(size_t size);
void* _r(void* p, size_t size);

char** SplitString(char* string, char* p);

char* MappingAddress(unsigned int address);
unsigned int GetAddressFromChunk(unsigned int chunk);
unsigned int GetChunkFromAddress(unsigned int address);
unsigned int GetInodeAddress(unsigned int inode);
unsigned int GetDataAddress(unsigned int data);

int CheckFreeChunk(Bitmap bitmap, int number);
unsigned int GetFreeChunk(Bitmap bitmap);
void FreeChunk(Bitmap bitmap, int number);

int Mkdir(char* path);
Inode CreateDirectory(int parent);
int WriteData(unsigned int address, char* dat, int size);

unsigned int ReadBit(char x, int bit);
char* ReadData(void* dest, unsigned int address, int size);
InodeTable* ReadInode(unsigned int inode);
void PrintData(char* data, int size);

void PrintInode(Inode inode) {
    printf("\n_INODE____________________________________________________\n");
    printf("number: %-6d size: %-6d link: %-6d type: %-6s\nblocks: ", inode.inode_number, inode.size, inode.link, inode.type == DIRECTORY ? "directory" : "file");
    for (int i = 0; i < inodeLinks;printf(" %d |", inode.blocks[i++]));

    if (inode.type == FILE) {

        return;
    }
    printf("\n_DIRECTORY________________________________________________\n");
    int len = inode.size / sizeof(InodeTable);

    InodeTable table[len];
    ReadData((char*)table, GetDataAddress(inode.blocks[0]), sizeof(table));

    for (int i = 0; i < len; i++) printf("  %-4d %s\n", table[i].inode_number, table[i].name);

    printf("\n");
}

void Initialize() {
    srand(time(NULL));
    printf("Initialize VFSF...\n");

    // Allocate memory
    data = _m(diskSize);

    // Setup Bitmap

    // Create root directory
    Inode root = CreateDirectory(-1);
    PrintInode(root);

    Inode test;
    ReadData((char*)&test, GetInodeAddress(root.inode_number), sizeof(Inode));

    if (root.blocks[0] != test.blocks[0] || root.inode_number != test.inode_number || root.size != test.size) {
        printf("Failed Creating Root Folder");
        exit(-1);
    }

    printf("\nSuccessful Initialize\n");
}

void CleanUp() { free(data); }

int main() {
    Initialize();

    Mkdir("//path/ed");

    CleanUp();

    return 0;
}
void* _m(size_t size) {
    void* p = NULL; do p = calloc(size, 1); while (!p);
    return p;
}

void* _r(void* p, size_t size) {
    void* o = NULL; do o = realloc(p, size); while (!o);
    return o;
}
char** SplitString(char* string, char* p) {
    int count = 4, k = 0, len = strlen(string);

    char a[len], * b, ** A = _m(count * sizeof(void*));
    strcpy(a, string);



    while (strstr(a, p) != NULL) {
        b = strstr(a, p);
        int l = b - a;
        if (l > 0) A[k++] = memcpy(_m(l + 1), a, l);
        strcpy(a, b + 1);

        if (k == count) A = _r(A, (count *= 2) * sizeof(char*));
    }

    A[k++] = strcpy(_m(strlen(a) + 1), a);
    A[k++] = NULL;
    return (char**)_r(A, k * sizeof(char*));
}

char* MappingAddress(unsigned int address) {
    return data + address;
}
unsigned int GetAddressFromChunk(unsigned int chunk) {
    return chunk * chunkSize;
}
unsigned int GetChunkFromAddress(unsigned int address) {
    return address / chunkSize;
}
unsigned int GetInodeAddress(unsigned int inode) {
    return inodeBitmap.address + sizeof(Inode) * inode;
}
unsigned int GetDataAddress(unsigned int data) {
    return dataBitmap.address + chunkSize * data;
}

Inode CreateDirectory(int parent) {
    Inode inode = { .inode_number = GetFreeChunk(inodeBitmap), .link = 2,.type = DIRECTORY };

    inode.blocks[0] = GetFreeChunk(dataBitmap);
    for (int i = 1; i < 13; inode.blocks[i++] = UINT_MAX);

    if (inode.inode_number >= inodeBitmap.size || inode.blocks[0] >= dataBitmap.size) return inode;

    InodeTable table[2] = {
        {.name = ".", .inode_number = inode.inode_number},
        {.name = "..", .inode_number = parent == -1 ? inode.inode_number : parent},
    };
    inode.size = sizeof(table);

    WriteData(GetInodeAddress(inode.inode_number), (char*)&inode, sizeof(inode));
    WriteData(GetDataAddress(inode.blocks[0]), (char*)table, sizeof(table));

    return inode;
}
InodeTable* IsExistPath(char* path) {
    return 1;
}
int Mkdir(char* path) {
    if (!IsExistPath(path)) return -1;

    return 1;
}

int WriteData(unsigned int address, char* dat, int size) {
    if (address == UINT_MAX) return UINT_MAX;
    memcpy(MappingAddress(address), dat, size);
}
unsigned int ReadBit(char x, int bit) { return (x & (1 << bit)) >> bit; }
char* ReadData(void* dest, unsigned int address, int size) { return memcpy(dest, MappingAddress(address), size); }

void PrintData(char* data, int size) {
    for (int i = 0; i < size;++i) {
        for (int j = 0; j < 8; printf("%d", ReadBit(data[i], j++)));
        printf((i + 1) % 8 != 0 ? " " : "\n");
    }
}
int CheckFreeChunk(Bitmap bitmap, int number) {
    return ReadBit(MappingAddress(GetAddressFromChunk(bitmap.chunk))[(int)(number / 8)], number % 8);
}

unsigned int GetFreeChunk(Bitmap bitmap) {
    char* dat = MappingAddress(GetAddressFromChunk(bitmap.chunk));

    for (int i = 0; i < ceil(bitmap.size / 8); ++i) {
        for (int j = 0; j < 8; ++j) {
            if (ReadBit(dat[i], j)) continue;                             // If dat is 1, continue
            dat[i] |= (1 << j);                                           // Write to Bitmap
            return i * 8 + j;
        }
    }
    return UINT_MAX;
}

void FreeChunk(Bitmap bitmap, int number) {
    char* dst = MappingAddress(GetAddressFromChunk(bitmap.chunk));
    if (ReadBit(dst[(number / 8)], number % 8) == 0) return;
    dst[(number / 8)] -= pow(2, number % 8);
}
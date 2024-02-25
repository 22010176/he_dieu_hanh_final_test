#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdlib.h>

#define pf                  printf
#define class(x)            typedef struct x x; struct x
#define max(a, b)           (a) > (b) ? (a) : (b)
#define min(a, b)           (a) < (b) ? (a) : (b)
#define CalcSize(a, b)      (size_t)ceil((double)a / b)
#define FreeMem(p)          for (int i = 0; p[i] != NULL; ++i) free(p[i]); free(p)

#define _FILE                2
#define _DIRECTORY           1
#define _ANYTYPE             3

#define SUCCESS              2
#define FULL                 1
#define EMPTY                -1
#define FAIL                 -1

#define MaxPointers          12
#define MaxFileNameLength    28

class (Inode) { unsigned int id, type, link, size, blocks[MaxPointers]; };
class (InodeTable) { int id; char name[MaxFileNameLength]; };

class (Super) {
    char* disk;
    size_t chunkSize, diskSize, numberInode, numberChunk;

    char* inodeBitmapChunk, * dataBitmapChunk;
    Inode* inodeChunk;
    char* dataChunk;

    int rootInode;
};

#endif
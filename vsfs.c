#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "utils.h"

#define class(x)        typedef struct x x; struct x
#define max(a, b)       a > b ? a : b
#define min(a, b)       a < b ? a : b
#define CalcSize(a, b)  (size_t)ceil((double)a / b)
#define FreeMem(p)      for (int i = 0; p[i] != NULL; ++i) free(p[i]); free(p)

#define _FILE            0
#define _DIRECTORY       1

#define SUCCESS         2
#define EMPTY           -1
#define FULL            1
#define FAIL            -1

#define MaxPointers     12


class (Inode) { int id, type, link, size, blocks[MaxPointers]; };
class (InodeTable) { int id; char name[28]; };


char* data; // 4kb

size_t chunkSize = 1024 * 4;
size_t diskSize = 256 * 1024;
size_t inodeSize = sizeof(Inode);
size_t inodeTableSize = sizeof(InodeTable);
size_t size_tSize = sizeof(size_t);
size_t intSize = sizeof(int);
size_t charSize = sizeof(char);

size_t numberInode, numberChunk;

char* inodeBitmapChunk, * dataBitmapChunk, * dataChunk;
Inode* inodeChunk;

int rootInode;

int GetFreeCell(char* bitmapChunk, int size) {
    for (int i = 0; i < size; i++) {
        if (bitmapChunk[i] != 0) continue;
        bitmapChunk[i] = FULL;
        return i;
    }
    return FAIL;
}

void InitParam() {
    data = _ca(diskSize);

    inodeBitmapChunk = data + 1024;
    dataBitmapChunk = data + 2 * 1024;
    inodeChunk = (Inode*)(data + 3 * 1024);
    dataChunk = data + 8 * 1024;
    numberInode = CalcSize(inodeSize, 5 * chunkSize);
    numberChunk = CalcSize(diskSize, chunkSize) - 8;
}

Inode CreateInode(int id, int type) {
    if (inodeBitmapChunk[id] == 0 || id >= numberInode) {
        printf("Invalid inode");
        return (Inode) {};
    }

    Inode inode = (Inode){ .id = id,.type = type,.link = 0,.size = 0 };
    for (int i = 0; i < MaxPointers; ++i) inode.blocks[i] = EMPTY;

    memcpy(&inodeChunk[inode.id], &inode, inodeSize);
    return inodeChunk[id];
}
int GetFreePointer(Inode* inode) {
    for (int i = 0; i < MaxPointers; ++i) if (inode->blocks[i] == EMPTY) return i;
    return MaxPointers;
}

void PrintInode(int inodeNum) {
    Inode inode = inodeChunk[inodeNum];
    printf("ID: %-3d TYPE: %-10s LINK: %-5d SIZE: %d\n", inode.id, inode.type == _FILE ? "File" : "Directory", inode.link, inode.size);

    for (int i = 0; i < MaxPointers; ++i) printf("%2d ", inode.blocks[i]);

    if (inode.type == _FILE) return;
    printf("\n");

    for (int i = 0, temp = 0; inode.blocks[i] != EMPTY && inode.size > temp;++i) {
        InodeTable* tables = (InodeTable*)(dataChunk + inode.blocks[i] * chunkSize);
        for (int j = 0; j * inodeTableSize < chunkSize && temp < inode.size; ++j) {
            printf("| %-5d %-30s |\n", tables[j].id, tables[j].name);
            temp += inodeTableSize;
        }
    }
}

int UpdateInodeData(Inode* inode, char* data, size_t size) {
    size_t sizeRemain = CalcSize(inode->size, chunkSize) * chunkSize - inode->size;
    int pointer = GetFreePointer(inode);

    if (sizeRemain > size && pointer > 0) {
        memcpy(dataChunk + inode->blocks[pointer - 1] * chunkSize + inode->size % chunkSize, data, size);
        inode->size += size;
        return SUCCESS;
    }

    if (pointer == MaxPointers) {
        printf("Cant not find free chunk\n");
        return FAIL;
    }

    size_t temp = size;
    size_t offset = 0;
    while (temp > 0) {
        int freeChunk = GetFreeCell(dataBitmapChunk, numberChunk);
        if (freeChunk == FAIL) {
            printf("Run out of free chunk!!!\n");
            return FAIL;
        }
        size_t writeSize = min(chunkSize, size);
        memcpy(dataChunk + freeChunk * chunkSize, data + offset, writeSize);

        temp -= writeSize; offset += writeSize; inode->size += writeSize;
        inode->blocks[pointer++] = freeChunk;
    }
    return SUCCESS;
}
void AddLinking(Inode* inode, InodeTable table) {
    if (inodeBitmapChunk[table.id] == 0) {
        printf("Inode doesnt exist!!!\n");
        return;
    }

    int result = UpdateInodeData(inode, (char*)&table, inodeTableSize);
    if (result == FAIL) {
        printf("Couldnt add link to folder, run out of space.\n");
        return;
    }
    ++inodeChunk[table.id].link;
}

int IsThisFileInFolder(Inode* inode, char* path) {
    for (int i = 0, temp = 0; inode->blocks[i] != EMPTY && inode->size > temp;++i) {
        InodeTable* tables = (InodeTable*)(dataChunk + inode->blocks[i] * chunkSize);

        for (int j = 0; j * inodeTableSize < chunkSize && temp < inode->size; ++j) {
            if (!strcmp(tables[j].name, path)) return tables->id;
            temp += inodeTableSize;
        }
    }
    return FAIL;
}

int IsPathExist(char** p, int mode) {
    int inodeID = rootInode;
    for (int i = 0; p[i + mode] != NULL; ++i) {
        inodeID = IsThisFileInFolder(&inodeChunk[inodeID], p[i]);
        if (inodeID == FAIL) {
            printf("Cant not find item in folder!!!\n");
            return FAIL;
        }
    }
    return inodeID;
}
void UpdateInode(Inode* inode) {
    if (inodeBitmapChunk[inode->id] == 0) {
        printf("Inode isnt ocupied yet!\n");
        return;
    }
    memcpy(&inodeChunk[inode->id], inode, inodeSize);
}

void v_mkdir(char* path) {
    char** splitStr = SplitString(path, "/");


    char* fileName = GetFileName(splitStr);
    int inodeParentID = IsPathExist(splitStr, 1), inodeChildID = IsPathExist(splitStr, 0);

    if (inodeParentID == FAIL || inodeChildID != FAIL) {
        printf("Cant create file/folder in this path. It is already existed or parent folder not found.\n");
        FreeMem(splitStr);
        free(fileName);
        return;
    }
    inodeChildID = GetFreeCell(inodeBitmapChunk, numberInode);
    Inode child = CreateInode(inodeChildID, _DIRECTORY), parent = inodeChunk[inodeParentID];

    InodeTable childTable; childTable.id = child.id; strcpy(childTable.name, fileName);

    AddLinking(&parent, childTable);

    AddLinking(&child, (InodeTable) { .id = parent.id, .name = ".." });
    AddLinking(&child, (InodeTable) { .id = child.id, .name = "." });

    UpdateInode(&child);
    UpdateInode(&parent);

    FreeMem(splitStr);
    free(fileName);
}
void v_unlink();
void v_link();

void v_open();
void v_write();

int main() {
    InitParam();

    rootInode = GetFreeCell(inodeBitmapChunk, numberInode);

    inodeChunk[rootInode] = CreateInode(rootInode, _DIRECTORY);
    AddLinking(inodeChunk + rootInode, (InodeTable) { .id = rootInode, .name = ".." });
    AddLinking(inodeChunk + rootInode, (InodeTable) { .id = rootInode, .name = "." });
    AddLinking(inodeChunk + rootInode, (InodeTable) { .id = rootInode, .name = "root" });
    AddLinking(inodeChunk + rootInode, (InodeTable) { .id = rootInode, .name = "" });

    v_mkdir("/a/");
    v_mkdir("/a/b");
    v_mkdir("/a/b/c");
    v_mkdir("/b");

    PrintInode(rootInode);
    PrintInode(1);
    PrintInode(2);
    PrintInode(3);

    free(data);
    return 0;
}
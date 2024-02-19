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
#define _ANYTYPE         3

#define SUCCESS          2
#define FULL             1
#define EMPTY           -1
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

    inodeBitmapChunk = data + chunkSize;
    dataBitmapChunk = data + 2 * chunkSize;
    inodeChunk = (Inode*)(data + 3 * chunkSize);
    dataChunk = data + 8 * chunkSize;
    numberInode = CalcSize(inodeSize, 5 * chunkSize);
    numberChunk = CalcSize(diskSize, chunkSize) - 8;
}
int GetFreePointer(Inode* inode) {
    for (int i = 0; i < MaxPointers; ++i) if (inode->blocks[i] == EMPTY) return i;
    return MaxPointers;
}
char* ReadInode(char* _dst, Inode* inode) {
    for (int i = 0, offset = 0, temp = inode->size; inode->blocks[i] != EMPTY;++i) {
        size_t readSize = min(chunkSize, temp);
        memcpy(_dst + offset, dataChunk + inode->blocks[i] * chunkSize, readSize);
        offset += readSize; temp -= readSize;
    }
    return _dst;
}
Inode CreateInode(int type) {
    int id = GetFreeCell(inodeBitmapChunk, numberInode);
    if (id >= numberInode || id == FAIL) {
        printf("Invalid inode");
        return (Inode) {};
    }

    Inode inode = (Inode){ .id = id ,.type = type,.link = 0,.size = 0 };
    for (int i = 0; i < MaxPointers; ++i) inode.blocks[i] = EMPTY;

    memcpy(&inodeChunk[inode.id], &inode, inodeSize);
    return inodeChunk[id];
}

void UpdateInode(Inode* inode) {
    if (inodeBitmapChunk[inode->id] == 0) {
        printf("Inode isnt ocupied yet!\n");
        return;
    }
    memcpy(&inodeChunk[inode->id], inode, inodeSize);
}
void PrintInode(int inodeNum) {
    printf("\n\n");
    if (dataBitmapChunk[inodeNum] == 0) return;

    Inode inode = inodeChunk[inodeNum];
    printf("ID: %-3d TYPE: %-10s LINK: %-5d SIZE: %d\n", inode.id, inode.type == _FILE ? "File" : "Directory", inode.link, inode.size);

    for (int i = 0; i < MaxPointers; ++i) printf("%2d ", inode.blocks[i]);

    if (inode.type == _FILE) return;
    printf("\n");

    int len = CalcSize(inode.size, inodeTableSize);
    InodeTable tables[CalcSize(inode.size, inodeTableSize)]; ReadInode((char*)tables, &inode);
    for (int i = 0; i < len; ++i) printf("| %-5d %30s |\n", tables[i].id, tables[i].name);
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

        if (pointer >= MaxPointers) {
            printf("No more pointer.\n");
            return FAIL;
        }
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
    if (inode->id == table.id) ++inode->link;
    else ++inodeChunk[table.id].link;
    // PrintInode(table.id);
}

int IsThisFileInFolder(Inode* inode, char* path, int type) {
    int len = CalcSize(inode->size, inodeTableSize);
    InodeTable table[len]; ReadInode((char*)table, inode);

    for (int i = 0; i < len; ++i) {
        if (strcmp(table[i].name, path)) continue;
        if (inodeChunk[table[i].id].type == type || type == _ANYTYPE) return table[i].id;
    }

    return FAIL;
}

int IsPathExist(char** p, int level, int type) {
    int inodeID = rootInode;
    for (int i = 0; p[i + level] != NULL; ++i) {
        inodeID = IsThisFileInFolder(&inodeChunk[inodeID], p[i], type);
        if (inodeID == FAIL) break;
    }
    return inodeID;
}

void v_mkdir(char* path) {
    char** splitStr = SplitString(path, "/");

    char* fileName = GetFileName(splitStr);
    int inodeParentID = IsPathExist(splitStr, 1, _DIRECTORY), inodeChildID = IsPathExist(splitStr, 0, _DIRECTORY);

    if (inodeParentID == FAIL || inodeChildID != FAIL) {
        if (inodeParentID == FAIL) printf("Path doesnt not existed.\n");
        if (inodeChildID != FAIL) printf("File already existesd.\n");
        FreeMem(splitStr);
        free(fileName);
        return;
    }
    Inode child = CreateInode(_DIRECTORY), parent = inodeChunk[inodeParentID];
    InodeTable childTable; childTable.id = child.id; strcpy(childTable.name, fileName);

    AddLinking(&child, (InodeTable) { .id = parent.id, .name = ".." });
    AddLinking(&child, (InodeTable) { .id = child.id, .name = "." });
    UpdateInode(&child);

    AddLinking(&parent, childTable);
    UpdateInode(&parent);

    FreeMem(splitStr);
    free(fileName);
}

void v_link(char* _dst, char* _src) {
    char** dstSplit = SplitString(_dst, "/");
    char** srcSplit = SplitString(_src, "/");

    char* dstName = GetFileName(dstSplit);
    int dstInode = IsPathExist(dstSplit, 1, _DIRECTORY);
    int srcInode = IsPathExist(srcSplit, 0, _ANYTYPE);

    if (dstInode == FAIL || srcInode == FAIL) {
        if (dstInode == FAIL) printf("Cant not find path.");
        if (srcInode == FAIL) printf("source file doesnt exist.");
        return;
    }
    InodeTable table = { .id = srcInode,.name = "" };
    strcpy(table.name, dstName);

    AddLinking(&inodeChunk[dstInode], table);

    FreeMem(dstSplit);
    FreeMem(srcSplit);
    free(dstName);
}
void v_unlink(char* dst) {
    char** strSplit = SplitString(dst, "/");

    int parentNumber = IsPathExist(strSplit, 1, _DIRECTORY);

    FreeMem(strSplit);
};

void v_open();
void v_write();

void PrintFileStructure(int inodeNumber, int level) {
    if (level == 0)printf("%d .\n", inodeNumber);
    int len = CalcSize(inodeChunk[inodeNumber].size, inodeTableSize);
    InodeTable tables[len]; ReadInode((char*)tables, &inodeChunk[inodeNumber]);
    for (int i = 0; i < len; ++i) {
        if (inodeNumber == tables[i].id || !strcmp("..", tables[i].name)) continue;
        Inode inode = inodeChunk[tables[i].id];
        printf("%*d. %s%s\n", (level + 1) * 5, tables[i].id, tables[i].name, inode.type == _FILE ? ".txt" : "/");
        if (inode.type == _DIRECTORY) PrintFileStructure(tables[i].id, level + 1);
    }


}


int main() {
    InitParam();

    Inode root = CreateInode(_DIRECTORY);
    rootInode = root.id;
    AddLinking(&root, (InodeTable) { .id = rootInode, .name = ".." });
    AddLinking(&root, (InodeTable) { .id = rootInode, .name = "." });
    AddLinking(&root, (InodeTable) { .id = rootInode, .name = "root" });
    AddLinking(&root, (InodeTable) { .id = rootInode, .name = "" });

    UpdateInode(&root);

    // v_mkdir("/a/");
    v_mkdir("/ab/");
    v_mkdir("/ab/d");
    v_mkdir("ab/e/");
    v_mkdir("ab/e/f");
    v_mkdir("ab/e/g");
    v_mkdir("a");
    v_mkdir("ab/f");
    v_link("/ab/ed", "a");

    // PrintInode(rootInode);
    // PrintInode(1);
    PrintFileStructure(rootInode, 0);

    free(data);
    return 0;
}
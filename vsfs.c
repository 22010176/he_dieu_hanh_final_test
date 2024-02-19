#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "utils.h"
#include "types.h"

// gcc *.c -o out/vsfs.exe
// ./out/vsfs

char* data; // 4kb

size_t chunkSize = 1024;
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

int GetFreeCell(char* bitmapChunk, int size);
int FreeCell(char* bitmapChunk, int size, int cell);

void InitParam();
void InitFolder();

Inode CreateInode(int type);
char* ReadInode(char* _dst, Inode* inode);
int GetFreePointer(Inode* inode);
void PrintInode(int inodeNum);
void UpdateInode(Inode* inode);
void PrintInode2(Inode* inode);
int UpdateInodeData(Inode* inode, char* data, size_t size);
void AddLinkToInode(Inode* inode, InodeTable table);
int IsThisFileInFolder(Inode* inode, char* path, int type);

void v_mkdir(char* path);
void v_link(char* _dst, char* _src);
void v_unlink(char* dst);
void v_open(char* path);
void v_write(char* path, char* content, size_t size);

void PrintFileStructure(int inodeNumber, int level);


/* TEST LIST

v_mkdir
    Create folder at destination
        1. Check if parent isnt exists          a
        2. Check if the path is existed.        b
        3. a or b true => Throw error

        4. if a or b false
            Create child Inode (check bitmap if the bit is correct set and file type PrintInode2())
            Link with Parent folder. (check the inode tables) Use ReadInode();

        5. Update it to the disk
            Use PrintInode(inodeID) to check.

v_open
    Create file at destination
        1. Check if parent isnt exists      a
        2. Check if the path is existed.    b
        3. a or b true => Throw error

        4. if a or b false
            Create child Inode (check bitmap if the bit is correct set and file type PrintInode2())
            Link with Parent folder. (check the inode tables) Use ReadInode();

        5. Update it to the disk
            Use PrintInode(inodeID) to check.

v_link
    Linking source path to destinate path
        1. Check if source path is exists.                          a
        2. Check if parent folder of destinate path is existed.     b
        3. a or b true => Throw error

        4. false
            Link the source path to destinate path. (Check the inodeTable of destinate inode);

v_unlink
    Remove item in path
        1. Check if item exists
        2. false -> throw error

        3. true -> FILE: check if it is file or not
            Is file: Remove it from parent inodeTable (Check Inode Table again)

        4. false -> DIRECTORY: Check if folder is empty
            True:       remove it like file
            False:      throw error

v_write
    Write Content to the inode table
        1. Check path like usuals
        2. Write content (Use Print Data and check if data is corrected write)
            Check blocks if it is correctly set.
            Check size if it is the same with data's size
            Check if the data in disk is the same as the the write data (can try write string to easier debug)

* Delete folder isnt empty (denied); Use PrintFileStructure to test
* Delete folder completely; Check inode bitmap, data bitmap. Use _Print to see that bit set to 0
    eg:
    there are 80 inodes -> bitmap:
    0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
    0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
    Inode 1 is set =>
    0100 0000 0000 0000 0000 0000 0000 0000 0000 0000
    0000 0000 0000 0000 0000 0000 0000 0000 0000 0000

    So if we delete it, it need to be 0

* Create file, write content to it. Focus on large content that require multiple block to stored.
    Check if data is preserved.
    Check if file run out of pointers, will it return error log.

* Check if link work, try to link it to multiples destination and print the folder structures;

*/
int main() {
    InitParam();
    InitFolder();


    // create some directories
    v_mkdir("a");
    v_mkdir("C");
    v_mkdir("/ab/");
    v_mkdir("/ab/d");
    v_mkdir("ab/e/");
    v_mkdir("ab/e/f");
    v_mkdir("ab/e/f/h");
    v_mkdir("ab/e/g");
    v_mkdir("ab/f");

    // Can understand that it is just copy folder
    v_link("/ab/ed", "a");
    v_link("ab/e/eh", "ab/d");

    // Create some more directory
    v_mkdir("b");
    v_mkdir("bdd");
    v_mkdir("bddd");
    v_mkdir("bddd");

    // Create File
    v_open("a.txt");
    v_open("b/bee");

    // Write to file some content
    v_write("b/bee", "Asdfasdf", 9);

    // Remove or delete item
    v_unlink("ab/d/");
    v_unlink("a");
    v_unlink("C");

    // Print folder structure
    PrintFileStructure(rootInode, 0);

    _Print(inodeBitmapChunk, numberInode / 8);
    _Print(dataBitmapChunk, numberChunk / 8);

    free(data);
    return 0;
}

int CheckCell(char* bitmapChunk, int size, int cell) {
    if (cell >= size) {
        printf("Invalid Cell number.\n");
        return FAIL;
    }
    return ReadBit(bitmapChunk[cell / 8], cell % 8);
}
int GetFreeCell(char* bitmapChunk, int size) {
    int len = CalcSize(size, 8);
    for (int i = 0; i < len; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (ReadBit(bitmapChunk[i], j)) continue;                                // If address is 1, continue
            bitmapChunk[i] |= (1 << j);                                                     // Write to Bitmap
            return i * 8 + j;
        }
    }

    return FAIL;
}
int FreeCell(char* bitmapChunk, int size, int cell) {
    if (cell >= size) {
        printf("Invalid Cell number.\n");
        return FAIL;
    }
    if (ReadBit(bitmapChunk[cell / 8], cell % 8) == 0) return SUCCESS;
    bitmapChunk[cell / 8] -= 1 << (cell % 8);

    return SUCCESS;
}
void InitParam() {
    data = _ca(diskSize);

    inodeBitmapChunk = data + chunkSize;
    dataBitmapChunk = data + 2 * chunkSize;
    inodeChunk = (Inode*)(data + 3 * chunkSize);
    dataChunk = data + 8 * chunkSize;
    numberInode = CalcSize(5 * chunkSize, inodeSize);
    numberChunk = CalcSize(diskSize, chunkSize) - 8;
}
void InitFolder() {
    Inode root = CreateInode(_DIRECTORY);
    rootInode = root.id;
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = ".." });
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = "." });
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = "root" });
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = "" });

    UpdateInode(&root);
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
void PrintInode(int inodeNum) {
    printf("\n\n");
    if (CheckCell(inodeBitmapChunk, numberInode, inodeNum) == 0) return;

    Inode inode = inodeChunk[inodeNum];
    printf("ID: %-3d TYPE: %-10s LINK: %-5d SIZE: %d\n", inode.id, inode.type == _FILE ? "File" : "Directory", inode.link, inode.size);

    for (int i = 0; i < MaxPointers; ++i) printf("%2d ", inode.blocks[i]);

    if (inode.type == _FILE) return;
    printf("\n");

    int len = CalcSize(inode.size, inodeTableSize);
    InodeTable tables[CalcSize(inode.size, inodeTableSize)]; ReadInode((char*)tables, &inode);
    for (int i = 0; i < len; ++i) printf("| %-5d %30s |\n", tables[i].id, tables[i].name);
}
void UpdateInode(Inode* inode) {
    if (CheckCell(inodeBitmapChunk, numberInode, inode->id) == 0) {
        printf("Inode isnt ocupied yet!\n");
        return;
    }
    memcpy(&inodeChunk[inode->id], inode, inodeSize);
}
Inode CreateInode(int type) {
    int id = GetFreeCell(inodeBitmapChunk, numberInode);
    if (id >= numberInode || id == FAIL) {
        printf("Invalid inode");
        return (Inode) {};
    }

    Inode inode = { .id = id ,.type = type,.link = 0,.size = 0 };
    for (int i = 0; i < MaxPointers; ++i) inode.blocks[i] = EMPTY;

    UpdateInode(&inode);
    return inode;
}

void PrintInode2(Inode* inode) {
    printf("\n\n");

    printf("ID: %-3d TYPE: %-10s LINK: %-5d SIZE: %d\n", inode->id, inode->type == _FILE ? "File" : "Directory", inode->link, inode->size);

    for (int i = 0; i < MaxPointers; ++i) printf("%2d ", inode->blocks[i]);

    if (inode->type == _FILE) return;
    printf("\n");

    int len = CalcSize(inode->size, inodeTableSize);
    InodeTable tables[CalcSize(inode->size, inodeTableSize)]; ReadInode((char*)tables, inode);
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
void AddLinkToInode(Inode* inode, InodeTable table) {
    if (CheckCell(inodeBitmapChunk, numberInode, inode->id) == 0) {
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
        inodeID = IsThisFileInFolder(&inodeChunk[inodeID], p[i], p[i + level + 1] == NULL ? type : _ANYTYPE);
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

    AddLinkToInode(&child, (InodeTable) { .id = parent.id, .name = ".." });
    AddLinkToInode(&child, (InodeTable) { .id = child.id, .name = "." });
    UpdateInode(&child);

    AddLinkToInode(&parent, childTable);
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
        if (dstInode == FAIL) printf("Cant not find path.\n");
        if (srcInode == FAIL) printf("source file doesnt exist.\n");
        return;
    }
    InodeTable table = { .id = srcInode,.name = "" };
    strcpy(table.name, dstName);

    AddLinkToInode(&inodeChunk[dstInode], table);

    FreeMem(dstSplit);
    FreeMem(srcSplit);
    free(dstName);
}
void v_unlink(char* dst) {
    char** strSplit = SplitString(dst, "/");

    int parentNumber = IsPathExist(strSplit, 1, _DIRECTORY);
    int dstNumber = IsPathExist(strSplit, 0, _ANYTYPE);

    if (parentNumber == FAIL || dstNumber == FAIL) {
        printf("Cant unlink item: %s\n", dst);
        return;
    }

    if (--inodeChunk[dstNumber].link == 1) {
        Inode inode = inodeChunk[dstNumber];
        for (int i = 0; inode.blocks[i] != EMPTY; ++i) FreeCell(dataBitmapChunk, numberChunk, inode.blocks[i]);
        FreeCell(inodeBitmapChunk, numberInode, dstNumber);
    }

    Inode inode = inodeChunk[parentNumber];
    int len = CalcSize(inode.size, inodeTableSize);
    if (inode.type == _DIRECTORY && len > 2) {
        printf("There are item inside directory, cant not remove!\n");
        return;
    }
    InodeTable tables[len]; ReadInode((char*)tables, &inodeChunk[parentNumber]);

    for (int i = 0, j = 0; i < len - 1;i++) {
        if (tables[i].id == dstNumber) j = 1;
        if (j == 0) continue;
        tables[i] = tables[i + j];
    }

    for (int i = 0; inode.blocks[i] != EMPTY; ++i) {
        FreeCell(dataBitmapChunk, numberChunk, inode.blocks[i]);
        inode.blocks[i] = EMPTY;
    }
    inode.size = 0;
    UpdateInodeData(&inode, (char*)tables, (len - 1) * inodeTableSize);
    UpdateInode(&inode);

    FreeMem(strSplit);
};

void v_open(char* path) {
    char** splitStr = SplitString(path, "/");

    char* fileName = GetFileName(splitStr);
    int inodeParentID = IsPathExist(splitStr, 1, _DIRECTORY);
    int inodeChildID = IsPathExist(splitStr, 0, _FILE);

    if (inodeParentID == FAIL || inodeChildID != FAIL) {
        if (inodeParentID == FAIL) printf("Path doesnt not existed.\n");
        if (inodeChildID != FAIL) printf("File already existesd.\n");

        FreeMem(splitStr);
        free(fileName);
        return;
    }

    Inode child = CreateInode(_FILE), parent = inodeChunk[inodeParentID];

    InodeTable childTable;
    childTable.id = child.id; strcpy(childTable.name, fileName);

    UpdateInode(&child);

    AddLinkToInode(&parent, childTable);
    UpdateInode(&parent);

    FreeMem(splitStr);
    free(fileName);
}
void v_write(char* path, char* content, size_t size) {
    char** splitStr = SplitString(path, "/");
    int dst = IsPathExist(splitStr, 0, _FILE);

    if (dst == FAIL) {
        printf("File doesnt exist: %s\n", path);
        FreeMem(splitStr);
        return;
    }
    Inode inode = inodeChunk[dst];
    UpdateInodeData(&inode, content, size);
    UpdateInode(&inode);

    FreeMem(splitStr);
}

void PrintFileStructure(int inodeNumber, int level) {
    if (level == 0) printf("%d .\n", inodeNumber);
    int len = CalcSize(inodeChunk[inodeNumber].size, inodeTableSize);
    InodeTable tables[len]; ReadInode((char*)tables, &inodeChunk[inodeNumber]);
    for (int i = 0; i < len; ++i) {
        if (inodeNumber == tables[i].id || !strcmp("..", tables[i].name)) continue;

        Inode inode = inodeChunk[tables[i].id];
        printf("%*d. %s%s\n", (level + 1) * 5, tables[i].id, tables[i].name, inode.type == _FILE ? "" : "/");

        if (inode.type == _DIRECTORY) PrintFileStructure(tables[i].id, level + 1);
        else {
            // if (inode.size == 0) continue;
            // char data[inode.size]; ReadInode(data, &inode);
            // _Print(data, inode.size);
            // puts(data);
        }
    }
}
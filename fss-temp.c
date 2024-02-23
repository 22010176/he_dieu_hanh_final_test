#include "types.h"
#include "utils.h"
#include "bitmap.h"

#define GROUP_NUMBER                                16
#define CHUNK_SIZE                                  256

class (Address) { size_t block, offset; };

/*
Inode:
Tất cả các hàm sẽ không cập nhật inode vào bộ nhớ. Chỉ khi gọi UpdateInode, thì inode mới được cập nhất vào bộ nhớ.

Block access:
Sử dụng số đếm thực tế để xác định chunk.
Vd: 1 block có 24 inode. Nếu ta có inode có mã là 34.
Để xác định block, ta có
block = 34 / numberInode = 34 / 24 = 1.xxxx.
Ta sẽ lấy số 1, xác định được là block thứ 1. Và là inode thứ 10 trong block thứ nhất.

Arr
[item1, item2, item3, ..., itemN, EMPTY(-1)]
sẽ để arr kết thúc bằng item EMPTY hay là -1.
*/

char* disk;
size_t diskSize = 256 * 1024;                           // 4KB
size_t size_tSize = sizeof(size_t);
size_t intSize = sizeof(int);
size_t charSize = sizeof(char);

// Constants
size_t inodeSize = sizeof(Inode);
size_t inodeTableSize = sizeof(InodeTable);



// Change with group
size_t blockSize;
size_t chunkSize;
size_t numberChunk;
size_t numberInode;

int rootBlock = 0;
int rootInode = 0;                                          // index of root inode stored in file system (usually 0)
int blockIndex = 0;

char* inodeBitmapChunk;
char* dataBitmapChunk;
Inode* inodeChunk;
char* dataChunk;

Super* vss[GROUP_NUMBER];

// Debug
void PrintSuper(Super* temp);

// Group management
size_t GetRealIndex(Address localAddress, size_t itemSize) { return localAddress.block * itemSize + localAddress.offset % itemSize; }
Address GetAddress(size_t index, size_t size) { return (Address) { .block = (size_t)floor(index / size), .offset = index % size }; }
void SetBlock(size_t bIndex) {
    blockIndex = (bIndex % GROUP_NUMBER);

    Super* temp = vss[blockIndex];
    inodeBitmapChunk = temp->inodeBitmapChunk;
    dataBitmapChunk = temp->dataBitmapChunk;
    inodeChunk = temp->inodeChunk;
    dataChunk = temp->dataChunk;
}


void InitParam();
void InitRootFolder();

Address FindFreeInode(int type);
Address FindFreeChunk();

size_t GetArrIntLen(int arr[]) {
    size_t k = 0; for (;arr[k] != EMPTY; ++k);
    return k;
}

Inode CreateInode(int type);
void UpdateInode(Inode* inode);
int UpdateInodeData(Inode* inode, char* data, size_t size);
char* ReadInodeData(char dest[], Inode* inode);
int GetFreePointer(Inode* inode);
void PrintInodeFromMemory(int inodeIndex);
void PrintInode(Inode* inode);
void AddLinkToInode(Inode* inode, InodeTable table);

int IsThisFileInFolder(Inode* inode, char* path, int type);
int IsPathExist(char** p, int level, int type);

void v_mkdir(char* path);
void v_open(char* path);
int v_write(char* path, char* content, size_t size);

void PrintVSFS();

int main() {
    InitParam();
    InitRootFolder();

    v_mkdir("ed");
    v_mkdir("ed4");
    v_mkdir("ed5");
    v_mkdir("ed445");
    v_open("ed");
    PrintInodeFromMemory(75);

    // for (int i = 0; i < GROUP_NUMBER;++i) {
    //     SetBlock(i);
    //     PrintVSFS();
    // }

    PrintInodeFromMemory(rootInode);


    free(disk);
    return 0;
}

void InitParam() {
    disk = _ca(diskSize);
    blockSize = CalcSize(diskSize, GROUP_NUMBER);
    chunkSize = CHUNK_SIZE;
    numberChunk = blockSize / chunkSize - 8;
    numberInode = CalcSize(chunkSize * 6, inodeSize);

    pf("block size: %d\nchunk size: %d\nnumber chunk: %d\nnumberInode: %d\n\n", blockSize, chunkSize, numberChunk, numberInode);

    for (int i = 0; i < GROUP_NUMBER;++i) {
        Super* temp = (Super*)(disk + blockSize * i);

        temp->disk = (char*)temp;
        temp->chunkSize = chunkSize;
        temp->diskSize = blockSize;
        temp->numberInode = numberInode;
        temp->numberChunk = numberChunk;

        temp->inodeBitmapChunk = temp->disk + chunkSize;
        temp->dataBitmapChunk = temp->inodeBitmapChunk + CalcSize(chunkSize, 2);
        temp->inodeChunk = (Inode*)(temp->disk + 2 * chunkSize);
        temp->dataChunk = temp->disk + 8 * chunkSize;

        vss[i] = temp;
    }
    for (int i = 0; i < GROUP_NUMBER; ++i) PrintSuper(vss[i]);
}

void PrintSuper(Super* temp) {
    printf("Disk: %p\n", temp->disk);
    pf("Chunk Size: %-10lld Disk Size: %-10lld Inode number: %-10lld Data number: %-10lld\n", temp->chunkSize, temp->diskSize, temp->numberInode, temp->numberChunk);
    pf("inode bm add: %-30p inode add: %-30p\n", temp->inodeBitmapChunk, temp->inodeChunk);
    pf("data  bm add: %-30p data  add: %-30p\n", temp->dataBitmapChunk, temp->dataChunk);
    pf("\n");
}

void InitRootFolder() {
    Inode root = CreateInode(_DIRECTORY);
    rootInode = root.id;
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = ".." });
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = "." });
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = "root" });
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = "" });

    UpdateInode(&root);
}

Address FindFreeChunk() {
    SetBlock(blockIndex + 1);
    return (Address) { .block = blockIndex, .offset = GetFreeCell(dataBitmapChunk, numberChunk) };
}
Address FindFreeInode(int type) {
    if (type == _DIRECTORY) SetBlock(blockIndex + 1);
    return (Address) { .block = blockIndex, .offset = GetFreeCell(inodeBitmapChunk, numberInode) };
}

Inode CreateInode(int type) {
    Address id = FindFreeInode(type);
    if (id.offset >= numberInode || id.offset == FAIL) {
        printf("Invalid inode.\n");
        return (Inode) { .id = -1 };
    }

    Inode inode = { .id = GetRealIndex(id,numberInode),.type = type,.link = 0,.size = 0 };
    for (int i = 0; i < MaxPointers; ++i) inode.blocks[i] = EMPTY;

    UpdateInode(&inode);
    // PrintInode(&inode);
    // PrintInodeFromMemory(inode.id);
    return inode;
}
void UpdateInode(Inode* inode) {
    Address inodeAddress = GetAddress(inode->id, numberInode);
    SetBlock(inodeAddress.block);
    memcpy(&inodeChunk[inodeAddress.offset], inode, sizeof(Inode));
}
int UpdateInodeData(Inode* inode, char* data, size_t size) {
    Address add = GetAddress(inode->id, numberInode);
    SetBlock(add.block);

    if (CheckCell(inodeBitmapChunk, numberInode, add.offset) == 0) return FAIL;

    int sizeRemain = max(CalcSize(inode->size, chunkSize) * chunkSize - inode->size, 0);
    size_t temp = size;
    size_t offset = 0;
    int pointer = GetFreePointer(inode);

    if (sizeRemain > 0 && pointer > 0) {
        size_t writeSize = min(sizeRemain, size);
        add = GetAddress(inode->blocks[pointer - 1], numberChunk);
        SetBlock(add.block);

        memcpy(dataChunk + add.offset * chunkSize + inode->size % chunkSize, data, writeSize);
        inode->size += writeSize; temp -= writeSize; offset += writeSize;
    }

    if (pointer == MaxPointers) {
        printf("Cant not find free pointer\n");
        return FAIL;
    }

    while (temp > 0) {
        pointer = GetFreePointer(inode);
        if (pointer >= MaxPointers) {
            printf("No more pointer.\n");
            return FAIL;
        }

        Address freeChunk = FindFreeChunk();
        if (freeChunk.offset == FAIL) {
            printf("Run out of free chunk!!!\n");
            return FAIL;
        }
        size_t writeSize = min(chunkSize, size);
        memcpy(dataChunk + freeChunk.offset * chunkSize, data + offset, writeSize);

        temp -= writeSize; offset += writeSize; inode->size += writeSize;
        inode->blocks[pointer] = GetRealIndex(freeChunk, numberChunk);
    }
    return SUCCESS;
}
char* ReadInodeData(char _dst[], Inode* inode) {
    Address add = GetAddress(inode->id, numberInode);
    SetBlock(add.block);
    if (CheckCell(inodeBitmapChunk, numberInode, add.offset) == 0) return NULL;

    for (int i = 0, offset = 0, temp = inode->size; inode->blocks[i] != EMPTY && i < MaxPointers;++i) {
        Address add = GetAddress(inode->blocks[i], numberChunk);
        SetBlock(add.block);

        size_t readSize = min(chunkSize, temp);
        memcpy(_dst + offset, dataChunk + add.offset * chunkSize, readSize);
        offset += readSize; temp -= readSize;
    }
    return _dst;
}
int GetFreePointer(Inode* inode) {
    for (int i = 0; i < MaxPointers - 1; ++i) if (inode->blocks[i] == EMPTY) return i;
    return MaxPointers - 1;
}
void PrintInodeFromMemory(int inodeIndex) {
    Address address = GetAddress(inodeIndex, numberInode);
    SetBlock(address.block);
    Inode inode = inodeChunk[address.offset];
    PrintInode(&inode);
}
void PrintInode(Inode* inode) {
    printf("\n\n");

    printf("ID: %-3d TYPE: %-10s LINK: %-5d SIZE: %d\n", inode->id, inode->type == _FILE ? "File" : "Directory", inode->link, inode->size);

    for (int i = 0; i < MaxPointers; ++i) printf("%2d ", inode->blocks[i]);

    if (inode->type == _FILE) return;
    printf("\n");

    int len = CalcSize(inode->size, inodeTableSize);
    InodeTable tables[CalcSize(inode->size, inodeTableSize)]; ReadInodeData((char*)tables, inode);
    for (int i = 0; i < len; ++i) printf("| %-5d %30s |\n", tables[i].id, tables[i].name);
}
void AddLinkToInode(Inode* inode, InodeTable table) {
    Address add = GetAddress(inode->id, numberInode);
    SetBlock(add.block);
    if (CheckCell(inodeBitmapChunk, numberInode, add.offset) == 0) {
        printf("Inode doesnt exist!!!\n");
        return;
    }

    int result = UpdateInodeData(inode, (char*)&table, inodeTableSize);
    if (result == FAIL) {
        printf("Couldnt add link to folder, run out of space.\n");
        return;
    }
    if (inode->id == table.id) ++inode->link;
    else {
        add = GetAddress(table.id, numberInode);
        SetBlock(add.block);
        ++inodeChunk[add.offset].link;
    }
}

int IsThisFileInFolder(Inode* inode, char* path, int type) {
    int len = CalcSize(inode->size, inodeTableSize);
    InodeTable table[len]; ReadInodeData((char*)table, inode);

    for (int i = 0; i < len; ++i) {
        if (strcmp(table[i].name, path)) continue;
        if (inodeChunk[table[i].id].type == type || type == _ANYTYPE) return table[i].id;
    }

    return FAIL;
}
int IsPathExist(char** p, int level, int type) {
    int inodeID = rootInode;

    for (int i = 0; p[i + level] != NULL; ++i) {
        Address add = GetAddress(inodeID, numberInode);
        SetBlock(add.block);
        Inode inode = inodeChunk[add.offset];
        inodeID = IsThisFileInFolder(&inode, p[i], p[i + level + 1] == NULL ? type : _ANYTYPE);
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
    Inode child = CreateInode(_DIRECTORY);
    if (child.id == FAIL) {
        pf("Runnout of inode.\n");
        return;
    }
    // PrintInode(&child);
    Address parentAdd = GetAddress(inodeParentID, numberInode);
    // Address childAdd = GetAddress(child.id, numberInode);

    SetBlock(parentAdd.block);
    Inode parent = inodeChunk[parentAdd.offset];
    InodeTable childTable; childTable.id = child.id;
    strcpy(childTable.name, fileName);
    // PrintInodeFromMemory(child.id);

    AddLinkToInode(&child, (InodeTable) { .id = parent.id, .name = ".." });
    AddLinkToInode(&child, (InodeTable) { .id = child.id, .name = "." });

    AddLinkToInode(&parent, childTable);

    UpdateInode(&child);
    UpdateInode(&parent);

    FreeMem(splitStr);
    free(fileName);
}
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

    Inode child = CreateInode(_FILE);

    Address parentAdd = GetAddress(inodeParentID, numberInode);
    SetBlock(parentAdd.block);
    Inode parent = inodeChunk[parentAdd.offset];

    InodeTable childTable;
    childTable.id = child.id; strcpy(childTable.name, fileName);

    UpdateInode(&child);

    AddLinkToInode(&parent, childTable);
    UpdateInode(&parent);

    FreeMem(splitStr);
    free(fileName);
}
int v_write(char* path, char* content, size_t size) {
    char** splitStr = SplitString(path, "/");
    int dst = IsPathExist(splitStr, 0, _FILE);

    if (dst == FAIL) {
        printf("File doesnt exist: %s\n", path);
        FreeMem(splitStr);
        return FAIL;
    }
    Address add = GetAddress(dst, numberInode);
    SetBlock(add.block);
    Inode inode = inodeChunk[add.offset];
    int success = UpdateInodeData(&inode, content, size);
    UpdateInode(&inode);

    FreeMem(splitStr);
    return success;
}
void PrintFSS() {

}
void PrintVSFS() {
    pf("\nVSFS data:\n");

    pf("\ninode bitmap: ");
    _Print(inodeBitmapChunk, CalcSize(numberInode, 8));

    pf("\ndata bitmap: ");
    _Print(dataBitmapChunk, CalcSize(numberChunk, 8));

    pf("\nInode: \n");
    for (int i = 0; i < numberInode; ++i) {
        if (CheckCell(inodeBitmapChunk, numberInode, i) == 0) continue;
        Inode inode = inodeChunk[i];
        printf("\n[ id: %-4d type: %-5s blocks: [ ", inode.id, inode.type == _FILE ? "file" : "dir");
        for (int j = 0; inode.blocks[j] != EMPTY && j < MaxPointers; ++j) printf("%d ", inode.blocks[j]);
        printf("],");
    }

    pf("\n\nData: \n");
    for (int i = 0; i < numberInode; ++i) {
        if (CheckCell(inodeBitmapChunk, numberInode, i) == 0) continue;
        Inode inode = inodeChunk[i];
        if (inode.type == _FILE) {
            pf("Inode %d: [...]\n", i);
            // char dest[inode.size]; ReadInode(dest, &inode);
            // _Print(dest, min(inode.size, 32));
            // pf("]\n");
            continue;
        }
        size_t len = CalcSize(inode.size, inodeTableSize);
        InodeTable* tables = _ma(inode.size);
        ReadInodeData((char*)tables, &inode);

        pf("Inode %d: [ ", i);
        for (int j = 0; j < len;++j) pf("(%d, %s), ", tables[j].id, tables[j].name);

        pf("]\n");
        free(tables);
    }
}
#include "types.h"
#include "utils.h"
#include "bitmap.h"


#define GROUP_NUMBER                                16


// Add 2nd indirect pointer

char* disk; // 4kb

size_t diskSize = 256 * 1024;                           // 4KB

size_t inodeSize = sizeof(Inode);
size_t inodeTableSize = sizeof(InodeTable);
size_t size_tSize = sizeof(size_t);
size_t intSize = sizeof(int);
size_t charSize = sizeof(char);

size_t blockSize;
size_t chunkSize;
size_t numberChunk;
size_t numberInode;

int blockIndex = 0;
int rootBlock = 0;
int rootInode;                                              // index of root inode stored in file system (usually 0)

char* data;
char* inodeBitmapChunk, * dataBitmapChunk, * dataChunk;     // the start address of inode bitmap, data bitmap, data chunk
Inode* inodeChunk;                                          // the start address of inode's chunk

Super* vss[GROUP_NUMBER];

void PrintSuper(Super* temp) {
    printf("Disk: %p\n", temp->disk);
    pf("Chunk Size: %-10lld Disk Size: %-10lld Inode number: %-10lld Data number: %-10lld\n", temp->chunkSize, temp->diskSize, temp->numberInode, temp->numberChunk);
    pf("inode bm add: %-30p inode add: %-30p\n", temp->inodeBitmapChunk, temp->inodeChunk);
    pf("data  bm add: %-30p data  add: %-30p\n", temp->dataBitmapChunk, temp->dataChunk);
    pf("\n");
}

void InitParam() {
    disk = _ca(diskSize);
    blockSize = CalcSize(diskSize, GROUP_NUMBER);
    chunkSize = 256;
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

int GetBlockIndex(int index, int size) { return index / size; }
int GetRealIndex(int indexBlock, int sizeItem, int indexItem) { return indexBlock * sizeItem + indexItem % sizeItem; }
int GetLogicIndex(int realIndex, int size) { return realIndex % size; }

Inode CreateInode(int type);
char* ReadInode(char* _dst, Inode* inode);
void UpdateInode(Inode* inode);
int UpdateInodeData(Inode* inode, char* data, size_t size);
void PrintInode(Inode* inode);
void PrintInodeInDisk(int inodeIndex);
int GetFreePointer(Inode* inode);
void AddLinkToInode(Inode* inode, InodeTable table);
int CheckInodeState(int inodeNumber);

void SetBlock(int index) {
    blockIndex = index % GROUP_NUMBER;
    Super* temp = vss[blockIndex];

    data = temp->disk;

    inodeBitmapChunk = temp->inodeBitmapChunk;
    dataBitmapChunk = temp->dataBitmapChunk;
    inodeChunk = temp->inodeChunk;
    dataChunk = temp->dataChunk;
}
void InitRootFolder() {
    SetBlock(rootBlock);

    Inode inode = CreateInode(_DIRECTORY);

    AddLinkToInode(&inode, (InodeTable) { .id = inode.id, .name = "." });
    AddLinkToInode(&inode, (InodeTable) { .id = inode.id, .name = ".." });
    AddLinkToInode(&inode, (InodeTable) { .id = inode.id, .name = "root" });
    AddLinkToInode(&inode, (InodeTable) { .id = inode.id, .name = "" });

    UpdateInode(&inode);
}


int main() {
    InitParam();
    InitRootFolder();

    PrintInodeInDisk(rootInode);




    free(disk);
    return 0;
}

Inode CreateInode(int type) {
    if (type != _DIRECTORY && type != _FILE && type != _ANYTYPE) {
        pf("Invalid type!!");
        return (Inode) {};
    }
    int id = GetFreeCell(inodeBitmapChunk, numberInode);

    Inode inode = { .id = GetRealIndex(blockIndex,numberInode,id),.link = 0,.size = 0,.type = type };
    for (int i = 0; i < MaxPointers; ++i) inode.blocks[i] = EMPTY;

    UpdateInode(&inode);
    return inode;
}

char* ReadInode(char* _dst, Inode* inode) {
    SetBlock(GetBlockIndex(inode->id, numberInode));

    for (int i = 0, offset = 0, temp = inode->size; inode->blocks[i] != EMPTY && i < MaxPointers - 1; ++i) {
        size_t readSize = min(chunkSize, temp);

        int chunkIndex = inode->blocks[i];
        int logicChunkIndex = GetLogicIndex(chunkIndex, numberChunk);
        SetBlock(GetBlockIndex(chunkIndex, numberChunk));

        memcpy(_dst + offset, dataChunk + logicChunkIndex * chunkSize, readSize);
        offset += readSize; temp -= readSize;
    }
    return _dst;
}

void UpdateInode(Inode* inode) {
    SetBlock(GetBlockIndex(inode->id, numberInode));

    int logicIndex = GetLogicIndex(inode->id, numberInode);
    if (!CheckInodeState(inode->id)) return;
    memcpy(&inodeChunk[logicIndex], inode, inodeSize);
}

int* WriteData(int* dst, char* data, size_t size) {
    size_t temp = size, offset = 0, k = 0, c = 0;

    while (temp > 0) {
        int chunk = GetFreeCell(dataBitmapChunk, numberChunk);

        if (chunk == FAIL) for (int i = 0; i < GROUP_NUMBER; ++i) {
            SetBlock(blockIndex + 1);
            chunk = GetFreeCell(dataBitmapChunk, numberChunk);
            if (chunk != FAIL) break;
        }

        if (chunk == FAIL) {
            pf("Disk is full!\n");
            dst[k] = EMPTY;
            return dst;
        }

        size_t writeSize = min(chunkSize, size);
        memcpy(dataChunk + chunk * chunkSize, data + offset, writeSize);
        temp -= writeSize; offset += writeSize;
        dst[k++] = GetRealIndex(blockIndex, numberChunk, chunk);

        if (c++ % 12 == 0) SetBlock(blockIndex + 1);
    }
    dst[k] = EMPTY;

    return dst;
}

int GetArrSize(int* arr) {
    int i = 0;
    for (; arr[i] != EMPTY; ++i);
    return i;
}

int UpdateInodeData(Inode* inode, char* data, size_t size) {

}

void PrintInode(Inode* inode) {
    pf("\n\n");
    pf("ID: %-3d TYPE: %-10s LINK: %-5d SIZE: %d\n", inode->id, inode->type == _FILE ? "File" : "Directory", inode->link, inode->size);
    for (int i = 0; i < MaxPointers; ++i) pf("%2d ", inode->blocks[i]);

    if (inode->type == _FILE) return;
    pf("\n");

    int len = CalcSize(inode->size, inodeTableSize);
    if (len == 0) return;

    InodeTable tables[CalcSize(inode->size, inodeTableSize)]; ReadInode((char*)tables, inode);
    for (int i = 0; i < len; ++i) pf("| %-5d %30s |\n", tables[i].id, tables[i].name);
}

void PrintInodeInDisk(int inodeIndex) {
    int logicIndex = GetLogicIndex(inodeIndex, numberInode);
    if (!CheckInodeState(inodeIndex)) return;

    SetBlock(GetBlockIndex(inodeIndex, numberInode));
    PrintInode(&inodeChunk[logicIndex]);
}

int GetFreePointer(Inode* inode) {
    for (int i = 0; i < MaxPointers - 1; ++i) if (inode->blocks[i] == EMPTY) return i;
    return MaxPointers;
}

void AddLinkToInode(Inode* inode, InodeTable table) {
    if (!CheckInodeState(inode->id)) return;

    int result = UpdateInodeData(inode, (char*)&table, inodeTableSize);
    if (result == FAIL) {
        pf("Couldnt add link to folder, run out of space.\n");
        return;
    }

    if (inode->id == table.id) {
        ++inode->link;
        return;
    }

    SetBlock(GetBlockIndex(table.id, numberInode));
    ++inodeChunk[GetLogicIndex(table.id, numberInode)].link;
}

int CheckInodeState(int inodeIndex) {
    SetBlock(GetBlockIndex(inodeIndex, numberInode));

    int logicIndex = GetLogicIndex(inodeIndex, numberInode);
    if (CheckCell(inodeBitmapChunk, numberInode, logicIndex) == 0) {
        pf("Inode doesnt exists!!! Inode number: %d.\n", inodeIndex);
        return 0;
    }
    return 1;
}
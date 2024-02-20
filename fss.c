#include "types.h"
#include "utils.h"
#include "bitmap.h"


#define GROUP_NUMBER                                16


char* disk; // 4kb

size_t diskSize = 256 * 1024;                           // 4KB

size_t inodeSize = sizeof(Inode);
size_t inodeTableSize = sizeof(InodeTable);
size_t size_tSize = sizeof(size_t);
size_t intSize = sizeof(int);
size_t charSize = sizeof(char);

int indexBlock = 0;
int rootInode;                                              // index of root inode stored in file system (usually 0)

char* data;
size_t dataSize, chunkSize, numberInode, numberChunk;       // Number of Inodes and Data Chunks have in disks
char* inodeBitmapChunk, * dataBitmapChunk, * dataChunk;     // the start address of inode bitmap, data bitmap, data chunk
Inode* inodeChunk;                                          // the start address of inode's chunk

Super* vss[GROUP_NUMBER];

void PrintSuper(Super* temp) {
    printf("Disk: %p\n", temp->disk);
    pf("Chunk Size: %5lld Disk Size: %-10lld Inode number: %-10lld Data number: %-10lld\n", temp->chunkSize, temp->diskSize, temp->numberInode, temp->numberChunk);
    pf("inode bm add: %-30p inode add: %-30p\n", temp->inodeBitmapChunk, temp->inodeChunk);
    pf("data  bm add: %-30p data  add: %-30p\n", temp->dataBitmapChunk, temp->dataChunk);
    pf("\n");
}

void InitParam() {
    disk = _ma(diskSize);
    size_t blockSize = CalcSize(diskSize, GROUP_NUMBER);
    size_t chunkSize = 256;
    size_t numberChunk = blockSize / chunkSize;
    size_t numberInode = CalcSize(chunkSize * 6, inodeSize);

    pf("block size: %d\nchunk size: %d\nnumber chunk: %d\nnumberInode: %d\n\n", blockSize, chunkSize, numberChunk, numberInode);

    for (int i = 0; i < GROUP_NUMBER;++i) {
        Super* temp = (Super*)(disk + blockSize * i);

        temp->disk = (char*)temp;
        temp->chunkSize = chunkSize;
        temp->diskSize = blockSize;
        temp->numberInode = numberInode;
        temp->numberChunk = numberChunk - 8;

        temp->inodeBitmapChunk = temp->disk + chunkSize;
        temp->dataBitmapChunk = temp->inodeBitmapChunk + CalcSize(chunkSize, 2);
        temp->inodeChunk = (Inode*)temp->disk + 2 * chunkSize;
        temp->dataChunk = temp->disk + 8 * chunkSize;

        vss[i] = temp;
    }
    // for (int i = 0; i < GROUP_NUMBER; ++i) PrintSuper(vss[i]);
}

int GetBlockIndex(int index, int size) { return index / size; }
int GetRealIndex(int block, int size, int index) { return block * size + index; }
int GetLogicIndex(int realIndex, int size) { return realIndex % size; }

Inode CreateInode(int type);
char* ReadInode(char* _dst, Inode* inode);
void UpdateInode(Inode* inode);
int UpdateInodeData(Inode* inode, char* data, size_t size);
void PrintInode(Inode* inode);
void PrintInodeInDisk(int inodeNum);
int GetFreePointer(Inode* inode);
void AddLinkToInode(Inode* inode, InodeTable table);

void SetDisk(int index) {
    Super* temp = vss[index];

    data = temp->disk;
    dataSize = temp->diskSize;
    chunkSize = temp->chunkSize;
    numberInode = temp->numberInode;
    numberChunk = temp->numberChunk;

    inodeBitmapChunk = temp->inodeBitmapChunk;
    dataBitmapChunk = temp->dataBitmapChunk;
    inodeChunk = temp->inodeChunk;
    dataChunk = temp->dataChunk;
}
void InitFolder() {
    SetDisk(indexBlock);






}


int main() {
    InitParam();
    InitFolder();


    free(disk);
    return 0;
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

char* ReadInode(char* _dst, Inode* inode) {
    for (int i = 0, offset = 0, temp = inode->size; inode->blocks[i] != EMPTY;++i) {
        size_t readSize = min(chunkSize, temp);
        memcpy(_dst + offset, dataChunk + inode->blocks[i] * chunkSize, readSize);
        offset += readSize; temp -= readSize;
    }
    return _dst;
}

void UpdateInode(Inode* inode) {
    if (CheckCell(inodeBitmapChunk, numberInode, inode->id) == 0) {
        printf("Inode isnt ocupied yet!\n");
        return;
    }
    memcpy(&inodeChunk[inode->id], inode, inodeSize);
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

int GetFreePointer(Inode* inode) {
    for (int i = 0; i < MaxPointers; ++i) if (inode->blocks[i] == EMPTY) return i;
    return MaxPointers;
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


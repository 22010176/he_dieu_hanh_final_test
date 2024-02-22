#include "types.h"
#include "utils.h"
#include "bitmap.h"

#define GROUP_NUMBER                                16

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
*/

char* disk;

size_t diskSize = 256 * 1024;                           // 4KB

// Constants
size_t inodeSize = sizeof(Inode);
size_t inodeTableSize = sizeof(InodeTable);
size_t size_tSize = sizeof(size_t);
size_t intSize = sizeof(int);
size_t charSize = sizeof(char);


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
    blockIndex = bIndex % GROUP_NUMBER;

    Super* temp = vss[blockIndex];
    inodeBitmapChunk = temp->inodeBitmapChunk;
    dataBitmapChunk = temp->dataBitmapChunk;
    inodeChunk = temp->inodeChunk;
    dataChunk = temp->dataChunk;
}


void InitParam();
void InitRootFolder();

size_t FindFreeInode(int type);
Address FindFreeChunk();

int* WriteChunk(int dest[], char* data, size_t size);
int WriteToChunk(Address add, char* data, size_t size);
char* ReadChunk(char dest[], int* chunks, size_t size);

Inode CreateInode(int type);
void UpdateInode(Inode* inode);
void UpdateInodeData(Inode* inode, char* data, size_t size);
char* ReadInodeData();
void PrintInodeFromMemory(int inodeIndex);
void PrintInode(Inode* inode);

int IsThisFileInFolder(Inode* inode, char* path, int type);
int IsPathExist(char** p, int level, int type);

void v_mkdir(char* path);
void v_link(char* _dst, char* _src);
void v_unlink(char* dst);
void v_open(char* path);
void v_write(char* path, char* content, size_t size);


int main() {
    InitParam();
    InitRootFolder();




    free(disk);
    return 0;
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

void PrintSuper(Super* temp) {
    printf("Disk: %p\n", temp->disk);
    pf("Chunk Size: %-10lld Disk Size: %-10lld Inode number: %-10lld Data number: %-10lld\n", temp->chunkSize, temp->diskSize, temp->numberInode, temp->numberChunk);
    pf("inode bm add: %-30p inode add: %-30p\n", temp->inodeBitmapChunk, temp->inodeChunk);
    pf("data  bm add: %-30p data  add: %-30p\n", temp->dataBitmapChunk, temp->dataChunk);
    pf("\n");
}

void InitRootFolder() {
    Inode inode = CreateInode(_DIRECTORY);

}

Address FindFreeChunk() {
    SetBlock(blockIndex + 1);
    return (Address) { .block = blockIndex, .offset = GetFreeCell(dataBitmapChunk, numberChunk) };
}
int* WriteChunk(int dest[], char* data, size_t size) {
    size_t temp = size, offset = 0, k = 0;
    while (temp > 0) {
        Address chunk = FindFreeChunk();
        size_t writeSize = min(chunkSize, temp);
        WriteToChunk(chunk, data + offset, writeSize);

        temp -= writeSize; offset += writeSize;
        dest[k++] = GetRealIndex(chunk, numberChunk);
    }
    dest[k] = EMPTY;
    return dest;
}
int WriteToChunk(Address add, char* data, size_t size) {
    SetBlock(add.block);
    memcpy(dataChunk + add.offset * chunkSize, data, size);
    return SUCCESS;
}
char* ReadChunk(char dest[], int* chunks, size_t size) {
    size_t temp = size, offset = 0, k = 0;
    while (temp > 0) {
        Address address = GetAddress(chunks[k], numberChunk);
        size_t readSize = min(chunkSize, temp);
        SetBlock(address.block);

        memcpy(dest + offset, dataChunk + chunkSize * address.offset, readSize);
        temp -= readSize; offset += readSize; ++k;
    }
    return dest;
}

size_t FindFreeInode(int type) {

}
Inode CreateInode(int type) {
    Inode inode = { .id = 0, .link = 0,.size = 0,.type = type };
    for (int i = 0; i < MaxPointers; ++i) inode.blocks[i] = EMPTY;
    return inode;
}
void UpdateInode(Inode* inode) {
    Address inodeAddress = GetAddress(inode->id, numberInode);
    SetBlock(inodeAddress.block);
    memcpy(inodeChunk + inodeSize * inodeAddress.offset, inode, inodeSize);
}
void UpdateInodeData(Inode* inode, char* data, size_t size) {

}
char* ReadInodeData() {

}
void PrintInodeFromMemory(int inodeIndex) {

}
void PrintInode(Inode* inode) {

}

int IsThisFileInFolder(Inode* inode, char* path, int type) {

}
int IsPathExist(char** p, int level, int type) {

}

void v_mkdir(char* path) {

}
void v_link(char* _dst, char* _src) {

}
void v_unlink(char* dst) {

}
void v_open(char* path) {

}
void v_write(char* path, char* content, size_t size) {

}
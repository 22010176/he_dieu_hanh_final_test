#include "Super.h"

Super::Super(uint8_t data[24]) {
    size_t* arr = (size_t*)data;

    this->diskSize = arr[0];
    this->inodeNum = arr[1];
    this->chunkSize = arr[2];
}
Super::Super(size_t diskSize, size_t inodeNum, size_t chunkSize) : diskSize{ diskSize }, inodeNum{ inodeNum }, chunkSize{ chunkSize } {}

size_t Super::GetDiskSize() const { return this->diskSize; }
size_t Super::GetInodeNum() const { return this->inodeNum; }
size_t Super::GetChunkSize() const { return this->chunkSize; }
void  Super::Print() const {
    std::cout << std::endl << "Disk Size: " << this->diskSize << std::endl;
    std::cout << "Inode number: " << this->inodeNum << std::endl;
    std::cout << "Chunk size: " << this->chunkSize << std::endl;
    std::cout << std::endl;
}

uint8_t* Super::ExportData() { return ExportData(new uint8_t[sizeof(Super)]); }
uint8_t* Super::ExportData(uint8_t* _dst) {
    memcpy(_dst, this, sizeof(Super));
    return _dst;
}
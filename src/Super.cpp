#include "Super.h"

Super::Super(uint8_t data[24]) {
    DEBUG(size_t * arr = (size_t*)data);

    DEBUG(this->diskSize = arr[0]);
    DEBUG(this->inodeNum = arr[1]);
    DEBUG(this->chunkSize = arr[2]);
}
Super::Super(size_t diskSize, size_t inodeNum, size_t chunkSize) : diskSize{ diskSize }, inodeNum{ inodeNum }, chunkSize{ chunkSize } {}

size_t Super::GetDiskSize() const { DEBUG(return this->diskSize); }
size_t Super::GetInodeNum() const { DEBUG(return this->inodeNum); }
size_t Super::GetChunkSize() const { DEBUG(return this->chunkSize); }
void  Super::Print() const {
    DEBUG(std::cout << std::endl << "Disk Size: " << this->diskSize << std::endl);
    DEBUG(std::cout << "Inode number: " << this->inodeNum << std::endl);
    DEBUG(std::cout << "Chunk size: " << this->chunkSize << std::endl);
    DEBUG(std::cout << std::endl);
}

uint8_t* Super::ExportData() { DEBUG(return ExportData(new uint8_t[sizeof(Super)])); }
uint8_t* Super::ExportData(uint8_t* _dst) {
    DEBUG(memcpy(_dst, this, sizeof(Super)));
    DEBUG(return _dst);
}
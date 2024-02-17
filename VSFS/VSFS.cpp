#include "VSFS.h"

VSFS::VSFS(Super super) {
    size_t diskSize = super.GetDiskSize();
    size_t inodeNum = super.GetInodeNum();
    size_t chunkSize = super.GetChunkSize();


    SetupParameter(diskSize, inodeNum, chunkSize);
}
VSFS::VSFS(uint8_t data[24]) {
    size_t* temp = (size_t*)data;

    size_t diskSize = temp[0];
    size_t inodeNum = temp[1];
    size_t chunkSize = temp[2];

    SetupParameter(diskSize, inodeNum, chunkSize);
}

inline void VSFS::SetupParameter(size_t diskSize, size_t inodeNum, size_t chunkSize) {
    if (inodeNum * sizeof(Inode) > diskSize) {
        std::cout << "Invalid Inode size" << std::endl;
        exit(-1);
    }
    else if (diskSize < chunkSize) {
        std::cout << "Invalid chunkSize size" << std::endl;
        exit(-1);
    }
    size_t InodeChunkNum =
        CalcSize(inodeNum * sizeof(Inode), chunkSize) +
        CalcSize(CalcSize(inodeNum, 8), chunkSize);

    size_t totalNotDataChunk = (1 + InodeChunkNum) * chunkSize;
    size_t dataOffset = totalNotDataChunk * chunkSize;

    this->disk = new uint8_t[diskSize];
    this->indoes = StorageManagement(disk + chunkSize, InodeChunkNum * chunkSize, sizeof(Inode));
    this->datas = StorageManagement(disk + dataOffset, diskSize - dataOffset, chunkSize);
}
VSFS::~VSFS() { delete[] disk; }

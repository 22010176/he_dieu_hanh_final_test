#include "VSFS.h"

VSFS::VSFS(Super super) {
    this->diskSize = super.GetDiskSize();
    this->inodeNum = super.GetInodeNum();
    this->chunkSize = super.GetChunkSize();


    SetupParameter();
}
VSFS::VSFS(uint8_t data[24]) {
    size_t* temp = (size_t*)data;

    this->diskSize = temp[0];
    this->inodeNum = temp[1];
    this->chunkSize = temp[2];

    SetupParameter();
}
VSFS::~VSFS() { delete[] disk; }

void VSFS::SetupParameter() {
    if (inodeNum * sizeof(Inode) > diskSize) {
        std::cout << "Invalid Inode size" << std::endl;
        exit(-1);
    }
    else if (diskSize < chunkSize) {
        std::cout << "Invalid chunkSize size" << std::endl;
        exit(-1);
    }

    size_t inodeChunkNum =
        CalcSize(inodeNum * sizeof(Inode), chunkSize) +
        CalcSize(CalcSize(inodeNum, 8), chunkSize);

    size_t totalNotDataChunk = 1 + inodeChunkNum;
    size_t dataOffset = totalNotDataChunk * chunkSize;

    std::cout << diskSize << " " << dataOffset << " " << totalNotDataChunk << " " << inodeChunkNum << std::endl;
    if (dataOffset > diskSize) {
        std::cout << "Not enough place for data" << std::endl;
        exit(-1);
    }

    this->disk = new uint8_t[diskSize];
    this->indoes = StorageManagement(disk + chunkSize, inodeChunkNum * chunkSize, sizeof(Inode));
    this->datas = StorageManagement(disk + dataOffset, diskSize - dataOffset, chunkSize);
}

StorageManagement VSFS::GetInodes() const { return indoes; }
StorageManagement VSFS::GetDatas() const { return datas; }

size_t VSFS::GetDiskSize() const { return this->diskSize; }
size_t VSFS::GetInodeNumber() const { return this->inodeNum; }
size_t VSFS::GetChunkSize() const { return this->chunkSize; }

void VSFS::InitFileSystem() {

}
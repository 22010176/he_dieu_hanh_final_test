#include "VSFS.h"

VSFS::VSFS(Super super) {
    this->diskSize = super.GetDiskSize();
    this->inodeNum = super.GetInodeNum();
    this->chunkSize = super.GetChunkSize();

    std::cout << diskSize << " " << inodeNum << " " << chunkSize << std::endl;;
    SetupParameter();
    InitFileSystem();
}
VSFS::VSFS(uint8_t data[24]) {
    size_t* temp = (size_t*)data;

    this->diskSize = temp[0];
    this->inodeNum = temp[1];
    this->chunkSize = temp[2];

    SetupParameter();

    InitFileSystem();
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

    // std::cout << diskSize << " " << dataOffset << " " << totalNotDataChunk << " " << inodeChunkNum << std::endl;
    if (dataOffset > diskSize) {
        std::cout << "Not enough place for data" << std::endl;
        exit(-1);
    }

    this->disk = new uint8_t[diskSize];
    this->indoes = StorageManagement(disk + chunkSize, inodeChunkNum * chunkSize, sizeof(Inode) + 4);
    this->datas = StorageManagement(disk + dataOffset, diskSize - dataOffset, chunkSize);

    std::cout << "Inode" << std::endl;
    this->indoes.PrintS();

    std::cout << "Data" << std::endl;
    this->datas.PrintS();
}

StorageManagement VSFS::GetInodes() const { return indoes; }
StorageManagement VSFS::GetDatas() const { return datas; }

size_t VSFS::GetDiskSize() const { return this->diskSize; }
size_t VSFS::GetInodeNumber() const { return this->inodeNum; }
size_t VSFS::GetChunkSize() const { return this->chunkSize; }

void VSFS::InitFileSystem() {
    Inode inode(indoes.GetBitmap().GetFreeCell(), Inode::DIRECTORY);
    rootInode = inode.GetId();

    inode.SetLink(4);
    inode.SetSize(InodeTable::GetExportSize() * 2);

    InodeTable table[]{ {rootInode, "."}, {rootInode,".."}, {rootInode,""},{rootInode,"root"} };

    uint8_t tables[InodeTable::GetExportSize() * 4];
    for (int i = 0; i < 4; ++i) table[i].ExportData(tables + i * InodeTable::GetExportSize());

    std::vector<uint32_t> chunks = datas.WriteM(tables, sizeof(tables));
    for (uint32_t c : chunks) inode.AddPointer(c);

    uint8_t result[Inode::GetExportSize()];

    uint32_t inodeI = indoes.WriteS(rootInode, inode.ExportData(result), sizeof(result));

    indoes.PrintS(inodeI);
    datas.PrintM(chunks);
}
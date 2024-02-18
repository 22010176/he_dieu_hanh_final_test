#include "VSFS.h"

#include "../src/Test.h"

VSFS::VSFS(Super super) {
    DEBUG(this->diskSize = super.GetDiskSize());
    DEBUG(this->inodeNum = super.GetInodeNum());
    DEBUG(this->chunkSize = super.GetChunkSize());

    DEBUG(std::cout << diskSize << " " << inodeNum << " " << chunkSize << std::endl);
    DEBUG(SetupParameter());
    DEBUG(InitFileSystem());
}
VSFS::VSFS(uint8_t data[24]) {
    DEBUG(size_t * temp = (size_t*)data);

    DEBUG(this->diskSize = temp[0]);
    DEBUG(this->inodeNum = temp[1]);
    DEBUG(this->chunkSize = temp[2]);

    DEBUG(SetupParameter());

    DEBUG(InitFileSystem());
}
VSFS::~VSFS() { DEBUG(delete[] disk); }

void VSFS::SetupParameter() {
    DEBUG(if (inodeNum * sizeof(Inode) > diskSize)) {
        DEBUG(std::cout << "Invalid Inode size" << std::endl);
        DEBUG(exit(-1));
    }
    else if (DEBUG(diskSize < chunkSize)) {
        DEBUG(std::cout << "Invalid chunkSize size" << std::endl);
        DEBUG(exit(-1));
    }

    DEBUG(size_t inodeChunkNum =
        CalcSize(inodeNum * sizeof(Inode), chunkSize) +
        CalcSize(CalcSize(inodeNum, 8), chunkSize));

    DEBUG(size_t totalNotDataChunk = 1 + inodeChunkNum);
    DEBUG(size_t dataOffset = totalNotDataChunk * chunkSize);

    // std::cout << diskSize << " " << dataOffset << " " << totalNotDataChunk << " " << inodeChunkNum << std::endl;
    DEBUG(if (dataOffset > diskSize)) {
        DEBUG(std::cout << "Not enough place for data" << std::endl);
        DEBUG(exit(-1));
    }

    DEBUG(this->disk = new uint8_t[diskSize]);
    DEBUG(this->indoes = new StorageManagement(disk + chunkSize, inodeChunkNum * chunkSize, sizeof(Inode) + 4));
    DEBUG(this->datas = new StorageManagement(disk + dataOffset, diskSize - dataOffset, chunkSize));

    Test::_StorageManagement3(this->indoes);
    Test::_StorageManagement3(this->datas);
}

StorageManagement* VSFS::GetInodes() const { DEBUG(return indoes); }
StorageManagement* VSFS::GetDatas() const { DEBUG(return datas); }

size_t VSFS::GetDiskSize() const { DEBUG(return this->diskSize); }
size_t VSFS::GetInodeNumber() const { DEBUG(return this->inodeNum); }
size_t VSFS::GetChunkSize() const { DEBUG(return this->chunkSize); }

void VSFS::InitFileSystem() {
    // DEBUG(Inode inode(indoes.GetBitmap().GetFreeCell(), Inode::DIRECTORY));
    // DEBUG(rootInode = inode.GetId());

    // DEBUG(inode.SetLink(4));
    // DEBUG(inode.SetSize(InodeTable::GetExportSize() * 2));

    // DEBUG(InodeTable table[]) { {rootInode, "."}, { rootInode,".." }, { rootInode,"" }, { rootInode,"root" } }

    // DEBUG(uint8_t tables[InodeTable::GetExportSize() * 4]);
    // DEBUG(for (int i = 0; i < 4; ++i)) {
    //     DEBUG(table[i].ExportData(tables + i * InodeTable::GetExportSize()));
    // }

    // DEBUG(std::vector<uint32_t> chunks = datas.WriteM(tables, sizeof(tables)));
    // DEBUG(for (uint32_t c : chunks) inode.AddPointer(c));

    // DEBUG(uint8_t result[Inode::GetExportSize()]);

    // DEBUG(uint32_t inodeI = indoes.WriteS(rootInode, inode.ExportData(result), sizeof(result)));

    // DEBUG(indoes.PrintS(inodeI));
    // DEBUG(datas.PrintM(chunks));
}
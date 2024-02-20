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
VSFS::~VSFS() {
    DEBUG(delete inodeData);
    DEBUG(delete dataData);
    DEBUG(delete[] disk);
}

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
        CalcSize(inodeNum * Inode::GetExportSize(), chunkSize) +
        CalcSize(CalcSize(inodeNum, 8), chunkSize));

    DEBUG(size_t totalNotDataChunk = 1 + inodeChunkNum);
    DEBUG(size_t dataOffset = totalNotDataChunk * chunkSize);

    // std::cout << diskSize << " " << dataOffset << " " << totalNotDataChunk << " " << inodeChunkNum << std::endl;
    DEBUG(if (dataOffset > diskSize)) {
        DEBUG(std::cout << "Not enough place for data" << std::endl);
        DEBUG(exit(-1));
    }

    DEBUG(this->disk = new uint8_t[diskSize]);
    DEBUG(this->inodeData = new StorageManagement(disk + chunkSize, inodeChunkNum * chunkSize, sizeof(Inode) + 4));
    DEBUG(this->dataData = new StorageManagement(disk + dataOffset, diskSize - dataOffset, chunkSize));

    Test::_StorageManagement3(this->inodeData);
    Test::_StorageManagement3(this->dataData);
}

StorageManagement* VSFS::GetInodes() const { DEBUG(return inodeData); }
StorageManagement* VSFS::GetDatas() const { DEBUG(return dataData); }

size_t VSFS::GetDiskSize() const { DEBUG(return this->diskSize); }
size_t VSFS::GetInodeNumber() const { DEBUG(return this->inodeNum); }
size_t VSFS::GetChunkSize() const { DEBUG(return this->chunkSize); }

bool VSFS::LinkingFolder(Inode* inode, InodeTable table) {
    DEBUG(if (table.id == inode->GetId())) {
        inode->SetLink(inode->GetLink() + 1);
        uint8_t res[InodeTable::GetExportSize()]; table.ExportData(res);
        UpdateInodeData(inode, res, InodeTable::GetExportSize());
        return true;
    }


    DEBUG(if (inodeData->GetBitmap().CheckCell(table.id) == 0) return false);
    DEBUG(uint8_t data[Inode::GetExportSize()]);
    // Update the _dst folder
    DEBUG(Inode _dst = Inode(inodeData->CopyS(data, table.id)));
    DEBUG(_dst.SetLink(_dst.GetLink() + 1));

    DEBUG(_dst.ExportData(data));
    DEBUG(inodeData->WriteS(table.id, data, Inode::GetExportSize()));
    inode->SetLink(inode->GetLink() + 1);
    // Update the inode table for src Inode
    // DEBUG(uint8_t _ta[InodeTable::GetExportSize()]);
    // DEBUG(table.ExportData(_ta));
    // DEBUG(UpdateInodeData(inode, _ta, table.GetExportSize()));
    // DEBUG(return true);
    return true;
}
bool VSFS::LinkingFolder(Inode* inode, uint32_t inodeId, const std::string& name) { DEBUG(return LinkingFolder(inode, InodeTable(inodeId, name))); }
Inode VSFS::CreateFolder(uint32_t type) { return Inode(this->inodeData->GetBitmap().GetFreeCell(), type); }
void VSFS::UpdateInodeData(Inode* inode, uint8_t* data, size_t size) {
    DEBUG(uint8_t freePointer = inode->FindFreePointer());
    if (freePointer == Inode::defaultEmptyPointer) return;
    size_t writedSize = 0;
    DEBUG(if (freePointer > 0)) {
        DEBUG(writedSize = dataData->UpdateS(inode->GetBlocks()[freePointer - 1], data, size));
        DEBUG(if (writedSize == size) return inode->SetSize(inode->GetSize() + size));
    }

    DEBUG(std::vector<uint32_t> chunks = dataData->WriteM(data + writedSize, size - writedSize));
    DEBUG(for (uint32_t chunk : chunks) inode->AddPointer(chunk));
    DEBUG(inode->SetSize(inode->GetSize() + size));
}

void VSFS::InitFileSystem() {
    DEBUG(Inode inode = CreateFolder(Inode::DIRECTORY));
    DEBUG(rootInode = inode.GetId());

    InodeTable tables[]{ InodeTable(rootInode,"."),InodeTable(rootInode,".."),InodeTable(rootInode,""),InodeTable(rootInode,"root") };
    DEBUG(for (InodeTable table : tables) LinkingFolder(&inode, table));

    DEBUG(uint8_t data[Inode::GetExportSize()]);
    DEBUG(inodeData->WriteS(rootInode, inode.ExportData(data), Inode::GetExportSize()));
    DEBUG(inodeData->PrintS(rootInode));
    DEBUG(Inode(data).Print());
    DEBUG(PrintInodeFromDisk(rootInode));
    DEBUG(inode.Print());
}

void VSFS::PrintInodeFromDisk(uint32_t inodeId) const {
    DEBUG(uint8_t temp[Inode::GetExportSize()]);
    DEBUG(Inode inode(inodeData->CopyS(temp, inodeId)));

    DEBUG(inode.Print());
}
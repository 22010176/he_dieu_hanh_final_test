#pragma once

#include "../src/StorageManagement.h"
#include "../src/Super.h"
#include "../src/Inode.h"

class VSFS {
private:
    uint8_t* disk;
    uint32_t rootInode;
    size_t diskSize, inodeNum, chunkSize;
    StorageManagement indoes, datas;

public:
    VSFS(Super super);
    VSFS(uint8_t data[24]);

    void SetupParameter();

    StorageManagement GetInodes() const;
    StorageManagement GetDatas() const;

    size_t GetDiskSize() const;
    size_t GetInodeNumber() const;
    size_t GetChunkSize() const;

    ~VSFS();
};


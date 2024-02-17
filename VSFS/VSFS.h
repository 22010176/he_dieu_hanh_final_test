#pragma once

#include "../src/StorageManagement.h"
#include "../src/Super.h"

class VSFS {
private:
    uint8_t* disk;
    uint32_t rootInode;
    StorageManagement indoes, datas;

public:
    VSFS(Super super);
    VSFS(uint8_t data[24]);
    inline void SetupParameter(size_t diskSize, size_t inodeNum, size_t chunkSize);

    ~VSFS();
};


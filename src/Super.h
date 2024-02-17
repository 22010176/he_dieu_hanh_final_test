#pragma once

#include <cstdlib>
#include <cstdint>
#include <iostream>

class Super {
private:
    size_t diskSize, inodeNum, chunkSize;
public:
    Super(uint8_t data[24]);
    Super(size_t diskSize, size_t inodeNum, size_t chunkSize);

    size_t GetDiskSize() const;
    size_t GetInodeNum() const;
    size_t GetChunkSize() const;

    void Print() const;

    uint8_t* ExportData();
    uint8_t* ExportData(uint8_t* _dst);
};
#pragma once

#include <cstdlib>
#include <cstring>
#include <iostream>
#include  <cstdint>

#include "Utils.h"

class Disk {
private:
    size_t diskSize;
    uint8_t* storage;

    void DebugAddress(uint32_t address) const;
    void DebugSize(size_t size) const;
public:
    Disk();
    Disk(uint8_t data[16]);
    Disk(size_t disksize);
    Disk(uint8_t* storage, size_t disksize);
    ~Disk();

    uint8_t* GetStorage() const;
    size_t GetDiskSize() const;
    uint8_t* MapAddress(uint32_t address) const;

    void SetStorage(uint8_t* storage);
    void SetDiskSize(size_t size);

    uint8_t* Read(uint32_t address, size_t size) const;
    uint8_t* Copy(uint8_t* _dst, uint32_t address, size_t size) const;
    void Print(uint32_t address, size_t size) const;

    uint32_t Write(uint32_t address, uint8_t* data, size_t size);
    uint32_t Free(uint32_t address, size_t size);

    uint8_t* ExportData() const;
    uint8_t* ExportData(uint8_t _dst[16]) const;
};

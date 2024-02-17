#pragma once

#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>

#include "StorageManagement.h"

struct InodeTable {
    uint32_t id;
    char name[28];

    InodeTable(char* tables, size_t size);
    InodeTable(uint32_t id, const std::string& name);

    uint8_t* ExportData();
    void Print();
};


// 64
class Inode {
private:
    const static size_t InodeBlockSize;
    const static uint32_t defaultEmptyPointer;

    const static uint32_t DIRECTORY;
    const static uint32_t FILE;

    uint32_t id, type;
    size_t size, link;
    std::vector<uint32_t> blocks;

public:
    Inode();
    Inode(uint8_t data[72]);
    Inode(uint32_t id, uint32_t type);
    ~Inode();

    uint32_t FindFreePointer() const;
    uint32_t AddPointer(uint32_t val);
    void RemovePointer();

    void Print();

    uint8_t* ExportData();
};

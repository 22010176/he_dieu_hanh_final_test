#pragma once

#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <cstring>

struct InodeTable {
    uint32_t id;
    char name[28];

    InodeTable(uint8_t tables[32]);
    InodeTable(uint32_t id, const std::string& name);

    void Print() const;

    uint8_t* ExportData() const;
    uint8_t* ExportData(uint8_t _dst[32]) const;
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

    void Print() const;

    uint8_t* ExportData() const;
    uint8_t* ExportData(uint8_t _dst[72]) const;
};

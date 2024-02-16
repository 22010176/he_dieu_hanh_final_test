#pragma once

#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>

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
    uint32_t id, type;
    size_t size, link;
    std::vector<uint32_t> blocks;

public:
    Inode(char data[64]);
    Inode(uint32_t id, size_t size, uint32_t type);
    ~Inode();

    void AddLink(InodeTable table);
    void AddLink(uint32_t id, char name[28]);
    char* ExportData();

    void Print();
};

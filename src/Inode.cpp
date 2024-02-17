#include "Inode.h"

const size_t Inode::InodeBlockSize = 12;
const uint32_t Inode::defaultEmptyPointer = UINT_MAX;
const uint32_t Inode::DIRECTORY = 0;
const uint32_t Inode::FILE = 1;

Inode::Inode() : id{ 0 }, type{ 0 }, size{ 0 }, link{ 0 }, blocks{ std::vector<uint32_t>{} } {}
Inode::Inode(uint8_t data[72]) {
    uint32_t* uintDat = (uint32_t*)data;

    this->id = uintDat[0];
    this->type = uintDat[1];
    this->size = *((size_t*)data + 1); // 2 + 3
    this->link = *((size_t*)data + 2); // 4 + 5
    this->blocks = std::vector<uint32_t>(&uintDat[6], &uintDat[6 + Inode::InodeBlockSize]);
}
Inode::Inode(uint32_t id, uint32_t type) : id{ id }, size{}, type{ type }, link{} {
    std::cout << this->type << std::endl;
    this->blocks = std::vector<uint32_t>(InodeBlockSize, Inode::defaultEmptyPointer);
}
Inode::~Inode() {}

uint32_t Inode::FindFreePointer() const {
    for (uint32_t i = 0; i < Inode::InodeBlockSize; ++i)
        if (blocks.at(i) == Inode::defaultEmptyPointer) return i;
    return Inode::defaultEmptyPointer;
}
uint32_t Inode::AddPointer(uint32_t val) {
    uint32_t pointer = FindFreePointer();
    if (pointer == defaultEmptyPointer) return -1;
    blocks[pointer] = val;

    return pointer;
}
void Inode::RemovePointer() {
    for (uint32_t i = 0; blocks[i] != defaultEmptyPointer;++i)
        if (blocks[i + 1] == defaultEmptyPointer) blocks[i] = defaultEmptyPointer;
}
void Inode::Print() {
    std::cout << "\nInodeID: " << id << "  Type: " << (this->type == FILE ? "File" : "Directory") << std::endl;
    std::cout << "Size: " << size << "  Link: " << link << std::endl << "|";
    for (uint32_t pointer : blocks) printf(" %d |", pointer);
    std::cout << std::endl;
}

uint8_t* Inode::ExportData() {
    uint8_t* data = new uint8_t[72];
    size_t offset = 0;

    memcpy(data, &this->id, sizeof(this->id));
    offset += sizeof(this->id);
    std::cout << this->type << std::endl;
    memcpy(data + offset, &this->type, sizeof(this->type));
    offset += sizeof(this->type);

    memcpy(data + offset, &this->size, sizeof(this->size));
    offset += sizeof(this->size);

    memcpy(data + offset, &this->link, sizeof(this->link));
    offset += sizeof(this->link);

    memcpy(data + offset, blocks.data(), sizeof(uint32_t) * Inode::InodeBlockSize);

    return data;
}
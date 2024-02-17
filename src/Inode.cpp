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

uint32_t Inode::GetId() const { return id; }
uint32_t Inode::GetType() const { return type; }
size_t Inode::GetSize() const { return size; }
size_t Inode::GetLink() const { return link; }
size_t Inode::GetExportSize() { return 32; }
std::vector<uint32_t> Inode::GetBlocks() const { return blocks; }

void Inode::SetSize(size_t size) { this->size = size; }
void Inode::SetLink(size_t link) { this->link = link; }

void Inode::Print() const {
    std::cout << "\nInodeID: " << id << "  Type: " << (this->type == FILE ? "File" : "Directory") << std::endl;
    std::cout << "Size: " << size << "  Link: " << link << std::endl << "|";
    for (uint32_t pointer : blocks) printf(" %d |", pointer);
    std::cout << std::endl;
}

uint8_t* Inode::ExportData() const { return ExportData(new uint8_t[72]); }
uint8_t* Inode::ExportData(uint8_t _dst[72]) const {
    size_t offset = 0;
    memcpy(_dst, &this->id, sizeof(this->id));
    offset += sizeof(this->id);
    std::cout << this->type << std::endl;
    memcpy(_dst + offset, &this->type, sizeof(this->type));
    offset += sizeof(this->type);

    memcpy(_dst + offset, &this->size, sizeof(this->size));
    offset += sizeof(this->size);

    memcpy(_dst + offset, &this->link, sizeof(this->link));
    offset += sizeof(this->link);

    memcpy(_dst + offset, blocks.data(), sizeof(uint32_t) * Inode::InodeBlockSize);

    return _dst;
}

InodeTable::InodeTable(uint8_t tables[32]) {
    this->id = *(uint32_t*)tables;
    strcpy(this->name, (char*)tables + 4);
}
InodeTable::InodeTable(uint32_t id, const std::string& name) : id{ id } { strcpy(this->name, name.c_str()); }

void InodeTable::Print() const { printf("| ID: %5d NAME: %28s |", this->id, this->name); }

uint8_t* InodeTable::ExportData() const { return ExportData(new uint8_t[32]); }
uint8_t* InodeTable::ExportData(uint8_t _dst[32]) const {
    size_t offset = 0;
    memcpy(_dst, &this->id, sizeof(this->id));
    offset += sizeof(this->id);
    memcpy(_dst + offset, name, sizeof(name));

    return _dst;
}
size_t InodeTable::GetExportSize() { return 32; }
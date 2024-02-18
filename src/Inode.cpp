#include "Inode.h"

const size_t Inode::InodeBlockSize = 12;
const uint32_t Inode::defaultEmptyPointer = UINT_MAX;
const uint32_t Inode::DIRECTORY = 0;
const uint32_t Inode::FILE = 1;

Inode::Inode() : id{ 0 }, type{ 0 }, size{ 0 }, link{ 0 }, blocks{ std::vector<uint32_t>{} } {}
Inode::Inode(uint8_t data[72]) {
    DEBUG(uint32_t * uintDat = (uint32_t*)data);

    DEBUG(this->id = uintDat[0]);
    DEBUG(this->type = uintDat[1]);
    DEBUG(this->size = *((size_t*)data + 1)); // 2 + 3
    DEBUG(this->link = *((size_t*)data + 2)); // 4 + 5
    DEBUG(this->blocks = std::vector<uint32_t>(&uintDat[6], &uintDat[6 + Inode::InodeBlockSize]));
}
Inode::Inode(uint32_t id, uint32_t type) : id{ id }, size{}, type{ type }, link{} {
    DEBUG(std::cout << this->type << std::endl);
    DEBUG(this->blocks = std::vector<uint32_t>(InodeBlockSize, Inode::defaultEmptyPointer));
}
Inode::~Inode() {}

uint32_t Inode::FindFreePointer() const {
    DEBUG(for (uint32_t i = 0; i < Inode::InodeBlockSize; ++i)) {
        DEBUG(if (blocks.at(i) == Inode::defaultEmptyPointer) return i);
    }
    DEBUG(return Inode::defaultEmptyPointer);
}
uint32_t Inode::AddPointer(uint32_t val) {
    DEBUG(uint32_t pointer = FindFreePointer());
    DEBUG(if (pointer == defaultEmptyPointer) return -1);
    DEBUG(blocks[pointer] = val);

    DEBUG(return pointer);
}
void Inode::RemovePointer() {
    DEBUG(for (uint32_t i = 0; blocks[i] != defaultEmptyPointer;++i)) {
        DEBUG(if (blocks[i + 1] == defaultEmptyPointer) blocks[i] = defaultEmptyPointer);
    }
}

uint32_t Inode::GetId() const { DEBUG(return id); }
uint32_t Inode::GetType() const { DEBUG(return type); }
size_t Inode::GetSize() const { DEBUG(return size); }
size_t Inode::GetLink() const { DEBUG(return link); }
size_t Inode::GetExportSize() { DEBUG(return 32); }
std::vector<uint32_t> Inode::GetBlocks() const { DEBUG(return blocks); }

void Inode::SetSize(size_t size) { DEBUG(this->size = size); }
void Inode::SetLink(size_t link) { DEBUG(this->link = link); }

void Inode::Print() const {
    DEBUG(std::cout << "\nInodeID: " << id << "  Type: " << (this->type == FILE ? "File" : "Directory") << std::endl);
    DEBUG(std::cout << "Size: " << size << "  Link: " << link << std::endl << "|");
    DEBUG(for (uint32_t pointer : blocks) printf(" %d |", pointer));
    DEBUG(std::cout << std::endl);
}

uint8_t* Inode::ExportData() const { DEBUG(return ExportData(new uint8_t[72])); }
uint8_t* Inode::ExportData(uint8_t _dst[72]) const {
    DEBUG(size_t offset = 0);
    DEBUG(memcpy(_dst, &this->id, sizeof(this->id)));
    DEBUG(offset += sizeof(this->id));
    DEBUG(std::cout << this->type << std::endl);
    DEBUG(memcpy(_dst + offset, &this->type, sizeof(this->type)));
    DEBUG(offset += sizeof(this->type));

    DEBUG(memcpy(_dst + offset, &this->size, sizeof(this->size)));
    DEBUG(offset += sizeof(this->size));

    DEBUG(memcpy(_dst + offset, &this->link, sizeof(this->link)));
    DEBUG(offset += sizeof(this->link));

    DEBUG(memcpy(_dst + offset, blocks.data(), sizeof(uint32_t) * Inode::InodeBlockSize));

    DEBUG(return _dst);
}

InodeTable::InodeTable(uint8_t tables[32]) {
    DEBUG(this->id = *(uint32_t*)tables);
    DEBUG(strcpy(this->name, (char*)tables + 4));
}
InodeTable::InodeTable(uint32_t id, const std::string& name) : id{ id } {
    DEBUG(strcpy(this->name, name.c_str()));
}

void InodeTable::Print() const { DEBUG(printf("| ID: %5d NAME: %28s |", this->id, this->name)); }

uint8_t* InodeTable::ExportData() const { DEBUG(return ExportData(new uint8_t[32])); }
uint8_t* InodeTable::ExportData(uint8_t _dst[32]) const {
    DEBUG(size_t offset = 0);
    DEBUG(memcpy(_dst, &this->id, sizeof(this->id)));
    DEBUG(offset += sizeof(this->id));
    DEBUG(memcpy(_dst + offset, name, sizeof(name)));

    DEBUG(return _dst);
}
size_t InodeTable::GetExportSize() { DEBUG(return 32); }
#include "Disk.h"

Disk::Disk() : storage{ nullptr }, diskSize{ 0 } {}
Disk::Disk(uint8_t data[16]) {
    DEBUG(this->diskSize = *(size_t*)data);
    DEBUG(std::cout << std::endl << diskSize << std::endl);
    DEBUG(this->storage = (uint8_t*)memcpy(new uint8_t[diskSize], *((uint8_t**)data + 1), diskSize));
}
Disk::Disk(size_t size) : diskSize{ size }, storage{ new uint8_t[size]() } {}
Disk::Disk(uint8_t* storage, size_t disksize) : storage{ storage }, diskSize{ diskSize } {}
Disk::~Disk() {}

size_t Disk::GetExportSize() { return 16; }

uint8_t*& Disk::GetStorage() { DEBUG(return storage); }
size_t& Disk::GetDiskSize() { DEBUG(return diskSize); }

void Disk::SetStorage(uint8_t* storage) {
    DEBUG(this->storage = storage);
}
void Disk::SetDiskSize(size_t size) {
    DEBUG(this->diskSize = size);
}
void Disk::SetStorage(uint8_t*& storage) {
    DEBUG(this->storage = storage);
}
void Disk::SetDiskSize(size_t& size) {
    DEBUG(this->diskSize = size);
}

void Disk::DebugAddress(uint32_t address) const {
    DEBUG(if (address < diskSize) return);
    DEBUG(if (storage == nullptr)) {
        DEBUG(std::cout << std::endl << "storage isnt valid!!! " << storage << std::endl);
        DEBUG(exit(-1));
    }
    DEBUG(Print());
    DEBUG((std::cout << std::endl << __FILE__ << "#" << __LINE__ << " invalid address: " << address << std::endl));
    DEBUG(exit(-1));
}
void Disk::DebugSize(size_t size) const {
    DEBUG(if (size < diskSize) return);
    DEBUG(Print());
    DEBUG(std::cout << __FILE__ << "#" << __LINE__ << " invalid size: " << size << std::endl);
    DEBUG(exit(-1));
}
uint8_t* Disk::MapAddress(uint32_t address) const {
    DEBUG(DebugAddress(address));
    DEBUG(return storage + address);
}

uint32_t Disk::Write(uint32_t address, uint8_t* data, size_t size) {
    DEBUG(DebugSize(size));
    DEBUG(memcpy(MapAddress(address), data, size));
    DEBUG(return 1);
}
uint8_t* Disk::Read(uint32_t address, size_t size) const {
    DEBUG(size_t s = std::min(size, diskSize));
    DEBUG(return (uint8_t*)memcpy(new uint8_t[s], MapAddress(address), s));
}
uint8_t* Disk::Copy(uint8_t* _dst, uint32_t address, size_t size) const {
    DEBUG(return (uint8_t*)memcpy(_dst, MapAddress(address), std::min(diskSize - address, size)));
}
uint32_t Disk::Free(uint32_t address, size_t size) {
    DEBUG(memset(MapAddress(address), 0, std::min(diskSize, size)));
    DEBUG(return 1);
}

void Disk::Print() const {
    DEBUG(std::cout << std::endl);
    DEBUG(std::cout << "Disk size: " << diskSize << std::endl);
    DEBUG(printf("Address: %p\n", storage));
}
void Disk::Print(uint32_t address, size_t size) const {
    DEBUG(std::cout << "\n Address: " << address << "   Size: " << size << '\n');
    DEBUG(PrintMem(MapAddress(address), size));
}

uint8_t* Disk::ExportData() const { DEBUG(return ExportData(new uint8_t[16])); }
uint8_t* Disk::ExportData(uint8_t _dst[16]) const {
    DEBUG(memcpy(_dst, &this->diskSize, sizeof(size_t)));
    DEBUG(memcpy(_dst + sizeof(size_t), &this->storage, sizeof(uint8_t*)));
    DEBUG(return _dst);
}

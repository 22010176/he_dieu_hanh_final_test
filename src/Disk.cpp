#include "Disk.h"

Disk::Disk() : storage{ nullptr }, diskSize{ 0 } {}
Disk::Disk(size_t size) : diskSize{ size }, storage{ new uint8_t[size]() } {}
Disk::Disk(uint8_t* storage, size_t disksize) : storage{ storage }, diskSize{ diskSize } {}
Disk::~Disk() { delete[] storage; }

uint8_t* Disk::GetStorage() const { return storage; }
size_t Disk::GetDiskSize() const { return diskSize; }

void Disk::DebugAddress(uint32_t address) const {
    if (address < diskSize) return;
    if (storage == nullptr) {
        std::cout << "storage isnt valid!!! " << storage << std::endl;
        exit(-1);
    }

    std::cout << __FILE__ << "#" << __LINE__ << " invalid address: " << address << std::endl;
    exit(-1);
}
void Disk::DebugSize(size_t size) const {
    if (size < diskSize) return;

    std::cout << __FILE__ << "#" << __LINE__ << " invalid size: " << size << std::endl;
    exit(-1);
}
uint8_t* Disk::MapAddress(uint32_t address) const {
    DebugAddress(address);
    return storage + address;
}

uint32_t Disk::Write(uint32_t address, uint8_t* data, size_t size) {
    DebugSize(size);
    memcpy(MapAddress(address), data, size);
    return 1;
}
uint8_t* Disk::Read(uint32_t address, size_t size) const {
    size_t s = std::min(size, diskSize);
    return (uint8_t*)memcpy(new uint8_t[s], MapAddress(address), s);
}
uint8_t* Disk::Copy(uint8_t* _dst, uint32_t address, size_t size) const { return (uint8_t*)memcpy(_dst, MapAddress(address), std::min(diskSize, size)); }
void Disk::Print(uint32_t address, size_t size) const {
    std::cout << "\n Address: " << address << "   Size: " << size << '\n';
    PrintMem(MapAddress(address), size);
    std::cout << '\n';
}
uint32_t Disk::Free(uint32_t address, size_t size) {
    memset(MapAddress(address), 0, std::min(diskSize, size));
    return 1;
}

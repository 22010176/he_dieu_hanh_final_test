#include "StorageManagement.h"

StorageManagement::StorageManagement(size_t diskSize, size_t chunkSize) : Disk(diskSize), chunkSize{ chunkSize } { SetupParameter(diskSize, chunkSize); }
StorageManagement::StorageManagement(uint8_t* storage, size_t diskSize, size_t chunkSize) : Disk(storage, diskSize), chunkSize{ chunkSize } { SetupParameter(diskSize, chunkSize); }
StorageManagement::~StorageManagement() {}

size_t StorageManagement::GetChunkNumber() const { return len; }
size_t StorageManagement::GetChunkSize() const { return chunkSize; }
Bitmap StorageManagement::GetBitmap() const { return bitmap; }


uint32_t StorageManagement::GetAddress(uint32_t index) const {
    DebugChunk(index);
    return offset + index * chunkSize;
}
void StorageManagement::DebugChunk(uint32_t chunk) const {
    if (chunk < this->len)  return;
    std::cout << __FILE__ << "#" << __LINE__ << "Invalid Chunk: " << chunk << std::endl;
    exit(-1);
}
void StorageManagement::SetupParameter(size_t diskSize, size_t chunkSize) {
    len = diskSize / chunkSize;

    bitmap = Bitmap(this->GetStorage(), len);
    size_t bitmapChunk = CalcSize(bitmap.GetBitmapSize(), chunkSize);

    len -= bitmapChunk;
    offset = bitmapChunk * chunkSize;
}

uint32_t StorageManagement::WriteS(uint8_t* data, size_t size) {
    uint32_t freeChunk = bitmap.GetFreeCell();
    uint32_t address = GetAddress(freeChunk);
    size_t writeSize = std::min(chunkSize - 4, size);

    Write(address, (uint8_t*)&writeSize, 4);
    Write(address + 4, data, writeSize);

    return freeChunk;
}
size_t StorageManagement::UpdateS(uint32_t chunk, uint8_t* data, size_t size) {
    size_t chunkCurSize = SizeS(chunk);
    if (size > chunkSize - chunkCurSize - 4) return 0;

    size_t newSize = chunkCurSize + size;
    uint32_t address = GetAddress(chunk);

    Write(address, (uint8_t*)&newSize, 4);
    Write(address + 4 + chunkCurSize, (uint8_t*)data, size);

    return size;
}
uint32_t StorageManagement::FreeS(uint32_t chunk) {
    bitmap.FreeCell(chunk);
    return Free(GetAddress(chunk), SizeS(chunk));;
}


uint8_t* StorageManagement::ReadS(uint32_t chunk) const {
    size_t size = SizeS(chunk);
    return Copy(new uint8_t[size], GetAddress(chunk) + 4, size);
}
uint8_t* StorageManagement::CopyS(uint8_t* _dst, uint32_t chunk) const { return Copy(_dst, GetAddress(chunk), SizeS(chunk)); }
void StorageManagement::PrintS(uint32_t chunk)const {
    size_t size = SizeS(chunk);

    std::cout << std::endl << "chunk: " << chunk;
    Print(GetAddress(chunk), size);
}
size_t StorageManagement::SizeS(uint32_t chunk)const {
    size_t size; Copy((uint8_t*)&size, GetAddress(chunk), 4);
    return size;
}


std::vector<uint32_t> StorageManagement::WriteM(uint8_t* data, size_t size) {
    std::vector <uint32_t> result;
    size_t temp = size, offset = 0;
    while (temp > 0) {
        result.push_back(WriteS(data + offset, temp));
        size_t s = SizeS(result.back());
        temp -= s; offset += s;
    }
    return result;
}
uint8_t* StorageManagement::ReadM(std::vector<uint32_t> chunk) { return CopyM(new uint8_t[SizeM(chunk)], chunk); }
uint8_t* StorageManagement::CopyM(uint8_t* _dst, std::vector<uint32_t> chunk) {
    size_t offset = 0;
    for (uint32_t c : chunk) {
        CopyS(_dst + offset, c);
        offset += SizeS(c);
    }
    return _dst;
}
void StorageManagement::PrintM(std::vector<uint32_t> chunk) { for (uint32_t c : chunk) PrintS(c); }
void StorageManagement::FreeM(std::vector<uint32_t> chunk) { for (uint32_t c : chunk) FreeS(c); }
size_t StorageManagement::SizeM(std::vector<uint32_t> chunk) {
    size_t size = 0;
    for (size_t s : chunk) size += SizeS(s);

    return size;
}
std::vector<uint32_t> StorageManagement::PackM(std::vector<uint32_t> chunk) {
    size_t size = SizeM(chunk);
    uint8_t x[size];

    CopyM(x, chunk); FreeM(chunk);
    return WriteM(x, size);
};

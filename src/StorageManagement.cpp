#include "StorageManagement.h"

StorageManagement::StorageManagement() : chunkSize{ 0 }, len{ 0 }, offset{ 0 } {}
// StorageManagement::StorageManagement(const StorageManagement& copied) {
//     std::cout << "Running" << std::endl;
//     DEBUG(SetStorage(copied.GetStorage()));
//     DEBUG(SetDiskSize(copied.GetDiskSize()));
//     DEBUG(SetupParameter(GetDiskSize(), GetChunkSize()));
// }
StorageManagement::StorageManagement(uint8_t data[40]) : Disk(data), bitmap(data + 16) {
    DEBUG(SetupParameter(GetDiskSize(), *((size_t*)data + 4)));
}
StorageManagement::StorageManagement(size_t diskSize, size_t chunkSize) : Disk(diskSize) {
    DEBUG(SetupParameter(diskSize, chunkSize));
}
StorageManagement::StorageManagement(uint8_t* storage, size_t diskSize, size_t chunkSize) : Disk(storage, diskSize) {
    DEBUG(SetupParameter(diskSize, chunkSize));
}
StorageManagement::~StorageManagement() {}

// StorageManagement& StorageManagement::operator=(const StorageManagement& _src) {
//     DEBUG(std::cout << "WTFfff \n");

//     // GetStorage() = _src.GetStorage();
//     // GetDiskSize() = _src.GetDiskSize();
//     // DEBUG(SetDiskSize(_src.GetDiskSize()));
//     // DEBUG(SetStorage(_src.GetStorage()));
//     DEBUG(SetupParameter(GetDiskSize(), GetChunkSize()));

//     DEBUG(return *this);
// }

size_t StorageManagement::GetChunkNumber() const { return len; }
size_t StorageManagement::GetChunkSize() const { return chunkSize; }
Bitmap StorageManagement::GetBitmap() const { return bitmap; }

void StorageManagement::SetChunkSize(size_t size) {
    DEBUG(SetupParameter(GetDiskSize(), chunkSize));
}

uint32_t StorageManagement::GetAddress(uint32_t index) const {
    DEBUG(DebugChunk(index));
    DEBUG(return offset + index * chunkSize);
}
void StorageManagement::DebugChunk(uint32_t chunk) const {
    if (chunk < this->len) return;
    PrintS();
    std::cout << __FILE__ << "#" << __LINE__ << "Invalid Chunk: " << chunk << std::endl;
    exit(-1);
}
void StorageManagement::SetupParameter(size_t diskSize, size_t chunkSize) {
    if (chunkSize <= 4) {
        std::cout << "Invalid Chunk size, need to bigger than 4" << std::endl;
        exit(-1);
    }

    DEBUG(this->chunkSize = chunkSize);
    DEBUG(len = diskSize / chunkSize);
    DEBUG(bitmap = Bitmap(this->GetStorage(), len));
    DEBUG(size_t bitmapChunk = CalcSize(bitmap.GetBitmapSize(), chunkSize));
    for (uint32_t i = 1; i <= bitmapChunk; ++i) {
        DEBUG(bitmap.SetCell(bitmap.GetTotalCell() - i, 1));
    }

    DEBUG(len -= bitmapChunk);
    DEBUG(offset = bitmapChunk * chunkSize);
    // PrintS();
}

uint32_t StorageManagement::WriteS(uint8_t* data, size_t size) {
    DEBUG(return WriteS(bitmap.GetFreeCell(), data, size));
}
uint32_t StorageManagement::WriteS(uint32_t cell, uint8_t* data, size_t size) {
    DEBUG(bitmap.SetCell(cell, 1));
    DEBUG(uint32_t address = GetAddress(cell));
    DEBUG(size_t writeSize = std::min(chunkSize - 4, size));

    DEBUG(Write(address, (uint8_t*)&writeSize, 4));
    DEBUG(Write(address + 4, data, writeSize));

    return cell;
}
size_t StorageManagement::UpdateS(uint32_t chunk, uint8_t* data, size_t size) {
    DEBUG(size_t chunkCurSize = SizeS(chunk));
    DEBUG(if (size > chunkSize - chunkCurSize - 4) return 0);

    DEBUG(size_t newSize = chunkCurSize + size);
    DEBUG(uint32_t address = GetAddress(chunk));

    DEBUG(Write(address, (uint8_t*)&newSize, 4));
    DEBUG(Write(address + 4 + chunkCurSize, (uint8_t*)data, size));

    return size;
}
uint32_t StorageManagement::FreeS(uint32_t chunk) {
    DEBUG(bitmap.FreeCell(chunk));
    DEBUG(return Free(GetAddress(chunk), SizeS(chunk)));
}


uint8_t* StorageManagement::ReadS(uint32_t chunk) const {
    DEBUG(size_t size = SizeS(chunk));
    DEBUG(return Copy(new uint8_t[size], GetAddress(chunk) + 4, size));
}
uint8_t* StorageManagement::CopyS(uint8_t* _dst, uint32_t chunk) const { return Copy(_dst, GetAddress(chunk) + 4, SizeS(chunk)); }
void StorageManagement::PrintS() const {
    DEBUG(Disk::Print());
    DEBUG(std::cout << std::endl << "chunkSize: " << chunkSize << " chunk number: " << len << std::endl);
    DEBUG(std::cout << "chunk offset: " << offset << std::endl);
    DEBUG(bitmap.Print());
}
void StorageManagement::PrintS(uint32_t chunk) const {
    DEBUG(size_t size = SizeS(chunk));

    DEBUG(std::cout << std::endl << "chunk: " << chunk);
    DEBUG(Print(GetAddress(chunk), size));
}
size_t StorageManagement::SizeS(uint32_t chunk) const {
    // std::cout << chunk << std::endl;
    DEBUG(size_t size = 0);
    DEBUG(Copy((uint8_t*)&size, GetAddress(chunk), 4));
    DEBUG(return size);
}

uint8_t* StorageManagement::ReadM(std::vector<uint32_t> chunk) const { return CopyM(new uint8_t[SizeM(chunk)], chunk); }
uint8_t* StorageManagement::CopyM(uint8_t* _dst, std::vector<uint32_t> chunk) const {
    DEBUG(size_t offset = 0);
    DEBUG(for (uint32_t c : chunk)) {
        DEBUG(CopyS(_dst + offset, c));
        DEBUG(offset += SizeS(c));
    }
    DEBUG(return _dst);
}
void StorageManagement::PrintM(std::vector<uint32_t> chunk) const {
    DEBUG(for (uint32_t c : chunk) PrintS(c));
}
size_t StorageManagement::SizeM(std::vector<uint32_t> chunk) const {
    DEBUG(size_t size = 0);
    DEBUG(for (size_t s : chunk) size += SizeS(s));
    DEBUG(return size);
}

std::vector<uint32_t> StorageManagement::WriteM(uint8_t* data, size_t size) {
    DEBUG(std::vector<uint32_t> result{});
    DEBUG(size_t temp = size; size_t offset = 0);
    DEBUG(while (temp > 0)) {
        DEBUG(result.push_back(WriteS(data + offset, temp)));
        DEBUG(size_t s = SizeS(result.back()));

        DEBUG(temp -= s; offset += s);
    }
    DEBUG(return result);
}
void StorageManagement::FreeM(std::vector<uint32_t> chunk) {
    DEBUG(for (uint32_t c : chunk) FreeS(c));
}
std::vector<uint32_t> StorageManagement::PackM(std::vector<uint32_t> chunk) {
    DEBUG(size_t size = SizeM(chunk));
    DEBUG(uint8_t x[size]);

    DEBUG(CopyM(x, chunk));
    DEBUG(FreeM(chunk));
    DEBUG(return WriteM(x, size));
};

uint8_t* StorageManagement::ExportData() const { DEBUG(return ExportData(new uint8_t[40])); }
uint8_t* StorageManagement::ExportData(uint8_t _dst[40]) const {
    size_t offset = 0;

    DEBUG(memcpy(_dst + offset, Disk::ExportData(), Disk::GetExportSize()));
    offset += Disk::GetExportSize();

    DEBUG(memcpy(_dst + offset, bitmap.ExportData(), Bitmap::GetExportSize()));
    offset += Bitmap::GetExportSize();

    DEBUG(memcpy(_dst + offset, &this->chunkSize, sizeof(this->chunkSize)));

    DEBUG(return _dst);
}
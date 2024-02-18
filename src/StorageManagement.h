#pragma once

#include <cstdlib>
#include <cstdint>
#include <vector>

#include "Disk.h"
#include "Bitmap.h"

// Fuck export Data, dont use it;
// doesnt Test PackeM either, but hope it will work

class StorageManagement : public Disk {
private:
    size_t chunkSize, len, offset;
    Bitmap bitmap;

    void DebugChunk(uint32_t chunk) const;
    void SetupParameter(size_t diskSize, size_t chunkSize);
public:
    StorageManagement();
    // StorageManagement(const StorageManagement& copied);
    StorageManagement(uint8_t a[40]);
    StorageManagement(size_t diskSize, size_t chunkSize);
    StorageManagement(uint8_t* storage, size_t diskSize, size_t chunkSize);
    ~StorageManagement();

    // StorageManagement& operator=(const StorageManagement& _src);

    size_t GetChunkNumber() const;
    size_t GetChunkSize() const;
    Bitmap GetBitmap() const;
    uint32_t GetAddress(uint32_t index) const;

    uint8_t* ReadS(uint32_t chunk) const;
    uint8_t* CopyS(uint8_t* _dst, uint32_t chunk) const;
    void PrintS() const;
    void PrintS(uint32_t chunk) const;
    size_t SizeS(uint32_t chunk) const;

    void SetChunkSize(size_t size);

    uint32_t WriteS(uint8_t* data, size_t size);                        // Write        
    uint32_t WriteS(uint32_t cell, uint8_t* data, size_t size);
    size_t UpdateS(uint32_t chunk, uint8_t* data, size_t size);
    uint32_t FreeS(uint32_t chunk);

    uint8_t* ReadM(std::vector<uint32_t> chunk) const;                                  // Read
    uint8_t* CopyM(uint8_t* _dst, std::vector<uint32_t> chunk) const;
    void PrintM(std::vector<uint32_t> chunk) const;
    size_t SizeM(std::vector<uint32_t> chunk) const;                                  // Sized

    std::vector<uint32_t> WriteM(uint8_t* data, size_t size);                     // Write
    void FreeM(std::vector<uint32_t> chunk);
    std::vector<uint32_t> PackM(std::vector<uint32_t> chunk);

    uint8_t* ExportData() const;
    uint8_t* ExportData(uint8_t _dst[40]) const;
};


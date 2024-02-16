#pragma once

#include <cstdlib>
#include <cstdint>
#include <vector>

#include "Disk.h"
#include "Bitmap.h"

class StorageManagement : public Disk {
private:
    size_t chunkSize, len, offset;
    Bitmap bitmap;

    void DebugChunk(uint32_t chunk) const;
    void SetupParameter(size_t diskSize, size_t chunkSize);
public:
    StorageManagement(size_t diskSize, size_t chunkSize);
    StorageManagement(uint8_t* storage, size_t diskSize, size_t chunkSize);
    ~StorageManagement();

    size_t GetChunkNumber() const;
    size_t GetChunkSize() const;
    Bitmap GetBitmap() const;
    uint32_t GetAddress(uint32_t index) const;

    uint8_t* ReadS(uint32_t chunk) const;
    uint8_t* CopyS(uint8_t* _dst, uint32_t chunk) const;
    void PrintS(uint32_t chunk) const;
    size_t SizeS(uint32_t chunk) const;

    uint32_t WriteS(uint8_t* data, size_t size);                        // Write        
    size_t UpdateS(uint32_t chunk, uint8_t* data, size_t size);
    uint32_t FreeS(uint32_t chunk);

    std::vector<uint32_t> WriteM(uint8_t* data, size_t size);                     // Write
    uint8_t* ReadM(std::vector<uint32_t> chunk);                                  // Read
    uint8_t* CopyM(uint8_t* _dst, std::vector<uint32_t> chunk);
    void PrintM(std::vector<uint32_t> chunk);
    void FreeM(std::vector<uint32_t> chunk);
    size_t SizeM(std::vector<uint32_t> chunk);                                  // Sized
    std::vector<uint32_t> PackM(std::vector<uint32_t> chunk);
};


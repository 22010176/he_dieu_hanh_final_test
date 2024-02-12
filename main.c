#include "utils.h"

// Setup parameter              
// Use store and read it in Super later
uint8_t* disk;
const uint32_t dSize = 4 * 1024 * 16 * 4;                    // byte (256KB)
const uint32_t cSize = 256;                                  // byte (4KB)

const Bitmap iBitmap = { .size = (cSize * 5) / _s(Inode), .chunk = 1, .address = 3 * cSize };
const Bitmap dBitmap = { .size = 56,.chunk = 2, .address = 8 * cSize };

// Address
uint8_t* MappingAddress(uint32_t address);
uint32_t GetDataAddress(uint32_t chunkNumber);

// Bitmap
uint8_t CheckCell(Bitmap bm, uint32_t i);
void FreeCell(Bitmap bm, uint32_t i);
uint8_t GetFreeCell(Bitmap bm);

// Read-Write-Debug
void Write(uint32_t address, uint8_t* data, uint32_t size);
uint8_t* Read(uint32_t address, uint32_t size);
void Print(uint8_t* address, uint32_t size);

uint32_t* WriteChunk(uint8_t* data, uint32_t size);
uint32_t* UpdateChunk(uint32_t chunk, uint8_t* data, uint32_t size);
uint8_t* ReadChunk(uint32_t* chunk);

void Init() {
    disk = _ca(dSize);

    uint32_t size = 60;
    uint32_t x[size];
    for (int i = 0; i < size; i++) {
        x[i] = rand() * rand() * rand();
    }

    uint32_t* chunk = WriteChunk((uint8_t*)x, _s(x));
    uint32_t ss = 33;

    uint32_t* chunk2 = UpdateChunk(chunk[0], (uint8_t*)&ss, 4);
    // printf("%d %d\n", chunk[0], chunk[1]);
    uint32_t* a = (uint32_t*)ReadChunk(chunk2);

    for (int i = 0; i < size; i++) {
        if (a[i] == x[i]) continue;
        printf("%-10u %-10u\n", a[i], x[i]);
    }

    free(a);
    free(chunk);
}

void CleanUp() {

    free(disk);
}



int main(int argc, char* argv[]) {
    srand((time_t)time(NULL));
    Init();


    printf("Hello Wolrd");

    CleanUp();
    return 0;
}

uint8_t* MappingAddress(uint32_t address) { return disk + address; }
uint32_t GetDataAddress(uint32_t chunkNumber) { return (chunkNumber + 8) * cSize; }

void Write(uint32_t address, uint8_t* data, uint32_t size) { memcpy(_m(address), data, size); }
uint8_t* Read(uint32_t address, uint32_t size) { return memcpy(_ma(size), _m(address), size); }
void Print(uint8_t* address, uint32_t size) {
    for (int i = 0; i < size;++i) {
        for (int j = 0; j < 8; printf("%d", ReadBit(address[i], j++)));
        printf((i + 1) % 8 != 0 ? " " : "\n");
    }
    printf("\n\n");
}

uint32_t* WriteChunk(uint8_t* data, uint32_t size) {
    uint32_t result[] = { GetFreeCell(dBitmap), min(size, cSize - 4) };

    Write(GetDataAddress(result[0]), (uint8_t*)&result[1], 4);
    Write(GetDataAddress(result[0]) + 4, data, result[1]);

    return memcpy(_ma(_s(result)), result, _s(result));
}
uint32_t* UpdateChunk(uint32_t chunk, uint8_t* data, uint32_t size) {
    uint32_t* offset = (uint32_t*)Read(GetDataAddress(chunk), 4);
    uint32_t s = min(size, cSize - 4 - *offset), s2[] = { chunk, s + *offset };

    Write(GetDataAddress(chunk), (uint8_t*)&s2[1], 4);
    Write(GetDataAddress(chunk) + *offset, data, s);
    free(offset);

    return memcpy(_ma(_s(s2)), s2, _s(s2));
}

uint8_t* ReadChunk(uint32_t* chunk) { return Read(GetDataAddress(chunk[0]) + 4, chunk[1]); }

uint8_t CheckCell(Bitmap bm, uint32_t i) { return ReadBit(_m(bm.address)[i / 8], i % 8); }
void FreeCell(Bitmap bm, uint32_t i) {
    uint8_t* dst = _m(bm.address);
    if (ReadBit(dst[i / 8], i % 8) == 0) return;
    dst[i / 8] -= pow(2, i % 8);
}
uint8_t GetFreeCell(Bitmap bm) {
    uint8_t* dat = _m(bm.address);
    int size = ceil(bm.size / 8);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (ReadBit(dat[i], j)) continue;                             // If dat is 1, continue
            dat[i] |= (1 << j);                                           // Write to Bitmap
            return i * 8 + j;
        }
    }
    return (uint8_t)FAIL;
}



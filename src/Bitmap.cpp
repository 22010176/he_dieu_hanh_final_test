#include "Bitmap.h"

size_t Bitmap::GetExportSize() { return 16; }

Bitmap::Bitmap(uint8_t* address, size_t len) : address{ (uint8_t*)memset(address,0,CalcSize(len,8)) }, len{ len }, size{ CalcSize(len,8) } {}
Bitmap::Bitmap(uint8_t* address, uint8_t* data) : address{ address } {
    DEBUG(this->len = *(size_t*)data);
    DEBUG(this->size = CalcSize(len, 8));
    DEBUG(memcpy(this->address, data + sizeof(len), size));
}
Bitmap::Bitmap(uint8_t data[16]) {
    DEBUG(memcpy(&this->address, data, 8));
    DEBUG(memcpy(&this->len, data + 8, 8));
    DEBUG(this->size = CalcSize(this->len, 8));
}
Bitmap::Bitmap() : len{ 0 }, size{ 0 }, address{ nullptr } {}
Bitmap::~Bitmap() {}

size_t Bitmap::GetTotalCell() const { DEBUG(return this->len); }
size_t Bitmap::GetBitmapSize() const { DEBUG(return this->size); }

bool Bitmap::CheckCell(uint32_t cell) const {
    DEBUG(return ReadBit(address[cell / 8], cell % 8));
}
void Bitmap::FreeCell(uint32_t cell) {
    DEBUG(if (ReadBit(address[cell / 8], cell % 8) == 0) return);
    DEBUG(SetCell(cell, 0));
}
uint32_t Bitmap::GetFreeCell() {
    DEBUG(for (uint32_t i = 0; i < len; ++i)) {
        DEBUG(if (CheckCell(i)) continue);
        DEBUG(SetCell(i, 1));
        DEBUG(return i);
    }
    DEBUG(return -1);
}
void Bitmap::SetCell(uint32_t cell, char stage) {
    DEBUG(if (cell > len)) {
        DEBUG(std::cout << "Invalid Cell: " << cell << std::endl);
        DEBUG(exit(-1));
    }

    DEBUG(if (stage == 0)) {
        DEBUG(address[cell / 8] &= ~(1 << (cell % 8)));
    }
    else {
        DEBUG(address[cell / 8] |= (1 << (cell % 8)));
    }
}
void Bitmap::Print() const {
    DEBUG(printf("\n\n"));
    DEBUG(printf("address: %p size: %d number: %d\n", address, size, len));
    DEBUG(printf("\n\n"));
    DEBUG(PrintMem(address, size));
    DEBUG(std::cout << std::endl);
}
uint8_t* Bitmap::ExportData() const {
    DEBUG(return ExportData(new uint8_t[16]));
}
uint8_t* Bitmap::ExportData(uint8_t _dst[16]) const {
    DEBUG(memcpy(_dst, &this->address, sizeof(uint8_t*)));
    DEBUG(memcpy(_dst + sizeof(uint8_t*), &this->len, sizeof(size_t)));
    DEBUG(return _dst);
}

uint8_t* Bitmap::ExportRawData() const { DEBUG(return ExportRawData(new uint8_t[size + sizeof(len)])); }
uint8_t* Bitmap::ExportRawData(uint8_t* _dst) const {
    DEBUG(memcpy(_dst, &len, sizeof(len)));
    DEBUG(memcpy(_dst + sizeof(len), this->address, size));
    DEBUG(return _dst);
}
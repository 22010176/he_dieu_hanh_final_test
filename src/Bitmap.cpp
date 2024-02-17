#include "Bitmap.h"

Bitmap::Bitmap(uint8_t* address, size_t len) : address{ (uint8_t*)memset(address,0,CalcSize(len,8)) }, len{ len }, size{ CalcSize(len,8) } {}
Bitmap::Bitmap(uint8_t data[16]) {
    memcpy(&this->address, data, 8);
    memcpy(&this->len, data + 8, 8);
    this->size = CalcSize(this->len, 8);
}
Bitmap::Bitmap() {}
Bitmap::~Bitmap() { memset(address, 0, size); }

size_t Bitmap::GetTotalCell() const { return this->len; }
size_t Bitmap::GetBitmapSize() const { return this->size; }

bool Bitmap::CheckCell(uint32_t cell) const {
    return ReadBit(address[cell / 8], cell % 8);
}
void Bitmap::FreeCell(uint32_t cell) {
    if (ReadBit(address[cell / 8], cell % 8) == 0) return;
    SetCell(cell, 0);
}
uint32_t Bitmap::GetFreeCell() {
    for (uint32_t i = 0; i < len; ++i) {
        if (CheckCell(i)) continue;
        SetCell(i, 1);
        return i;
    }
    return -1;
}
void Bitmap::SetCell(uint32_t cell, char stage) {
    if (stage == 0) address[cell / 8] &= ~(1 << (cell % 8));
    else address[cell / 8] |= (1 << (cell % 8));
}
void Bitmap::Print() const {
    printf("\n\n");
    printf("address: %p size: %d number: %d\n", address, size, len);
    printf("\n\n");
    PrintMem(address, size);
    std::cout << std::endl;
}
uint8_t* Bitmap::ExportData() const {
    uint8_t* data = new uint8_t[16];
    memcpy(data, &this->address, sizeof(uint8_t*));
    memcpy(data + sizeof(uint8_t*), &this->len, sizeof(size_t));
    return data;
}

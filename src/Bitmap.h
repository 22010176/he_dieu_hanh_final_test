#pragma once

#include "cstring"
#include "cmath"

#include "Utils.h"

// 8 * 3 = 24
class Bitmap {
private:
    uint8_t* address;
    size_t len, size;
public:
    Bitmap();
    Bitmap(uint8_t data[16]);
    Bitmap(uint8_t* address, size_t len);
    ~Bitmap();

    size_t GetTotalCell() const;
    size_t GetBitmapSize() const;

    bool CheckCell(uint32_t cell) const;
    void FreeCell(uint32_t cell);
    uint32_t GetFreeCell();
    void SetCell(uint32_t cell, char stage);

    void Print() const;
    uint8_t* ExportData() const;
};

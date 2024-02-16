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
    Bitmap(uint8_t* address, size_t size);
    Bitmap(char data[24]);
    Bitmap();
    ~Bitmap();

    size_t GetTotalCell() const;
    size_t GetBitmapSize() const;

    bool CheckCell(uint32_t cell) const;
    void FreeCell(uint32_t cell);
    uint32_t GetFreeCell();
    void SetCell(uint32_t cell, char stage);

    void Print() const;
    char* ExportData() const;
};

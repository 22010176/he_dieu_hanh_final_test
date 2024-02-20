#include "bitmap.h"

int CheckCell(char* bitmapChunk, int size, int cell) {
    if (cell >= size) {
        printf("Invalid Cell number: %d >= %d\n", cell, size);
        return FAIL;
    }
    return ReadBit(bitmapChunk[cell / 8], cell % 8);
}
int GetFreeCell(char* bitmapChunk, int size) {
    int len = CalcSize(size, 8);
    for (int i = 0; i < len; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (ReadBit(bitmapChunk[i], j)) continue;                                // If address is 1, continue
            bitmapChunk[i] |= (1 << j);                                                     // Write to Bitmap
            return i * 8 + j;
        }
    }

    return FAIL;
}
int FreeCell(char* bitmapChunk, int size, int cell) {
    if (cell >= size) {
        printf("Invalid Cell number.\n");
        return FAIL;
    }
    if (ReadBit(bitmapChunk[cell / 8], cell % 8) == 0) return SUCCESS;
    bitmapChunk[cell / 8] -= 1 << (cell % 8);

    return SUCCESS;
}
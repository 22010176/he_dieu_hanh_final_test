#pragma once

#include "Bitmap.h"

class Super {
    uint8_t* disk;
    size_t diskSize, chunkSize;
    Bitmap inodes, datas;
    uint32_t rootInode;

    Super(char data[]);

    char* ExportData();
};
#ifndef _BITMAP_H_
#define _BITMAP_H_

#include "stdio.h"

#include "types.h"
#include "utils.h"

int CheckCell(char* bitmapChunk, int size, int cell);
int GetFreeCell(char* bitmapChunk, int size);
int FreeCell(char* bitmapChunk, int size, int cell);

#endif
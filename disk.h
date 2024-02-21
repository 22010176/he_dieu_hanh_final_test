#pragma once
#ifndef _DISK_H_
#define _DISK_H_

#include "types.h"


//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Constant
_physic disk = (_physic)EMPTY;
_size dSize = 4 * 1024 * 16 * 4;                           // byte (256KB)
_size cSize = 128;                                         // byte (4KB) (minimum 256byte)
Bitmap iBitmap, dBitmap;
_chunk iStartChunk = EMPTY;
_chunk dStartChunk = EMPTY;

_inode rootInode = EMPTY;


#endif
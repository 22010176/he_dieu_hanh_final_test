#pragma once
#ifndef _CONSTANT_H_
#define _CONSTANT_H_

#include "src/utils.h"
#include "src/types.h"

//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
static _physic disk = (_physic)EMPTY;
static _size dSize = 4 * 1024 * 16 * 4;                           // byte (256KB)
static _size cSize = 128;                                         // byte (4KB) (minimum 256byte)
static Bitmap iBitmap, dBitmap;
static _chunk iStartChunk = EMPTY;
static _chunk dStartChunk = EMPTY;
static _inode rootInode = EMPTY;


_physic* GetDiskP();
_size* GetDiskSizeP();
_size* GetChunkSizeP();
Bitmap* GetInodeBitmapP();
Bitmap* GetDataBitmapP();
_chunk* GetInodeStartChunkP();
_chunk* GetDataStartChunkP();
_inode* GetRootInodeP();



_physic GetDisk();
_size GetDiskSize();
_size GetChunkSize();
Bitmap GetInodeBitmap();
Bitmap GetDataBitmap();
_chunk GetInodeStartChunk();
_chunk GetDataStartChunk();
_inode GetRootInode();

#endif
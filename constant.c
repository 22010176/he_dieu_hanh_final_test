#include "constant.h"

extern _physic disk;
extern _size dSize;
extern _size cSize;
extern Bitmap iBitmap, dBitmap;
extern _chunk iStartChunk;
extern _chunk dStartChunk;
extern _inode rootInode;

_physic GetDisk() { return disk; }
_size GetDiskSize() { return dSize; }
_size GetChunkSize() { return cSize; }
Bitmap GetInodeBitmap() { return iBitmap; }
Bitmap GetDataBitmap() { return dBitmap; }
_chunk GetInodeStartChunk() { return iStartChunk; }
_chunk GetDataStartChunk() { return dStartChunk; }
_inode GetRootInode() { return rootInode; }


_physic* GetDiskP() { return &disk; }
_size* GetDiskSizeP() { return &dSize; }
_size* GetChunkSizeP() { return &cSize; }
Bitmap* GetInodeBitmapP() { return &iBitmap; }
Bitmap* GetDataBitmapP() { return &dBitmap; }
_chunk* GetInodeStartChunkP() { return &iStartChunk; }
_chunk* GetDataStartChunkP() { return &dStartChunk; }
_inode* GetRootInodeP() { return &rootInode; }
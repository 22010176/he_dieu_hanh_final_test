#include "utils.h"

// TODO: ALLOW folder to store more than 1 block slot
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Constant
_physic disk = (_physic)EMPTY;
const _size dSize = 4 * 1024 * 16 * 4;                           // byte (256KB)
const _size cSize = 128;                                         // byte (4KB) (minimum 256byte)
Bitmap iBitmap, dBitmap;
_logic iStartChunk = EMPTY;
_logic dStartChunk = EMPTY;

_index rootInode = EMPTY;
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________

// Address
_data MapAddress(_logic address);
_logic GetChunkAddress(_index chunk);
_logic GetInodeAddress(_index inode);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Bitmap
_bool IsCellFree(Bitmap bm, _index i);                                 // IsCellFree
_flag FreeCell(Bitmap bm, _index i);
_index GetFreeCell(Bitmap bm);
void PrintBitmap(Bitmap bm);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Read-Write-Debug
_flag _Write(_logic address, _data data, _size size);                 // _Write
_data _Read(_logic address, _size size);
_flag _Copy(_data _dst, _logic address, _size size);                             // _Read
void _Print(_logic address, _size size);                                 // _Print
_flag _Free(_logic address, _size size);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Operate with 1 chunk
_data ReadS(_index chunk);
_index WriteS(_data data, _size size);                        // Write        
_flag FreeS(_index chunk);
_size SizeS(_index chunk);                         // FreeChunk
void PrintS(_index chunk);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Operate with multiple chunk
_data ReadM(_index chunk[]);                                  // Read
_flag CopyM(_byte _dst[], _index chunk[]);
_index* WriteM(_data data, _size size);                     // Write
_flag FreeM(_index chunk[]);
_size SizeM(_index chunk[]);                                  // Sized
_index* PackM(_index chunk[]);                               // Packed
void PrintM(_index chunk[]);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Operation with Inodes
Inode CreateInode(_file type);
_flag WriteInode(Inode* inode);                                      // UpdateInode
Inode ReadInode(_index inode);                                 // GetInode
_flag FreeInode(_index inode);
void PrintInodeFromDisk(_index inode);                          // PrintChunk
void PrintInode(Inode* inode);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Main program
_flag AlocateMemoryForDisk();
_flag InitBitmap();
_flag CreateRootFolder();
_flag MainProgram();
_flag Clean();



int main() {
    AlocateMemoryForDisk();
    InitBitmap();
    CreateRootFolder();

    Clean();
    return 0;
}



// Address
_physic MapAddress(_logic address) { return disk + address; }
_logic GetChunkAddress(_index chunk) { return chunk * cSize; }
_logic GetDataCAddress(_index dataC) { return GetChunkAddress(dStartChunk) + dataC * cSize; }
_logic GetInodeAddress(_index inode) { return GetChunkAddress(iStartChunk) + inode * _s(Inode); }



// Bitmap
_bool IsCellFree(Bitmap bm, _index i) { return ReadBit(*MapAddress(GetChunkAddress(bm.chunk) + i / 8), i % 8); }
_flag FreeCell(Bitmap bm, _index i) {
    _physic address = MapAddress(GetChunkAddress(bm.chunk));
    if (ReadBit(address[i / 8], i % 8) == 0) return EMPTY;
    address[i / 8] -= 1 << (i % 8);

    return SUCCESS;
}
_index GetFreeCell(Bitmap bm) {
    _physic address = MapAddress(GetChunkAddress(bm.chunk));

    uint32_t size = ceil((float)bm.number / 8);
    for (uint32_t i = 0; i < size; ++i) {
        for (uint32_t j = 0; j < 8; ++j) {
            if (ReadBit(address[i], j)) continue;                                // If address is 1, continue
            address[i] |= (1 << j);                                                     // Write to Bitmap
            return i * 8 + j;
        }
    }

    return EMPTY;
}
void PrintBitmap(Bitmap bm) {
    printf("\n\n");
    printf("chunk: %d size: %d number: %d\n", bm.chunk, bm.size, bm.number);
    _Print(GetChunkAddress(bm.chunk), bm.size);
    printf("\n\n");
}



// Read-Write-Debug
_flag _Write(_logic address, _data data, _size size) {
    if (address == EMPTY) return FAIL;
    memcpy(MapAddress(address), data, size);
    return SUCCESS;
}
_data _Read(_logic address, _size size) {
    if (address == EMPTY) return (_data)FAIL;
    return memcpy(_ma(size), MapAddress(address), size);
}
_flag _Copy(_data _dst, _logic address, _size size) {
    if (address == EMPTY)  return FAIL;
    memcpy(_dst, MapAddress(address), size);
    return SUCCESS;
}
void _Print(_logic address, _size size) {
    if (address == EMPTY) return;
    printf("\n\n");
    _byte data[size]; _Copy(data, address, size);
    for (int i = 0; i < size;++i) {
        for (int j = 0; j < 8; printf("%d", ReadBit(data[i], j++)));
        printf((i + 1) % 8 != 0 ? " " : "\n");
    }
    printf("\n\n");
}
_flag _Free(_logic address, _size size) {
    if (address == EMPTY) return FAIL;
    memset(MapAddress(address), 0, size);
    return SUCCESS;
}



// Operate with 1 chunk     (first 4 byte = data size in chunk, after that is real data)
_data ReadS(_index chunk) { return _Read(GetDataCAddress(chunk) + 4, SizeS(chunk)); }
_flag CopyS(_data _dst, _index chunk) { return _Copy(_dst, GetDataCAddress(chunk) + 4, SizeS(chunk)); }
_index WriteS(_data data, _size size) {
    _index chunk = GetFreeCell(dBitmap);
    if (chunk == FAIL) return FAIL;

    _logic address = GetDataCAddress(chunk);
    _size s = min(size, cSize - 4);

    _Write(address, (_data)&s, 4);
    _Write(address + 4, data, s);

    return chunk;
}
_flag FreeS(_index chunk) { return _Free(GetDataCAddress(chunk), cSize); }
_size SizeS(_index chunk) {
    _size size; _Copy((_byte*)&size, GetDataCAddress(chunk), 4);
    return size;
}
void PrintS(_index chunk) { _Print(GetDataCAddress(chunk), cSize); }



// Operate with multiple chunk ()
_data ReadM(_index chunk[]) {
    _data result = _ma(SizeM(chunk)); CopyM(result, chunk);
    return result;
}
_flag CopyM(_byte _dst[], _index chunk[]) {
    for (_index i = 0, offset = 0; chunk[i] != EMPTY; ++i) {
        CopyS(_dst + offset, chunk[i]);
        offset += SizeS(chunk[i]);
    }
    return SUCCESS;
}
_index* WriteM(_data data, _size size) {
    _index* address = _ma(((_index)ceil((float)size / cSize) + 1) * _s(_index));
    _size temp = size, offset = 0, k = 0;

    while (temp > 0) {
        address[k] = WriteS((_data)(data + offset), temp);
        _size s = SizeS(address[k]);
        temp -= s; offset += s; ++k;
    }
    address[k] = EMPTY;
    return address;
}
_flag FreeM(_index chunk[]) {
    for (_index i = 0; chunk[i] != EMPTY; FreeS(chunk[i++]));
    return SUCCESS;
}
_size SizeM(_index chunk[]) {
    uint32_t size = 0; for (_index i = 0; chunk[i] != EMPTY; ++i) size += SizeS(chunk[i]);
    return size;
}                                  // Sized
_logic* PackM(_index chunk[]) {
    _size size = SizeM(chunk);
    printf("%d", size);
    _byte x[size]; CopyM(x, chunk);
    FreeM(chunk);
    return WriteM(x, size);
}                               // Packed
void PrintM(_index chunk[]) { for (_index i = 0; chunk[i] != EMPTY; PrintS(chunk[i++])); }



// Operation with Inodes
Inode CreateInode(_file type) {
    Inode inode = { .inode_number = GetFreeCell(iBitmap), .link = 0, .size = 0, .type = type };
    for (_index i = 0; i < INODE_MAX_POINTER; inode.blocks[i++] = EMPTY);
    WriteInode(&inode);

    return inode;
}
_flag WriteInode(Inode* inode) {
    return _Write(GetInodeAddress(inode->inode_number), (_data)inode, _s(Inode));
}
Inode ReadInode(_index inode) {
    Inode result; _Copy((_data)&result, GetInodeAddress(inode), _s(Inode));
    return result;
}                                 // GetInode
_flag FreeInode(_index inode) {
    FreeCell(iBitmap, inode);
    return _Free(GetInodeAddress(inode), _s(Inode));
}
void PrintInodeFromDisk(_index inode) {
    Inode result = ReadInode(inode);
    PrintInode(&result);
}
void PrintInode(Inode* inode) {
    printf("\n_INODE____________________________________________________\n");
    printf("number: %-6d size: %-6d link: %-6d type: %-6s\nblocks: ", inode->inode_number, inode->size, inode->link, inode->type == DIRECTORY ? "directory" : "file");

    printf("[");
    for (int i = 0; i < INODE_MAX_POINTER; printf(" %d%c", inode->blocks[i], i < INODE_MAX_POINTER - 1 ? ',' : ' '), i++);
    printf("]\n\n");
}



// Main program
_flag AlocateMemoryForDisk() {
    disk = _ca(dSize);
    return SUCCESS;
}
_flag InitBitmap() {
    uint32_t inodeMinNum = 128;
    iBitmap.chunk = 1;
    iBitmap.number = floor(ceil((float)inodeMinNum * _s(Inode) / cSize) * cSize / _s(Inode));
    iBitmap.size = ceil((float)iBitmap.number / 8);


    dBitmap.chunk = ceil((float)iBitmap.size / cSize) + 1;
    dBitmap.number = (float)dSize / cSize - 1 - ceil((float)iBitmap.size / cSize);
    dBitmap.size = ceil((float)dBitmap.number / 8);

    iStartChunk = dBitmap.chunk + ceil((float)dBitmap.size / cSize); // Data Bitmap + ChunkDataBitmap;
    dStartChunk = iStartChunk + ceil((float)iBitmap.number * _s(Inode) / cSize);

    return SUCCESS;

}
_flag CreateRootFolder() {

    return SUCCESS;
}
_flag MainProgram() {
    return SUCCESS;
}
_flag Clean() {
    free(disk);
    return SUCCESS;
}
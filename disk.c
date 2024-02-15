#include "disk.h"



// Address
_physic MapAddress(_logic address) {
    if (address >= GetDiskSize()) {
        printf("Invalid Address at MapAddress: %u", address);
        exit(-1);
    }
    return GetDisk() + address;
}
_logic GetChunkAddress(_chunk chunk) {
    if (chunk >= GetDataBitmap().number + ceil((float)GetInodeBitmap().size / GetChunkSize()) + 1) {
        printf("Invalid chunk at GetChunkAddress: %u", chunk);
        exit(-1);
    }
    return chunk * GetChunkSize();
}
_logic GetDataCAddress(_chunk dataC) {
    if (dataC >= GetDataBitmap().number) {
        printf("Invalid chunk at GetDataCAddress %u", dataC);
        exit(-1);
    }
    return GetChunkAddress(GetDataStartChunk()) + dataC * GetChunkSize();
}
_logic GetInodeAddress(_inode inode) {
    if (inode >= GetInodeBitmap().number) {
        printf("Invalid inode at GetInodeAddress");
        exit(-1);
    }
    return GetChunkAddress(GetInodeStartChunk()) + inode * _s(Inode);
}



// Bitmap
_bool IsCellFree(Bitmap bm, _id i) { return ReadBit(*MapAddress(GetChunkAddress(bm.chunk) + i / 8), i % 8); }
_flag FreeCell(Bitmap bm, _id i) {
    _physic address = MapAddress(GetChunkAddress(bm.chunk));
    if (ReadBit(address[i / 8], i % 8) == 0) return EMPTY;
    address[i / 8] -= 1 << (i % 8);
    return SUCCESS;
}
_id GetFreeCell(Bitmap bm) {
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
    if (address == EMPTY) {
        printf("Invalid Address at _Write");
        return FAIL;
    }
    memcpy(MapAddress(address), data, size);
    return SUCCESS;
}
_data _Read(_logic address, _size size) {
    if (address == EMPTY) {
        printf("Invalid Address at _Read");
        return (_data)FAIL;
    }
    return memcpy(_ma(size), MapAddress(address), size);
}
_flag _Copy(_data _dst, _logic address, _size size) {
    if (address == EMPTY) {
        printf("Invalid Address at _Copy");
        return FAIL;
    }
    memcpy(_dst, MapAddress(address), size);
    return SUCCESS;
}
void _Print(_logic address, _size size) {
    if (address == EMPTY) {
        printf("Invalid Address at _Print");
        return;
    }
    printf("\n\n");
    _byte data[size]; _Copy(data, address, size);
    for (int i = 0; i < size;++i) {
        for (int j = 0; j < 8; printf("%d", ReadBit(data[i], j++)));
        printf((i + 1) % 8 != 0 ? " " : "\n");
    }
    printf("\n");
}
_flag _Free(_logic address, _size size) {
    if (address == EMPTY) {
        printf("Invalid Address at _Free");
        return FAIL;
    }
    memset(MapAddress(address), 0, size);
    return SUCCESS;
}



// Operate with 1 chunk     (first 4 byte = data size in chunk, after that is real data)
_data ReadS(_chunk chunk) { return _Read(GetDataCAddress(chunk) + 4, SizeS(chunk)); }
_flag CopyS(_data _dst, _chunk chunk) { return _Copy(_dst, GetDataCAddress(chunk) + 4, SizeS(chunk)); }
_chunk WriteS(_data data, _size size) {
    _chunk chunk = GetFreeCell(GetDataBitmap());
    if (chunk >= GetDataBitmap().number) {
        printf("Cant get Free Chunk at WriteS %d", chunk);
        return FAIL;
    }

    _logic address = GetDataCAddress(chunk);
    _size s = min(size, GetChunkSize() - 4);

    _Write(address, (_data)&s, 4);
    _Write(address + 4, data, s);

    return chunk;
}
_size UpdateS(_chunk chunk, _data data, _size size) {
    _size s = SizeS(chunk);
    if (size > GetChunkSize() - 4 - s) return 0;

    _size newSize = s + size;
    _logic address = GetDataCAddress(chunk);

    _Write(address, (_data)&newSize, 4);
    _Write(address + s + 4, data, size);

    return size;
}
_flag FreeS(_chunk chunk) {
    FreeCell(GetDataBitmap(), chunk);
    return _Free(GetDataCAddress(chunk), GetChunkSize());
}
_size SizeS(_chunk chunk) {
    _size size; _Copy((_byte*)&size, GetDataCAddress(chunk), 4);
    return size;
}
void PrintS(_chunk chunk) { _Print(GetDataCAddress(chunk), GetChunkSize()); }



// Operate with multiple chunk ()
_data ReadM(_chunk chunk[]) {
    _data result = _ma(SizeM(chunk)); CopyM(result, chunk);
    return result;
}
_flag CopyM(_byte _dst[], _chunk chunk[]) {
    for (_id i = 0, offset = 0; chunk[i] != EMPTY; ++i) {
        CopyS(_dst + offset, chunk[i]);
        offset += SizeS(chunk[i]);
    }
    return SUCCESS;
}
_chunk* WriteM(_data data, _size size) {
    _chunk* chunk = _ma(((_chunk)ceil((float)size / GetChunkSize()) + 1) * _s(_chunk));
    _size temp = size, offset = 0, k = 0;

    while (temp > 0) {
        chunk[k] = WriteS((_data)(data + offset), temp);
        _size s = SizeS(chunk[k]);
        temp -= s; offset += s; ++k;
    }
    chunk[k] = EMPTY;
    return chunk;
}
_flag FreeM(_chunk chunk[]) {
    for (_id i = 0; chunk[i] != EMPTY; FreeS(chunk[i]), ++i);
    return SUCCESS;
}
_size SizeM(_chunk chunk[]) {
    uint32_t size = 0; for (_id i = 0; chunk[i] != EMPTY; ++i) size += SizeS(chunk[i]);
    return size;
}                                  // Sized
_chunk* PackM(_chunk chunk[]) {
    _size size = SizeM(chunk); _byte x[size];
    CopyM(x, chunk); FreeM(chunk);
    return WriteM(x, size);
}                               // Packed
void PrintM(_chunk chunk[]) { for (_id i = 0; chunk[i] != EMPTY; PrintS(chunk[i++])); }



// Operation with Inodes            (Every change need to call WriteInode to make change to the disk)
Inode CreateInode(_file type) {
    Inode inode = { .inode_number = GetFreeCell(GetInodeBitmap()), .link = 0, .size = 0, .type = type };
    for (_id i = 0; i < INODE_MAX_POINTER + 1; inode.blocks[i++] = EMPTY);
    return inode;
}
_flag WriteInode(Inode* inode) { return _Write(GetInodeAddress(inode->inode_number), (_data)inode, _s(Inode)); }
Inode ReadInode(_inode inode) {
    Inode result; _Copy((_data)&result, GetInodeAddress(inode), _s(Inode));
    return result;
}
_id _FindFreePointer(Inode* inode) {
    for (int i = 0; i < INODE_MAX_POINTER; ++i)
        if (inode->blocks[i] == EMPTY) return i;
    return EMPTY;
}
_flag _PackInodeBlocks(Inode* inode) {
    _chunk* result = PackM(inode->blocks);
    for (_id i = 0; result[i] != EMPTY; inode->blocks[i] = result[i], ++i);
    return SUCCESS;
}
_size _GetTotalFreePointerSize(Inode* inode) { return (INODE_MAX_POINTER - _FindFreePointer(inode) + 1) * GetChunkSize(); }
_flag WriteInodeBlocks(Inode* inode, _data data, _size size) {
    _size offset = 0;

    _chunk block = _FindFreePointer(inode);
    if (block > 0) offset = UpdateS(inode->blocks[(block == EMPTY ? INODE_MAX_POINTER : block) - 1], data, size);
    inode->size += offset;

    if (offset == size) return SUCCESS;
    if (offset == 0 && block == EMPTY) return FAIL;

    _chunk* chunks = WriteM(data + offset, min(size, _GetTotalFreePointerSize(inode)));
    for (_id i = block, j = 0; chunks[j] != EMPTY; ++i, ++j) {
        inode->blocks[i] = chunks[j];
        inode->size += SizeS(chunks[j]);
    }

    return SUCCESS;
}
_flag FreeInode(_inode inode) {
    Inode i = ReadInode(inode);
    FreeCell(GetInodeBitmap(), inode);
    return _Free(GetInodeAddress(inode), _s(Inode));
}
void PrintInodeFromDisk(_inode inode) {
    Inode result = ReadInode(inode);

    PrintInode(&result);
    for (_id i = 0; result.blocks[i] != EMPTY; ++i) PrintInodeTableFromDisk(result.blocks[i]);
}
void PrintInode(Inode* inode) {
    printf("\n_INODE____________________________________________________\n");
    printf("number: %-6d size: %-6d link: %-6d type: %-6s\nblocks: ", inode->inode_number, inode->size, inode->link, inode->type == DIRECTORY ? "directory" : "file");

    printf("[");
    for (int i = 0; i < INODE_MAX_POINTER; printf(" %d%c", inode->blocks[i], i < INODE_MAX_POINTER - 1 ? ',' : ' '), i++);
    printf("]\n\n");
}
InodeTable CreateInodeTable(_inode inodeNumber, char* name) {
    InodeTable table;
    table.inode_number = inodeNumber;
    memcpy(table.name, name, MAX_FILE_NAME_LENGTH);
    return table;
}
void PrintInodeTableFromDisk(_chunk chunk) {
    _size size = SizeS(chunk);
    _size len = ceil((float)size / _s(InodeTable));
    InodeTable tables[len]; CopyS((_data)tables, chunk);

    printf("\nChunk: %d\n", chunk);
    for (_id i = 0; i < len; i++) PrintInodeTable(tables + i);
    printf("\n");
}
void PrintInodeTable(InodeTable* table) { printf("\n| inode number: %-10d name: %-30s |", table->inode_number, table->name); }




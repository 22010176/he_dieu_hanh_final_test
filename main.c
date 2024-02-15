#include "src/utils.h"
#include "src/types.h"

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

_inode rootInode = EMPTY;
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Address
_data MapAddress(_logic address);
_logic GetChunkAddress(_chunk chunk);
_logic GetDataCAddress(_chunk dataC);
_logic GetInodeAddress(_inode inode);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Bitmap
_bool IsCellFree(Bitmap bm, _id i);                                 // IsCellFree
_flag FreeCell(Bitmap bm, _id i);
_id GetFreeCell(Bitmap bm);
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
_chunk WriteS(_data data, _size size);                        // Write        
_size UpdateS(_chunk chunk, _data data, _size size);
_data ReadS(_chunk chunk);
_flag CopyS(_data _dst, _chunk chunk);
void PrintS(_chunk chunk);
_flag FreeS(_chunk chunk);
_size SizeS(_chunk chunk);                         // FreeChunk
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Operate with multiple chunk
_chunk* WriteM(_data data, _size size);                     // Write
_data ReadM(_chunk chunk[]);                                  // Read
_flag CopyM(_byte _dst[], _chunk chunk[]);
void PrintM(_chunk chunk[]);
_flag FreeM(_chunk chunk[]);
_size SizeM(_chunk chunk[]);                                  // Sized
_chunk* PackM(_chunk chunk[]);                               // Packed
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Operation with Inodes
Inode CreateInode(_file type);
_flag WriteInode(Inode* inode);                                      // UpdateInode
Inode ReadInode(_inode inode);                                 // GetInode

_flag _PackInodeBlocks(Inode* inode);
_id _FindFreePointer(Inode* inode);
_flag _PackInodeBlocks(Inode* inode);
_size _GetTotalFreePointerSize(Inode* inode);
_flag WriteInodeBlocks(Inode* inode, _data data, _size size);

_flag FreeInode(_inode inode);
void PrintInodeFromDisk(_inode inode);                          // PrintChunk
void PrintInode(Inode* inode);
InodeTable CreateInodeTable(_inode inodeNumber, char* name);
void PrintInodeTableFromDisk(_chunk chunk);
void PrintInodeTable(InodeTable* table);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Path Utilities
_inode IsPathExist(char** path);                                   // Parent's inode or FAIL Flag
char* GetFileName(char** path);
_inode IsFileExist(_inode inode, char* folderName);
char* GetParentFolder(char** path);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Folder Implementation
Inode SetupFolder();
_flag LinkingFolder(Inode* inode, InodeTable* table);
_inode Mkdir(char* path);
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
_logic GetChunkAddress(_chunk chunk) { return chunk * cSize; }
_logic GetDataCAddress(_chunk dataC) { return GetChunkAddress(dStartChunk) + dataC * cSize; }
_logic GetInodeAddress(_inode inode) { return GetChunkAddress(iStartChunk) + inode * _s(Inode); }



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
    printf("\n");
}
_flag _Free(_logic address, _size size) {
    if (address == EMPTY) return FAIL;
    memset(MapAddress(address), 0, size);
    return SUCCESS;
}



// Operate with 1 chunk     (first 4 byte = data size in chunk, after that is real data)
_data ReadS(_chunk chunk) { return _Read(GetDataCAddress(chunk) + 4, SizeS(chunk)); }
_flag CopyS(_data _dst, _chunk chunk) { return _Copy(_dst, GetDataCAddress(chunk) + 4, SizeS(chunk)); }
_chunk WriteS(_data data, _size size) {
    _chunk chunk = GetFreeCell(dBitmap);
    if (chunk == FAIL) return FAIL;

    _logic address = GetDataCAddress(chunk);
    _size s = min(size, cSize - 4);

    _Write(address, (_data)&s, 4);
    _Write(address + 4, data, s);

    return chunk;
}
_size UpdateS(_chunk chunk, _data data, _size size) {
    _size s = SizeS(chunk);
    if (size > cSize - 4 - s) return 0;

    _size newSize = s + size;
    _logic address = GetDataCAddress(chunk);

    _Write(address, (_data)&newSize, 4);
    _Write(address + s + 4, data, size);

    return size;
}
_flag FreeS(_chunk chunk) {
    FreeCell(dBitmap, chunk);
    return _Free(GetDataCAddress(chunk), cSize);
}
_size SizeS(_chunk chunk) {
    _size size; _Copy((_byte*)&size, GetDataCAddress(chunk), 4);
    return size;
}
void PrintS(_chunk chunk) { _Print(GetDataCAddress(chunk), cSize); }



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
    _chunk* chunk = _ma(((_chunk)ceil((float)size / cSize) + 1) * _s(_chunk));
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
    Inode inode = { .inode_number = GetFreeCell(iBitmap), .link = 0, .size = 0, .type = type };
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
_size _GetTotalFreePointerSize(Inode* inode) { return (INODE_MAX_POINTER - _FindFreePointer(inode) + 1) * cSize; }
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
    printf("%d ", FreeM(i.blocks));
    FreeCell(iBitmap, inode);
    return _Free(GetInodeAddress(inode), _s(Inode));
}
void PrintInodeFromDisk(_inode inode) {
    Inode result = ReadInode(inode);
    PrintInode(&result);

    for (_id i = 0; result.blocks[i] != EMPTY; PrintInodeTableFromDisk(result.blocks[i]), ++i);
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




// Path Utilities
_inode IsPathExist(char** path) {
    _inode res = rootInode;
    for (_id i = 0; res != FAIL && path[i] != NULL; res = IsFileExist(res, path[i++]));

    return res;
}                                   // Parent's inode or FAIL Flag
char* GetFileName(char** path) {
    for (_id i = 0; path[i] != NULL; ++i) if (path[i + 1] == NULL)
        return strcpy(_ca(strlen(path[i]) + 1), path[i]);
    return "";
}
_inode IsFileExist(_id inode, char* folderName) {
    Inode data = ReadInode(inode);

    uint32_t size = SizeM(data.blocks) / _s(InodeTable);
    InodeTable table[size]; CopyM((_data)table, data.blocks);

    for (int i = 0; i < size; ++i)
        if (!strcmp(table[i].name, folderName)) return table[i].inode_number;

    return EMPTY;
}
char* GetParentFolder(char** path) { return JoinStringExceptLast(_ca(SizeStringArr(path)), path, "/"); }



// Folder Implementation
Inode SetupFolder() {
    Inode inode = CreateInode(DIRECTORY);
    InodeTable table = { .inode_number = inode.inode_number, .name = "." };

    LinkingFolder(&inode, &table);

    return inode;
}
_flag LinkingFolder(Inode* inode, InodeTable* table) {
    ++inode->link;
    WriteInodeBlocks(inode, (_data)table, _s(InodeTable));
    return SUCCESS;
}
_inode Mkdir(char* path) {
    return 0;
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
    Inode root = SetupFolder();
    rootInode = root.inode_number;

    WriteInode(&root);
    PrintInodeFromDisk(rootInode);

    FreeInode(root.inode_number);
    PrintInodeFromDisk(rootInode);

    // uint32_t a[] = { 333,4444,5555 };
    // printf("updateCode: %d\n", UpdateS(chunk[3], (_data)a, _s(a)));

    // uint32_t data2[len]; CopyM((_data)data2, chunk);

    // for (int i = 0; i < len + 3; ++i) printf("%d ", data2[i]);
    // PrintM(chunk);

    // Inode root = CreateInode(DIRECTORY);
    // rootInode = root.inode_number;

    // InodeTable _root = CreateInodeTable(root.inode_number, ".");
    // ++root.link;

    // WriteInodeBlocks(&root, (_data)&_root, _s(InodeTable));
    // WriteInode(&root);


    // PrintInodeFromDisk(root.inode_number);
    // PrintInodeTableFromDisk(root.blocks[0]);

    // PrintM(root.blocks);

    return SUCCESS;
}
_flag MainProgram() {
    return SUCCESS;
}
_flag Clean() {
    free(disk);
    return SUCCESS;
}
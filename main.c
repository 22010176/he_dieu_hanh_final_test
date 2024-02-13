#include "utils.h"

// Setup parameter              
// Use store and read it in Super later
uint8_t* disk;
const uint32_t dSize = 4 * 1024 * 16 * 4;                    // byte (256KB)
const uint32_t cSize = 1024 * 4;                                    // byte (4KB)

const Bitmap iBitmap = { .size = (cSize * 5) / _s(Inode), .chunk = 1, .address = 1 * cSize };
const Bitmap dBitmap = { .size = dSize / cSize - 8,.chunk = 2, .address = 2 * cSize };

// Address
uint8_t* MappingAddress(uint32_t address);
uint32_t GetDataAddress(uint32_t chunkNumber);
uint32_t GetInodeAddress(uint32_t inodeNumber);
uint32_t GetChunkAddress(uint32_t i);


// Bitmap
uint32_t CheckCell(Bitmap bm, uint32_t i);
void FreeCell(Bitmap bm, uint32_t i);
uint32_t GetFreeCell(Bitmap bm);


// Read-Write-Debug
uint32_t Write(uint32_t address, uint8_t* data, uint32_t size);
uint8_t* Read(uint32_t address, uint32_t size);
void Print(uint32_t address, uint32_t size);


// Operation with Chunk
uint32_t* WriteChunk(uint8_t* data, uint32_t size);                     // { data }, byte               -> { chunk, size }
uint32_t* WriteToChunk(uint32_t chunk, uint8_t* data, uint32_t size);
uint32_t* UpdateChunk(uint32_t chunk, uint8_t* data, uint32_t size);    // chunkNumber, { data }, byte  -> { chunk, size }
uint8_t* ReadChunk(uint32_t chunk);                                     // chunk                        -> data
uint32_t GetChunkSize(uint32_t chunk);


// Operation with Inodes
Inode CreateInode(uint32_t type);
uint32_t WriteInode(Inode inode);                                       // Inode -> InodeNumber
void UpdateInode(uint32_t inodeNumber, Inode src);
Inode* ReadInode(uint32_t inodeNumber);
void PrintInodeFromDisk(uint32_t inodeNumber);
void PrintInode(Inode inode);


// Operation with InodeTable
InodeTable CreateInodeTable(uint32_t inodeNumber, char* name);
uint32_t WriteInodeTable(InodeTable* tables, uint32_t len);                                           // tables, size (byte) -> { chunkNumber,size }
uint32_t AddInodeTable(uint32_t chunk, InodeTable* src, uint32_t len);                                // tables, size (byte) -> void
InodeTable* ReadInodeTable(uint32_t chunk, uint32_t len);
uint32_t DeleteInodeTable(uint32_t chunk, uint32_t inodeNumber);
void PrintInodeTableFromDisk(uint32_t chunk);
void PrintInodeTable(InodeTable table);


// Testing function
void TestInodeFunc();
void TestInodeTableFunc();


// Main Functions
void Init() {
    disk = _ca(dSize);


    TestInodeFunc();
    printf("\n\n");
    // free(data);
}

void CleanUp() { free(disk); }



int main(int argc, char* argv[]) {
    srand((time_t)time(NULL));
    Init();


    printf("Hello Wolrd");

    CleanUp();
    return 0;
}


void TestInodeFunc() {
    Inode i = CreateInode(FILE);

    WriteInode(i);
    PrintInode(i);

    i.blocks[0] = 33333;
    UpdateInode(i.inode_number, i);
    PrintInodeFromDisk(i.inode_number);

    Print(iBitmap.address, iBitmap.size);
    Print(GetInodeAddress(i.inode_number), _s(Inode));
}

void TestInodeTableFunc() {
    uint32_t len = 20;
    InodeTable table[len - 2];
    for (int i = 0; i < len - 2; i++) table[i] = CreateInodeTable(GetFreeCell(iBitmap), "test");

    uint32_t c = WriteInodeTable(table, _s(table) / _s(InodeTable));
    InodeTable table2[] = { CreateInodeTable(GetFreeCell(iBitmap), "test2"),CreateInodeTable(GetFreeCell(iBitmap), "test2") };

    AddInodeTable(c, table2, _s(table2) / _s(InodeTable));
    DeleteInodeTable(c, 2);
    DeleteInodeTable(c, 7);
    DeleteInodeTable(c, 8);


    AddInodeTable(c, &(InodeTable){.inode_number = GetFreeCell(iBitmap), .name = "hello"}, 1);
    AddInodeTable(c, &(InodeTable){.inode_number = GetFreeCell(iBitmap), .name = "heeello"}, 1);
    AddInodeTable(c, &(InodeTable){.inode_number = GetFreeCell(iBitmap), .name = "heeeello"}, 1);
    PrintInodeTableFromDisk(c);
}


uint8_t* MappingAddress(uint32_t address) { return address == FAIL ? (uint8_t*)FAIL : disk + address; }
uint32_t GetDataAddress(uint32_t chunkNumber) { return GetChunkAddress(chunkNumber + 8); }
uint32_t GetInodeAddress(uint32_t inodeNumber) {
    if (inodeNumber > iBitmap.size) return FAIL;
    return 3 * cSize + inodeNumber * _s(Inode);
}
uint32_t GetChunkAddress(uint32_t i) { return i == FAIL ? FAIL : i * cSize; }


uint32_t Write(uint32_t address, uint8_t* data, uint32_t size) {
    if (address == FAIL) return FAIL;
    memcpy(_m(address), data, size);
    return SUCCESS;
}
uint8_t* Read(uint32_t address, uint32_t size) {
    if (address == FAIL) return (uint8_t*)FAIL;
    return memcpy(_ma(size), _m(address), size);
}
void Print(uint32_t address, uint32_t size) {
    if (address == FAIL) return;

    uint8_t* data = _m(address);
    for (int i = 0; i < size;++i) {
        for (int j = 0; j < 8; printf("%d", ReadBit(data[i], j++)));
        printf((i + 1) % 8 != 0 ? " " : "\n");
    }
    printf("\n\n");
}


uint32_t* WriteChunk(uint8_t* data, uint32_t size) { return WriteToChunk(GetFreeCell(dBitmap), data, size); }
uint32_t* WriteToChunk(uint32_t chunk, uint8_t* data, uint32_t size) {
    uint32_t result[] = { chunk, min(size, cSize - 4) };
    if (result[0] == FAIL) return memcpy(_ma(_s(result)), result, _s(result));

    Write(GetDataAddress(result[0]), (uint8_t*)&result[1], 4);
    Write(GetDataAddress(result[0]) + 4, data, result[1]);

    return memcpy(_ma(_s(result)), result, _s(result));
}
uint32_t* UpdateChunk(uint32_t chunk, uint8_t* data, uint32_t size) {
    uint32_t offset = GetChunkSize(chunk);
    uint32_t s = min(size, cSize - 4 - offset), s2[] = { chunk, s <= 0 ? FAIL : s + offset };

    if (s == 0) return memcpy(_ma(_s(s2)), s2, _s(s2));

    Write(GetDataAddress(chunk), (uint8_t*)&s2[1], 4);
    Write(GetDataAddress(chunk) + 4 + offset, data, s);

    return memcpy(_ma(_s(s2)), s2, _s(s2));
}
uint8_t* ReadChunk(uint32_t chunk) { return Read(GetDataAddress(chunk) + 4, cSize - 4); }
uint32_t GetChunkSize(uint32_t chunk) {
    uint32_t* s = (uint32_t*)Read(GetDataAddress(chunk), 4);
    uint32_t res = *s;
    free(s);

    return res;
}


Inode CreateInode(uint32_t type) {
    Inode inode = { .inode_number = GetFreeCell(iBitmap), .link = 0, .size = 0 };
    GenArr(inode.blocks, INODE_MAX_POINTER, END);
    return inode;
}
uint32_t WriteInode(Inode inode) {
    Write(GetInodeAddress(inode.inode_number), (uint8_t*)&inode, _s(Inode));
    return inode.inode_number;
}
void UpdateInode(uint32_t inodeNumber, Inode src) {
    Write(GetInodeAddress(inodeNumber), (uint8_t*)&src, _s(Inode));
}
void PrintInodeFromDisk(uint32_t inodeNumber) {
    Inode* inode = ReadInode(inodeNumber);
    PrintInode(*inode);
    free(inode);
}
void PrintInode(Inode inode) {
    printf("\n_INODE____________________________________________________\n");
    printf("number: %-6d size: %-6d link: %-6d type: %-6s\nblocks: ", inode.inode_number, inode.size, inode.link, inode.type == DIRECTORY ? "directory" : "file");
    for (int i = 0; i < INODE_MAX_POINTER; printf(" %d |", inode.blocks[i++]));
    printf("\n\n");
}
void FreeInode(uint32_t inodeNumber) {
    uint8_t x[_s(Inode)] = { 0 };
    FreeCell(iBitmap, inodeNumber);
    Write(GetInodeAddress(inodeNumber), x, _s(Inode));
}
inline Inode* ReadInode(uint32_t inodeNumber) { return (Inode*)Read(GetInodeAddress(inodeNumber), _s(Inode)); }


InodeTable CreateInodeTable(uint32_t inodeNumber, char* name) {
    InodeTable table;
    table.inode_number = inodeNumber;
    strcpy(table.name, name);
    table.name[MAX_FILE_NAME_LENGTH - 1] = 0;

    return table;
}
uint32_t WriteInodeTable(InodeTable* tables, uint32_t len) {
    uint32_t* result = WriteChunk((uint8_t*)tables, len * _s(InodeTable));
    uint32_t chunk = result[0];
    free(result);

    return chunk;
}
uint32_t AddInodeTable(uint32_t chunk, InodeTable* src, uint32_t len) {
    uint32_t* x = UpdateChunk(chunk, (uint8_t*)src, _s(InodeTable) * len);
    uint32_t size = x[1];
    free(x);

    return size == FAIL ? FAIL : SUCCESS;
}
uint32_t DeleteInodeTable(uint32_t chunk, uint32_t inodeNumber) {
    uint32_t size = GetChunkSize(chunk);
    InodeTable* table = (InodeTable*)ReadChunk(chunk);

    int len = size / _s(InodeTable), c = 0;
    for (int i = 0; i < len; i++) {
        if (table[i].inode_number == inodeNumber) c = 1;
        if (c) table[i] = table[i + 1];
    }

    if (!c) return FAIL;

    free(WriteToChunk(chunk, (uint8_t*)table, (len - 1) * _s(InodeTable)));
    free(table);
    return SUCCESS;
}
InodeTable* ReadInodeTable(uint32_t chunk, uint32_t len) { return (InodeTable*)Read(GetDataAddress(chunk) + 4, len * _s(InodeTable)); }
void PrintInodeTableFromDisk(uint32_t chunk) {
    uint32_t size = GetChunkSize(chunk);
    uint32_t len = size / _s(InodeTable);
    InodeTable* tables = ReadInodeTable(chunk, size / _s(InodeTable));

    for (int i = 0; i < len; i++) PrintInodeTable(tables[i]);
    free(tables);
    printf("\n\n");
}
inline void PrintInodeTable(InodeTable table) { printf("inode number: %-5d name: %s.\n", table.inode_number, table.name); }


uint32_t CheckCell(Bitmap bm, uint32_t i) { return ReadBit(_m(bm.address)[i / 8], i % 8); }
void FreeCell(Bitmap bm, uint32_t i) {
    uint8_t* dst = _m(bm.address);
    if (ReadBit(dst[i / 8], i % 8) == 0) return;
    dst[i / 8] -= pow(2, i % 8);
}
uint32_t GetFreeCell(Bitmap bm) {
    uint8_t* dat = _m(bm.address);
    int size = ceil(bm.size / 8);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (ReadBit(dat[i], j)) continue;                             // If dat is 1, continue
            dat[i] |= (1 << j);                                           // Write to Bitmap
            return i * 8 + j;
        }
    }
    return FAIL;
}



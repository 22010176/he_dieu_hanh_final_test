#include "utils.h"

// Setup parameter              
// Use store and read it in Super later
uint8_t* disk;
const uint32_t dSize = 4 * 1024 * 16 * 4;                           // byte (256KB)
const uint32_t cSize = 4 * 1024;                                    // byte (4KB)

const Bitmap iBitmap = { .size = (cSize * 5) / _s(Inode), .chunk = 1 };
const Bitmap dBitmap = { .size = dSize / cSize - 8, .chunk = 2 };

uint32_t rootInode = EMPTY;

// Address
uint8_t* MappingAddress(uint32_t address);
uint32_t GetChunkAddress(uint32_t i);
uint32_t GetDataAddress(uint32_t chunkNumber);
uint32_t GetInodeAddress(uint32_t inodeNumber);


// Bitmap
uint32_t CheckCell(Bitmap bm, uint32_t i);
void FreeCell(Bitmap bm, uint32_t i);
uint32_t GetFreeCell(Bitmap bm);


// Read-Write-Debug
uint32_t Write(uint32_t address, uint8_t* data, uint32_t size);
uint8_t* Read(uint32_t address, uint32_t size);
void Print(uint32_t address, uint32_t size);


// Operation with Chunk
uint8_t* ReadChunk(uint32_t chunk);
void PrintChunk(uint32_t chunk);


// Operation with Inodes
Inode CreateInode(uint32_t type);
uint32_t WriteInode(Inode inode);                                       // Inode -> InodeNumber
void UpdateInode(uint32_t inodeNumber, Inode src);
Inode* ReadInode(uint32_t inodeNumber);
void PrintInodeFromDisk(uint32_t inodeNumber);
void PrintInode(Inode inode);


// Operation with Data Chunk
uint32_t* WriteData(uint8_t* data, uint32_t size);                     // { data }, byte               -> { chunk, size }
uint32_t* WriteToData(uint32_t chunk, uint8_t* data, uint32_t size);
uint32_t* UpdateData(uint32_t chunk, uint8_t* data, uint32_t size);    // chunkNumber, { data }, byte  -> { chunk, size }
uint32_t GetDataSize(uint32_t chunk);
uint8_t* ReadData(uint32_t chunk);
void PrintData(uint32_t chunk);


// Operation with InodeTable
InodeTable CreateInodeTable(uint32_t inodeNumber, char* name);
uint32_t WriteInodeTable(InodeTable* tables, uint32_t len);                                           // tables, size (byte) -> { chunkNumber,size }
uint32_t AddInodeTable(uint32_t chunk, InodeTable* src, uint32_t len);                                // tables, size (byte) -> void
InodeTable* ReadInodeTable(uint32_t chunk);
uint32_t DeleteInodeTable(uint32_t chunk, uint32_t inodeNumber);
void PrintInodeTableFromDisk(uint32_t chunk);
void PrintInodeTable(InodeTable table);


// Path Utilities
uint32_t IsPathExist(char* path);                                   // Parent's inode or FAIL Flag
char* GetFileName(char* path);
uint32_t IsFileExist(uint32_t inode, char* folderName);
char* GetParentFolder(char* path);


// Testing function
void TestInodeFunc();
void TestInodeTableFunc();


// Folder Implementation
uint32_t LinkingFolder(uint32_t src, InodeTable table);
uint32_t SetupFolder();
uint32_t Mkdir(char* path);


// Testing File System
void ListShow(char* path);
void PrintInodeData(uint32_t inodeNumber);
void PrintFolderStructure();


// Main Functions
void Init() {
    disk = _ca(dSize);

    // Setup Bitmap


}

void CleanUp() { free(disk); }



int main(int argc, char* argv[]) {
    srand((time_t)time(NULL));
    Init();

    uint32_t a = Mkdir("/a");
    uint32_t c = Mkdir("/c");
    uint32_t d = Mkdir("/d");
    uint32_t e = Mkdir("/d/eg");
    uint32_t f = Mkdir("/d/eg/ed");
    uint32_t f2 = Mkdir("/d/egd");

    PrintInodeData(rootInode);
    PrintInodeData(a);
    PrintInodeData(c);
    // PrintInodeData(d);
    PrintInodeData(e);
    PrintInodeData(f);
    PrintInodeData(f2);
    PrintInodeData(d);


    // PrintInodeData(rootInode);

    CleanUp();
    printf("\n\n\nGood!!!\n");
    return 0;
}


void TestInodeFunc() {
    Inode i = CreateInode(FILE);

    WriteInode(i);
    PrintInode(i);

    i.blocks[0] = 33333;
    UpdateInode(i.inode_number, i);
    PrintInodeFromDisk(i.inode_number);



    Print(GetChunkAddress(iBitmap.chunk), iBitmap.size);
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


uint32_t* WriteData(uint8_t* data, uint32_t size) { return WriteToData(GetFreeCell(dBitmap), data, size); }
uint32_t* WriteToData(uint32_t chunk, uint8_t* data, uint32_t size) {
    uint32_t result[] = { chunk, min(size, cSize - 4) };
    if (result[0] == FAIL) return memcpy(_ma(_s(result)), result, _s(result));

    Write(GetDataAddress(result[0]), (uint8_t*)&result[1], 4);
    Write(GetDataAddress(result[0]) + 4, data, result[1]);

    return memcpy(_ma(_s(result)), result, _s(result));
}
uint32_t* UpdateData(uint32_t chunk, uint8_t* data, uint32_t size) {
    uint32_t offset = GetDataSize(chunk);
    uint32_t s = min(size, cSize - 4 - offset), s2[] = { chunk, s <= 0 ? FAIL : s + offset };

    if (s == 0) return memcpy(_ma(_s(s2)), s2, _s(s2));

    Write(GetDataAddress(chunk), (uint8_t*)&s2[1], 4);
    Write(GetDataAddress(chunk) + 4 + offset, data, s);

    return memcpy(_ma(_s(s2)), s2, _s(s2));
}
uint32_t GetDataSize(uint32_t chunk) {
    uint32_t* s = (uint32_t*)Read(GetDataAddress(chunk), 4);
    uint32_t res = *s;
    free(s);

    return res;
}
uint8_t* ReadData(uint32_t chunk) { return Read(GetDataAddress(chunk) + 4, cSize - 4); }
void PrintData(uint32_t chunk) { PrintChunk(chunk + 8); }


uint8_t* ReadChunk(uint32_t chunk) { return Read(GetChunkAddress(chunk), cSize); }
void PrintChunk(uint32_t chunk) { Print(GetChunkAddress(chunk), cSize); }


Inode CreateInode(uint32_t type) {
    Inode inode = { .inode_number = GetFreeCell(iBitmap), .link = 0, .size = 0, .type = type };
    GenArr(inode.blocks, INODE_MAX_POINTER, EMPTY);
    return inode;
}
uint32_t WriteInode(Inode inode) {
    Write(GetInodeAddress(inode.inode_number), (uint8_t*)&inode, _s(Inode));
    return inode.inode_number;
}
void UpdateInode(uint32_t inodeNumber, Inode src) { Write(GetInodeAddress(inodeNumber), (uint8_t*)&src, _s(Inode)); }
void PrintInodeFromDisk(uint32_t inodeNumber) {
    Inode* inode = ReadInode(inodeNumber);
    PrintInode(*inode);
    free(inode);
}
void PrintInode(Inode inode) {
    printf("\n_INODE____________________________________________________\n");
    printf("number: %-6d size: %-6d link: %-6d type: %-6s\nblocks: ", inode.inode_number, inode.size, inode.link, inode.type == DIRECTORY ? "directory" : "file");

    printf("[");
    for (int i = 0; i < INODE_MAX_POINTER; printf(" %d%c", inode.blocks[i], i < INODE_MAX_POINTER - 1 ? ',' : ' '), i++);
    printf("]\n\n");
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
    memcpy(table.name, name, MAX_FILE_NAME_LENGTH);
    return table;
}
uint32_t WriteInodeTable(InodeTable* tables, uint32_t len) {
    uint32_t* result = WriteData((uint8_t*)tables, len * _s(InodeTable));
    uint32_t chunk = result[0];
    free(result);

    return chunk;
}
uint32_t AddInodeTable(uint32_t chunk, InodeTable* src, uint32_t len) {
    uint32_t* x = UpdateData(chunk, (uint8_t*)src, _s(InodeTable) * len);
    uint32_t size = x[1];
    free(x);

    return size == FAIL ? FAIL : SUCCESS;
}
uint32_t DeleteInodeTable(uint32_t chunk, uint32_t inodeNumber) {
    uint32_t size = GetDataSize(chunk);
    InodeTable* table = (InodeTable*)ReadData(chunk);

    int len = size / _s(InodeTable), c = 0;
    for (int i = 0; i < len; i++) {
        if (table[i].inode_number == inodeNumber) c = 1;
        if (c) table[i] = table[i + 1];
    }

    if (!c) return FAIL;

    free(WriteToData(chunk, (uint8_t*)table, (len - 1) * _s(InodeTable)));
    free(table);
    return SUCCESS;
}
InodeTable* ReadInodeTable(uint32_t chunk) { return (InodeTable*)ReadData(chunk); }
void PrintInodeTableFromDisk(uint32_t chunk) {
    uint32_t size = GetDataSize(chunk);
    uint32_t len = size / _s(InodeTable);
    InodeTable* tables = ReadInodeTable(chunk);

    for (int i = 0; i < len; i++) PrintInodeTable(tables[i]);
    free(tables);
    printf("\n\n");
}
inline void PrintInodeTable(InodeTable table) { printf("| inode number: %-10d name: %30s |\n", table.inode_number, table.name); }


uint32_t IsPathExist(char* path) {
    char** p = SplitString(path, "/");

    uint32_t res = rootInode;
    for (uint32_t i = 0; res != FAIL && p[i] != NULL; res = IsFileExist(res, p[i++]));

    _fm(p);
    return res;
}
char* GetFileName(char* path) {
    char** p = SplitString(path, "/");
    uint32_t i = 0;
    for (; p[i + 1] != NULL; ++i);

    char* result = strcpy(_ca(strlen(p[i]) + 1), p[i]);
    _fm(p);
    return result;
}
uint32_t IsFileExist(uint32_t inode, char* folderName) {
    Inode* data = ReadInode(inode);
    InodeTable* table = ReadInodeTable(data->blocks[0]);

    uint32_t size = GetDataSize(data->blocks[0]) / _s(InodeTable);
    free(data);

    for (int i = 0; i < size; ++i) if (!strcmp(table[i].name, folderName)) {
        uint32_t inodeNumber = table[i].inode_number;
        free(table);
        return inodeNumber;
    }

    free(table);
    return FAIL;
}
char* GetParentFolder(char* path) {
    char** p = SplitString(path, "/");
    char res[strlen(path) + 1];
    res[0] = 0;

    for (int i = 0; p[i + 1] != NULL; strcat(res, p[i++]), strcat(res, "/"));

    _fm(p);
    return strcpy(_ma(strlen(res) + 1), res);
}


uint32_t CheckCell(Bitmap bm, uint32_t i) { return ReadBit(_m(GetChunkAddress(bm.chunk))[i / 8], i % 8); }
void FreeCell(Bitmap bm, uint32_t i) {
    uint8_t* dst = _m(GetChunkAddress(bm.chunk));
    if (ReadBit(dst[i / 8], i % 8) == 0) return;
    dst[i / 8] -= pow(2, i % 8);
}
uint32_t GetFreeCell(Bitmap bm) {
    uint8_t* dat = _m(GetChunkAddress(bm.chunk));
    int size = ceil(bm.size / 8);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (ReadBit(dat[i], j)) continue;                                // If dat is 1, continue
            dat[i] |= (1 << j);                                                     // Write to Bitmap
            return i * 8 + j;
        }
    }
    return FAIL;
}


uint32_t SetupFolder() {
    Inode inode = CreateInode(DIRECTORY);
    InodeTable table = { .inode_number = inode.inode_number, .name = "." };

    inode.blocks[0] = WriteInodeTable(&table, 1);
    inode.link = 1;
    inode.size = sizeof(table);

    WriteInode(inode);
    return inode.inode_number;
};
uint32_t Mkdir(char* path) {
    if (IsPathExist(path) != FAIL) return FAIL;

    if (rootInode == EMPTY) {
        rootInode = SetupFolder();
        LinkingFolder(rootInode, CreateInodeTable(rootInode, ".."));
        LinkingFolder(rootInode, CreateInodeTable(rootInode, ""));
        LinkingFolder(rootInode, CreateInodeTable(rootInode, "root"));
    }
    char* parent = GetParentFolder(path);
    uint32_t parentInode = IsPathExist(parent);
    free(parent);

    if (parentInode == FAIL) return FAIL;
    char* fileName = GetFileName(path);

    uint32_t newFolder = SetupFolder();
    LinkingFolder(parentInode, CreateInodeTable(newFolder, fileName));
    LinkingFolder(newFolder, CreateInodeTable(parentInode, ".."));

    free(fileName);
    return newFolder;
};
uint32_t LinkingFolder(uint32_t src, InodeTable table) {
    Inode* child = ReadInode(src);

    // InodeTable table = CreateInodeTable(linkInode, linkName);
    AddInodeTable(child->blocks[0], &table, 1);

    child->link++;
    child->size = GetDataSize(child->blocks[0]);

    WriteInode(*child);

    free(child);
    return SUCCESS;
}


void ListShow(char* path) {

}
void PrintInodeData(uint32_t inodeNumber) {
    if (inodeNumber == FAIL) return;
    Inode* inode = ReadInode(inodeNumber);

    PrintInode(*inode);
    PrintInodeTableFromDisk(inode->blocks[0]);

    free(inode);
}
void PrintFolderStructure() {

}
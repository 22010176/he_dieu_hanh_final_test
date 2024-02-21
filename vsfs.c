#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "utils.h"
#include "types.h"
#include "bitmap.h"


// gcc bitmap.c utils.c vsfs.c -o out/vsfs.exe
// ./out/vsfs


size_t chunkSize = 1024;                            // Kích thước của 1 chunk (byte).
size_t diskSize = 256 * 1024;                       // Kích thước của toàn bộ bộ nhớ (byte).
size_t inodeSize = sizeof(Inode);                   // Lấy kích thước của một inode.
size_t inodeTableSize = sizeof(InodeTable);         // Lấy kích thước của một inodeTable.
size_t size_tSize = sizeof(size_t);
size_t intSize = sizeof(int);
size_t charSize = sizeof(char);


char* data;                                                 // 4kb
size_t numberInode, numberChunk;                            // Số lượng inode và data chunk có ở trong bộ nhớ.
char* inodeBitmapChunk, * dataBitmapChunk, * dataChunk;     // Địa chỉ của inode bitmap, data bitmap và data chunk.
Inode* inodeChunk;                                          // Địa chỉ của inode chunk.
int rootInode;                                              // mã inode của Inode root.

void InitParam();
void InitFolder();

Inode CreateInode(int type);
char* ReadInode(char* _dst, Inode* inode);
void UpdateInode(Inode* inode);
int UpdateInodeData(Inode* inode, char* data, size_t size);
void PrintInode(Inode* inode);
void PrintInodeInDisk(int inodeNum);
int GetFreePointer(Inode* inode);
void AddLinkToInode(Inode* inode, InodeTable table);

int IsThisFileInFolder(Inode* inode, char* path, int type);
int IsPathExist(char** p, int level, int type);

void v_mkdir(char* path);
void v_link(char* _dst, char* _src);
void v_unlink(char* dst);
void v_open(char* path);
void v_write(char* path, char* content, size_t size);

void PrintFileStructure(int inodeNumber, int level);


/* TEST LIST
Cần tạo thêm hàm:   In trạng thái bộ nhớ của ổ cứng.

v_mkdir: Tạo folder dựa vào đường dẫn đầu vào.
    1. Kiểm tra xem thư mục cha có tồn tại ko.
    2. Nếu thư mục cha tồn tại, thì kiểm tra xem thư mục đã được tạo ra chưa.
    3. Nếu tạo rồi thì sẽ báo lỗi ko tạo được thư mục.
    4. Nếu chưa tồn tại thư mục:
        Sẽ tạo ra một inode con (Cần kiểm tra xem vị trí thư mục con trong bitmap có được chỉnh về 1 chưa).
        Liên kết thư mục con với InodeTable của thư mục cha.
    5. Cập nhật 2 cái trên vào disk (Lúc trước đó chỉ mới đọc và thay đổi ở trong ram, chưa trực tiếp thay đổi bộ nhớ trên disk).

v_open: Tạo ra một tệp tin ở đường dẫn
    1. Kiểm tra xem thư mục cha có tồn tại ko.
    2. Nếu thứ mục cha tồn tại thì kiểm tra xem tệp tin đã tồn tại trong thư mục cha chưa.
    3. Nếu tệp tin được tạo rồi thì chương trình báo lỗi ko tạo được tệp tin.
    4. Nếu chưa tạo tệp tin:
        Tạo inode chưa nội dung tệp tin.
        Liên kết nó với thư mục cha.
    5. Cập nhật 2 inode vào disk.

v_link: Copy một file từ vị trí này sang một vị trí khác.
    1. Kiểm tra xem đường dẫn nguồn có tồn tại hay ko.
    2. Kiểm tra xem thư mục cha của đường dẫn đích có tồn tại ko.
    3. Nếu một trong 2 cái kia ko đáp ứng thì báo lỗi.
    4. Nếu cả 2 cái kia ổn:
        Tạo liên kết thư cha đường dẫn đích với đường dẫn nguồn.
    5. Cập nhật InodeTable vào disk.

v_unlink: Bỏ một liên kết ở trong đường dẫn đích.
    1. Kiểm tra xem đường dẫn tồn tại hay ko.
    2. Nếu nó không tồn tại thì báo lỗi.
    3. Nếu nó tồn tại thì:
        Kiểm tra xem liên kết dẫn tới nó có bằng 1 ko:
            True    => Xóa nó + dữ liệu ở trong block
            False   => Bỏ qua.
        Xóa nó trong Inode Table thư mục cha.
    4. Cập nhất cả 2 vào bộ nhớ.

v_write: Viết dữ liệu vào tệp tin đường dẫn.
    1. Kiểm tra đường dẫn có tồn tại ko.
    2. Nếu không tồn tại thì báo lỗi.
    3. Nếu nó tồn tại:
        Kiểm tra xem có phải là file ko.
        Nếu không thì báo lỗi
        Nếu đúng thì viết.
    4. Cập nhật vào Inode trên disk.

Cách kiểm tra Bitmap:
1. Giả sử một bitmap có 16 phần tử:
0000 0000 0000 0000
2. Nếu phần từ thứ 3 đang được dùng, thì bitmap sẽ ở trạng thái sau:
0001 0000 0000 0000
3. Nếu ta ko dùng nữa, thì bitmap sẽ cập nhất trạng thái thành
0000 0000 0000 0000
*/
int main() {
    InitParam();
    InitFolder();


    // Tạo một vài folder
    v_mkdir("a");
    v_mkdir("C");
    v_mkdir("/ab/");
    v_mkdir("/ab/d");
    v_mkdir("ab/e/");
    v_mkdir("ab/e/f");
    v_mkdir("ab/e/f/h");
    v_mkdir("ab/e/g");
    v_mkdir("ab/f");

    // Copy folder từ vị trí này sang vị trí khác.
    v_link("/ab/ed", "a");
    v_link("ab/e/eh", "ab/d");

    // Tạo thêm một vài folder nữa.
    v_mkdir("b");
    v_mkdir("bdd");
    v_mkdir("bddd");
    v_mkdir("bddd");

    // Tạo file.
    v_open("a.txt");
    v_open("b/bee");

    // Viết vào file một vài thứ.
    v_write("b/bee", "Asdfasdf", 9);

    // Xóa bỏ liên kết file.
    v_unlink("ab/d/");
    v_unlink("a");
    v_unlink("C");

    // In ra màn hình cấu trúc của folder.
    PrintFileStructure(rootInode, 0);

    _Print(inodeBitmapChunk, numberInode / 8);
    _Print(dataBitmapChunk, numberChunk / 8);

    free(data);
    return 0;
}


void InitParam() {
    data = _ca(diskSize);

    inodeBitmapChunk = data + chunkSize;
    dataBitmapChunk = data + 2 * chunkSize;
    inodeChunk = (Inode*)(data + 3 * chunkSize);
    dataChunk = data + 8 * chunkSize;
    numberInode = CalcSize(5 * chunkSize, inodeSize);
    numberChunk = CalcSize(diskSize, chunkSize) - 8;
}
void InitFolder() {
    Inode root = CreateInode(_DIRECTORY);
    rootInode = root.id;
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = ".." });
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = "." });
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = "root" });
    AddLinkToInode(&root, (InodeTable) { .id = rootInode, .name = "" });

    UpdateInode(&root);
}
int GetFreePointer(Inode* inode) {
    for (int i = 0; i < MaxPointers; ++i) if (inode->blocks[i] == EMPTY) return i;
    return MaxPointers;
}
char* ReadInode(char* _dst, Inode* inode) {
    for (int i = 0, offset = 0, temp = inode->size; inode->blocks[i] != EMPTY;++i) {
        size_t readSize = min(chunkSize, temp);
        memcpy(_dst + offset, dataChunk + inode->blocks[i] * chunkSize, readSize);
        offset += readSize; temp -= readSize;
    }
    return _dst;
}
void PrintInodeInDisk(int inodeNum) {
    printf("\n\n");
    if (CheckCell(inodeBitmapChunk, numberInode, inodeNum) == 0) return;

    Inode inode = inodeChunk[inodeNum];
    printf("ID: %-3d TYPE: %-10s LINK: %-5d SIZE: %d\n", inode.id, inode.type == _FILE ? "File" : "Directory", inode.link, inode.size);

    for (int i = 0; i < MaxPointers; ++i) printf("%2d ", inode.blocks[i]);

    if (inode.type == _FILE) return;
    printf("\n");

    int len = CalcSize(inode.size, inodeTableSize);
    InodeTable tables[CalcSize(inode.size, inodeTableSize)]; ReadInode((char*)tables, &inode);
    for (int i = 0; i < len; ++i) printf("| %-5d %30s |\n", tables[i].id, tables[i].name);
}
void UpdateInode(Inode* inode) {
    if (CheckCell(inodeBitmapChunk, numberInode, inode->id) == 0) {
        printf("Inode isnt ocupied yet!\n");
        return;
    }
    memcpy(&inodeChunk[inode->id], inode, inodeSize);
}
Inode CreateInode(int type) {
    int id = GetFreeCell(inodeBitmapChunk, numberInode);
    if (id >= numberInode || id == FAIL) {
        printf("Invalid inode");
        return (Inode) {};
    }

    Inode inode = { .id = id ,.type = type,.link = 0,.size = 0 };
    for (int i = 0; i < MaxPointers; ++i) inode.blocks[i] = EMPTY;

    UpdateInode(&inode);
    return inode;
}

void PrintInode(Inode* inode) {
    printf("\n\n");

    printf("ID: %-3d TYPE: %-10s LINK: %-5d SIZE: %d\n", inode->id, inode->type == _FILE ? "File" : "Directory", inode->link, inode->size);

    for (int i = 0; i < MaxPointers; ++i) printf("%2d ", inode->blocks[i]);

    if (inode->type == _FILE) return;
    printf("\n");

    int len = CalcSize(inode->size, inodeTableSize);
    InodeTable tables[CalcSize(inode->size, inodeTableSize)]; ReadInode((char*)tables, inode);
    for (int i = 0; i < len; ++i) printf("| %-5d %30s |\n", tables[i].id, tables[i].name);
}

int UpdateInodeData(Inode* inode, char* data, size_t size) {
    size_t sizeRemain = CalcSize(inode->size, chunkSize) * chunkSize - inode->size;
    int pointer = GetFreePointer(inode);

    if (sizeRemain > size && pointer > 0) {
        memcpy(dataChunk + inode->blocks[pointer - 1] * chunkSize + inode->size % chunkSize, data, size);
        inode->size += size;
        return SUCCESS;
    }

    if (pointer == MaxPointers) {
        printf("Cant not find free chunk\n");
        return FAIL;
    }

    size_t temp = size;
    size_t offset = 0;
    while (temp > 0) {
        int freeChunk = GetFreeCell(dataBitmapChunk, numberChunk);
        if (freeChunk == FAIL) {
            printf("Run out of free chunk!!!\n");
            return FAIL;
        }
        size_t writeSize = min(chunkSize, size);
        memcpy(dataChunk + freeChunk * chunkSize, data + offset, writeSize);

        temp -= writeSize; offset += writeSize; inode->size += writeSize;
        inode->blocks[pointer++] = freeChunk;

        if (pointer >= MaxPointers) {
            printf("No more pointer.\n");
            return FAIL;
        }
    }
    return SUCCESS;
}
void AddLinkToInode(Inode* inode, InodeTable table) {
    if (CheckCell(inodeBitmapChunk, numberInode, inode->id) == 0) {
        printf("Inode doesnt exist!!!\n");
        return;
    }

    int result = UpdateInodeData(inode, (char*)&table, inodeTableSize);
    if (result == FAIL) {
        printf("Couldnt add link to folder, run out of space.\n");
        return;
    }
    if (inode->id == table.id) ++inode->link;
    else ++inodeChunk[table.id].link;
}

int IsThisFileInFolder(Inode* inode, char* path, int type) {
    int len = CalcSize(inode->size, inodeTableSize);
    InodeTable table[len]; ReadInode((char*)table, inode);

    for (int i = 0; i < len; ++i) {
        if (strcmp(table[i].name, path)) continue;
        if (inodeChunk[table[i].id].type == type || type == _ANYTYPE) return table[i].id;
    }

    return FAIL;
}

int IsPathExist(char** p, int level, int type) {
    int inodeID = rootInode;
    for (int i = 0; p[i + level] != NULL; ++i) {
        inodeID = IsThisFileInFolder(&inodeChunk[inodeID], p[i], p[i + level + 1] == NULL ? type : _ANYTYPE);
        if (inodeID == FAIL) break;
    }
    return inodeID;
}

void v_mkdir(char* path) {
    char** splitStr = SplitString(path, "/");

    char* fileName = GetFileName(splitStr);
    int inodeParentID = IsPathExist(splitStr, 1, _DIRECTORY), inodeChildID = IsPathExist(splitStr, 0, _DIRECTORY);

    if (inodeParentID == FAIL || inodeChildID != FAIL) {
        if (inodeParentID == FAIL) printf("Path doesnt not existed.\n");
        if (inodeChildID != FAIL) printf("File already existesd.\n");
        FreeMem(splitStr);
        free(fileName);
        return;
    }
    Inode child = CreateInode(_DIRECTORY), parent = inodeChunk[inodeParentID];
    InodeTable childTable; childTable.id = child.id; strcpy(childTable.name, fileName);

    AddLinkToInode(&child, (InodeTable) { .id = parent.id, .name = ".." });
    AddLinkToInode(&child, (InodeTable) { .id = child.id, .name = "." });
    UpdateInode(&child);

    AddLinkToInode(&parent, childTable);
    UpdateInode(&parent);

    FreeMem(splitStr);
    free(fileName);
}

void v_link(char* _dst, char* _src) {
    char** dstSplit = SplitString(_dst, "/");
    char** srcSplit = SplitString(_src, "/");

    char* dstName = GetFileName(dstSplit);
    int dstInode = IsPathExist(dstSplit, 1, _DIRECTORY);
    int srcInode = IsPathExist(srcSplit, 0, _ANYTYPE);

    if (dstInode == FAIL || srcInode == FAIL) {
        if (dstInode == FAIL) printf("Cant not find path.\n");
        if (srcInode == FAIL) printf("source file doesnt exist.\n");
        return;
    }
    InodeTable table = { .id = srcInode,.name = "" };
    strcpy(table.name, dstName);

    AddLinkToInode(&inodeChunk[dstInode], table);

    FreeMem(dstSplit);
    FreeMem(srcSplit);
    free(dstName);
}
void v_unlink(char* dst) {
    char** strSplit = SplitString(dst, "/");

    int parentNumber = IsPathExist(strSplit, 1, _DIRECTORY);
    int dstNumber = IsPathExist(strSplit, 0, _ANYTYPE);

    if (parentNumber == FAIL || dstNumber == FAIL) {
        printf("Cant unlink item: %s\n", dst);
        return;
    }

    Inode child = inodeChunk[dstNumber];
    if (--child.link == 1) {
        for (int i = 0; child.blocks[i] != EMPTY; ++i) FreeCell(dataBitmapChunk, numberChunk, child.blocks[i]);
        UpdateInode(&child);
        FreeCell(inodeBitmapChunk, numberInode, dstNumber);
    }

    Inode parentInode = inodeChunk[parentNumber];
    int len = CalcSize(parentInode.size, inodeTableSize);
    InodeTable tables[len]; ReadInode((char*)tables, &inodeChunk[parentNumber]);

    for (int i = 0, j = 0; i < len - 1;i++) {
        if (tables[i].id == dstNumber) j = 1;
        if (j == 0) continue;
        tables[i] = tables[i + j];
    }

    for (int i = 0; parentInode.blocks[i] != EMPTY; ++i) {
        FreeCell(dataBitmapChunk, numberChunk, parentInode.blocks[i]);
        parentInode.blocks[i] = EMPTY;
    }
    parentInode.size = 0;
    UpdateInodeData(&parentInode, (char*)tables, (len - 1) * inodeTableSize);
    UpdateInode(&parentInode);

    FreeMem(strSplit);
};

void v_open(char* path) {
    char** splitStr = SplitString(path, "/");

    char* fileName = GetFileName(splitStr);
    int inodeParentID = IsPathExist(splitStr, 1, _DIRECTORY);
    int inodeChildID = IsPathExist(splitStr, 0, _FILE);

    if (inodeParentID == FAIL || inodeChildID != FAIL) {
        if (inodeParentID == FAIL) printf("Path doesnt not existed.\n");
        if (inodeChildID != FAIL) printf("File already existesd.\n");

        FreeMem(splitStr);
        free(fileName);
        return;
    }

    Inode child = CreateInode(_FILE), parent = inodeChunk[inodeParentID];

    InodeTable childTable;
    childTable.id = child.id; strcpy(childTable.name, fileName);

    UpdateInode(&child);

    AddLinkToInode(&parent, childTable);
    UpdateInode(&parent);

    FreeMem(splitStr);
    free(fileName);
}
void v_write(char* path, char* content, size_t size) {
    char** splitStr = SplitString(path, "/");
    int dst = IsPathExist(splitStr, 0, _FILE);

    if (dst == FAIL) {
        printf("File doesnt exist: %s\n", path);
        FreeMem(splitStr);
        return;
    }
    Inode inode = inodeChunk[dst];
    UpdateInodeData(&inode, content, size);
    UpdateInode(&inode);

    FreeMem(splitStr);
}

void PrintFileStructure(int inodeNumber, int level) {
    if (level == 0) printf("%d .\n", inodeNumber);
    int len = CalcSize(inodeChunk[inodeNumber].size, inodeTableSize);
    InodeTable tables[len]; ReadInode((char*)tables, &inodeChunk[inodeNumber]);
    for (int i = 0; i < len; ++i) {
        if (inodeNumber == tables[i].id || !strcmp("..", tables[i].name)) continue;

        Inode inode = inodeChunk[tables[i].id];
        printf("%*d. %s%s\n", (level + 1) * 5, tables[i].id, tables[i].name, inode.type == _FILE ? "" : "/");

        if (inode.type == _DIRECTORY) PrintFileStructure(tables[i].id, level + 1);
        else {
            // if (inode.size == 0) continue;
            // char data[inode.size]; ReadInode(data, &inode);
            // _Print(data, inode.size);
            // puts(data);
        }
    }
}
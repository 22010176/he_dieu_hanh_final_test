#include "src/utils.h"
#include "src/types.h"


#include "constant.h"
#include "disk.h"

//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Main program
_flag AlocateMemoryForDisk();
_flag InitBitmap();
_flag CreateRootFolder();
_flag MainProgram();
_flag Clean();
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Folder Implementation
_flag LinkingFolder(Inode* inode, InodeTable* table);
void PrintFolderStructure(_inode number, _size level);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Path Utilities
_inode IsPathExist(char** path);                                   // Parent's inode or FAIL Flag
char* GetFileName(char** path);
_inode IsFileExist(_inode inode, char* folderName);
char* GetParentFolder(char** path);
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
//____________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
// Main Command
_flag MkdirD(char* path);
_inode LinkD(char* _dst, char* _src);
_inode UnLinkD(char* path);

_File OpenF(char* path, char* mode);
_flag WriteF();
_flag ReadF();
_flag SeekF();




int main() {
    AlocateMemoryForDisk();
    InitBitmap();
    CreateRootFolder();

    // MkdirD("c");
    MkdirD("a");
    MkdirD("a/b");
    MkdirD("a/c");
    // MkdirD("a/c");
    // MkdirD("b");

    // PrintFolderStructure(GetRootInode(), 0);


    Clean();
    return 0;
}


_flag MkdirD(char* path) {
    char** f = SplitString(path, "/");
    if (IsPathExist(f) != EMPTY) return EMPTY;
    char** p = SplitStringExceptLast(path, "/");
    char* parentPath = GetParentFolder(f);
    char* fileName = GetFileName(f);
    _t;
    _inode parent = IsPathExist(p);
    _t;

    if (parent == EMPTY || IsPathExist(f) != EMPTY) {
        _t;
        _fm(f); _fm(p);
        free(fileName);
        free(parentPath);
        return FAIL;
    }

    Inode parentInode = ReadInode(parent), childrenInode = CreateInode(DIRECTORY);
    // PrintInode(&childrenInode);

    InodeTable childTable = CreateInodeTable(childrenInode.inode_number, fileName);

    LinkingFolder(&parentInode, &childTable);
    LinkingFolder(&childrenInode, &(InodeTable){.name = ".", .inode_number = childrenInode.inode_number});
    LinkingFolder(&childrenInode, &(InodeTable){.name = "..", .inode_number = parentInode.inode_number});

    WriteInode(&parentInode);
    WriteInode(&childrenInode);

    printf("\n______________________________________________________\n%s   %s\n", parentPath, fileName);
    PrintInodeFromDisk(parentInode.inode_number);
    PrintInodeFromDisk(childrenInode.inode_number);

    _fm(f); _fm(p);
    free(fileName);
    free(parentPath);
    return SUCCESS;
}



// Main program
_flag AlocateMemoryForDisk() {
    *GetDiskP() = _ca(GetDiskSize());
    return SUCCESS;
}
_flag InitBitmap() {
    uint32_t inodeMinNum = 128;

    _size cSize = GetChunkSize();
    _size dSize = GetDiskSize();
    Bitmap* iBitmap = GetInodeBitmapP(), * dBitmap = GetDataBitmapP();

    iBitmap->chunk = 1;
    iBitmap->number = floor(ceil((float)inodeMinNum * _s(Inode) / cSize) * cSize / _s(Inode));
    iBitmap->size = ceil((float)iBitmap->number / 8);

    dBitmap->chunk = ceil((float)iBitmap->size / cSize) + 1;
    dBitmap->number = (float)dSize / cSize - 1 - ceil((float)iBitmap->size / cSize);
    dBitmap->size = ceil((float)dBitmap->number / 8);

    _chunk* iStartChunk = GetInodeStartChunkP(), * dStartChunk = GetDataStartChunkP();
    *iStartChunk = dBitmap->chunk + ceil((float)dBitmap->size / cSize); // Data Bitmap + ChunkDataBitmap;
    *dStartChunk = *iStartChunk + ceil((float)iBitmap->number * _s(Inode) / cSize);

    return SUCCESS;
}
_flag CreateRootFolder() {
    Inode root = CreateInode(DIRECTORY);
    *GetRootInodeP() = root.inode_number;

    LinkingFolder(&root, &(InodeTable) {.inode_number = root.inode_number, .name = "" });
    LinkingFolder(&root, &(InodeTable) {.inode_number = root.inode_number, .name = "." });
    LinkingFolder(&root, &(InodeTable) {.inode_number = root.inode_number, .name = ".." });
    LinkingFolder(&root, &(InodeTable) {.inode_number = root.inode_number, .name = "~" });
    LinkingFolder(&root, &(InodeTable) {.inode_number = root.inode_number, .name = "root" });

    WriteInode(&root);

    return SUCCESS;
}
_flag MainProgram() {
    return SUCCESS;
}
_flag Clean() {
    free(*GetDiskP());
    return SUCCESS;
}



// Folder Implementation
_flag LinkingFolder(Inode* inode, InodeTable* table) {
    _flag status = WriteInodeBlocks(inode, (_data)table, _s(InodeTable));
    if (status == FAIL) return FAIL;

    Inode linkInode = ReadInode(table->inode_number);

    if (table->inode_number == inode->inode_number) ++inode->link;
    else ++linkInode.link;

    WriteInode(&linkInode);

    return SUCCESS;
}
void PrintFolderStructure(_inode number, _size level) {
    Inode inode = ReadInode(number);

    PrintInodeFromDisk(number);

    if (level == 0) printf("%d. %s\n", inode.inode_number, ".");


    _size tableLen = inode.size / _s(InodeTable);
    if (inode.type == FILE || tableLen <= 2) return;

    InodeTable table[tableLen]; CopyM((_data)table, inode.blocks);

    for (_id i = 0; i < tableLen; ++i) {
        if (table[i].inode_number == number || !strcmp(table[i].name, "..")) continue;
        printf("%*d. %s\n", 4 * (level + 1), table[i].inode_number, table[i].name);

        // PrintFolderStructure(table[i].inode_number, level + 1);
    }

}



// Path Utilities
_inode IsPathExist(char** path) {
    _inode res = GetRootInode();
    printf("root: %d\n", res);
    _t;
    for (_id i = 0; res != EMPTY && path[i] != NULL; ++i) {
        printf("%s %d\n", path[i], res);
        res = IsFileExist(res, path[i]);
        _t;
    }
    return res;
}                                   // Parent's inode or FAIL Flag
char* GetFileName(char** path) {
    for (_id i = 0; path[i] != NULL; ++i)
        if (path[i + 1] == NULL) return strcpy(_ma(strlen(path[i]) + 1), path[i]);
    return "";
}
_inode IsFileExist(_inode inode, char* folderName) {
    Inode data = ReadInode(inode);
    printf(" a: %d\n", inode);

    _size size = ceil((float)SizeM(data.blocks) / _s(InodeTable));
    InodeTable table[size]; CopyM((_data)table, data.blocks);

    for (_id i = 0; i < size; i++)
        if (!strcmp(folderName, table[i].name)) return table[i].inode_number;

    return EMPTY;
}
char* GetParentFolder(char** path) { return JoinStringExceptLast(_ca(SizeStringArr(path)), path, "/"); }





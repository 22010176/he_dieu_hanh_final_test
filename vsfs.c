#include "src/types.h"
#include "src/utils.h"
#include "src/bitmap.h"

// gcc src/bitmap.c src/utils.c vsfs.c -o ./out/vsfs
// ./out/vsfs

size_t chunkSize = 2048;
size_t diskSize = 1024 * 1024;
size_t inodeSize = sizeof(Inode);
size_t inodeTableSize = sizeof(InodeTable);
size_t size_tSize = sizeof(size_t);
size_t intSize = sizeof(int);
size_t charSize = sizeof(char);

char* data;
size_t numberInode, numberChunk;
char* inodeBitmapChunk, * dataBitmapChunk, * dataChunk;
Inode* inodeChunk;
int rootInode;

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
int FreeInode(Inode* inode);
void DeleteInodeTable(Inode* inode, size_t id);

int IsThisFileInFolder(Inode* inode, char* path, int type);
int IsPathExist(char** p, int level, int type);

void v_mkdir(char* path);
void v_link(char* _dst, char* _src);
void v_unlink(char* dst);
void v_open(char* path);
int v_write(char* path, char* content, size_t size);

void PrintFileStructure(int inodeNumber, int level);
void PrintVSFS();


int main() {
  InitParam();
  InitFolder();

  v_mkdir("a");
  v_mkdir("b");

  v_mkdir("c");
  v_mkdir("c/a");

  v_mkdir("c/g");
  v_mkdir("c/c");

  v_link("a/bd", "b");
  v_unlink("c");

  PrintFileStructure(rootInode, 0);
  PrintVSFS();

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
  AddLinkToInode(&root,
    (InodeTable) {
    .id = rootInode, .name = ".."
  });
  AddLinkToInode(&root,
    (InodeTable) {
    .id = rootInode, .name = "."
  });
  AddLinkToInode(&root,
    (InodeTable) {
    .id = rootInode, .name = "root"
  });
  AddLinkToInode(&root,
    (InodeTable) {
    .id = rootInode, .name = ""
  });

  UpdateInode(&root);
}
int GetFreePointer(Inode* inode) {
  for (int i = 0; i < MaxPointers; ++i)
    if (inode->blocks[i] == EMPTY) return i;
  return MaxPointers;
}
char* ReadInode(char* _dst, Inode* inode) {
  if (CheckCell(inodeBitmapChunk, numberInode, inode->id) == 0)
    return NULL;
  int i = 0, offset = 0;
  for (int temp = inode->size; i < MaxPointers;++i) {
    if (inode->blocks[i] == EMPTY) break;

    size_t readSize = min(chunkSize, temp);
    memcpy(
      _dst + offset,
      dataChunk + inode->blocks[i] * chunkSize,
      readSize
    );
    offset += readSize; temp -= readSize;
  }
  return _dst;
}

void PrintInodeInDisk(int inodeNum) {
  if (CheckCell(inodeBitmapChunk, numberInode, inodeNum) == 0)
    return;
  printf("\n\n");

  Inode inode = inodeChunk[inodeNum];
  printf("ID: %-3d TYPE: %-10s LINK: %-5d SIZE: %d\n",
    inode.id, inode.type == _FILE ? "File" : "Directory",
    inode.link, inode.size);

  for (int i = 0; i < MaxPointers; ++i)
    printf("%2d ", inode.blocks[i]);

  if (inode.type == _FILE) return;
  printf("\n");

  int len = CalcSize(inode.size, inodeTableSize);
  InodeTable tables[CalcSize(inode.size, inodeTableSize)];
  ReadInode((char*)tables, &inode);

  for (int i = 0; i < len; ++i)
    printf("| %-5d %30s |\n", tables[i].id, tables[i].name);
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
    printf("Invalid inode.\n");
    return (Inode) { .id = -1 };
  }

  Inode inode = { .id = id ,.type = type,.link = 0,.size = 0 };
  for (int i = 0; i < MaxPointers; ++i)
    inode.blocks[i] = EMPTY;

  UpdateInode(&inode);
  return inode;
}

void PrintInode(Inode* inode) {
  if (CheckCell(inodeBitmapChunk, numberInode, inode->id) == 0)
    return;
  printf("\n\n");

  printf("ID: %-3d TYPE: %-10s LINK: %-5d SIZE: %d\n",
    inode->id, inode->type == _FILE ? "File" : "Directory",
    inode->link, inode->size);

  for (int i = 0; i < MaxPointers; ++i)
    printf("%2d ", inode->blocks[i]);

  if (inode->type == _FILE) return;
  printf("\n");

  int len = CalcSize(inode->size, inodeTableSize);
  InodeTable tables[CalcSize(inode->size, inodeTableSize)];
  ReadInode((char*)tables, inode);
  for (int i = 0; i < len; ++i)
    printf("| %-5d %30s |\n", tables[i].id, tables[i].name);
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
int IsPathExist(char** p, int level, int type) {
  int inodeID = rootInode;
  for (int i = 0; p[i + level] != NULL; ++i) {
    inodeID = IsThisFileInFolder(
      &inodeChunk[inodeID], p[i], p[i + level + 1] == NULL ? type : _ANYTYPE
    );
    if (inodeID == FAIL) break;
  }
  return inodeID;
}
int UpdateInodeData(Inode* inode, char* data, size_t size) {
  if (CheckCell(inodeBitmapChunk, numberInode, inode->id) == 0)
    return FAIL;
  int sizeRemain = max(
    CalcSize(inode->size, chunkSize) * chunkSize - inode->size, 0
  );
  size_t temp = size, offset = 0;

  int pointer = GetFreePointer(inode);
  if (sizeRemain > 0 && pointer > 0) {
    size_t writeSize = min(sizeRemain, size);
    memcpy(
      dataChunk + inode->blocks[pointer - 1] * chunkSize
      + inode->size % chunkSize,
      data, writeSize);

    inode->size += writeSize;
    temp -= writeSize; offset += writeSize;
  }

  if (pointer == MaxPointers) {
    printf("Cant not find free pointer\n");
    return FAIL;
  }

  while (temp > 0) {
    pointer = GetFreePointer(inode);
    if (pointer >= MaxPointers) {
      printf("No more pointer.\n");
      return FAIL;
    }
    int freeChunk = GetFreeCell(dataBitmapChunk, numberChunk);
    if (freeChunk == FAIL) {
      printf("Run out of free chunk!!!\n");
      return FAIL;
    }
    size_t writeSize = min(chunkSize, size);
    memcpy(dataChunk + freeChunk * chunkSize, data + offset, writeSize);

    temp -= writeSize; offset += writeSize; inode->size += writeSize;
    inode->blocks[pointer] = freeChunk;

  }
  return SUCCESS;
}
int FreeInode(Inode* inode) {
  if (numberInode <= inode->id
    || !CheckCell(inodeBitmapChunk, numberInode, inode->id)) {
    pf("Invalid Inode num!\n");
    return FAIL;
  }

  if (inode->type == _DIRECTORY) {
    size_t len = CalcSize(inode->size, inodeTableSize);
    InodeTable tables[len]; ReadInode((char*)tables, inode);
    for (int i = 0; i < len;++i) {
      if (!strcmp(tables[i].name, "..")
        || !strcmp(".", tables[i].name)
        || tables[i].id == inode->id)
      {
        continue;
      }

      Inode child = inodeChunk[tables[i].id];
      FreeInode(&child);
    }
  }
  for (int i = 0; inode->blocks[i] != EMPTY && i < MaxPointers; ++i)
    FreeCell(dataBitmapChunk, numberChunk, inode->blocks[i]);

  UpdateInode(inode);
  FreeCell(inodeBitmapChunk, numberInode, inode->id);

  return SUCCESS;
}
int IsThisFileInFolder(Inode* inode, char* path, int type) {
  int len = CalcSize(inode->size, inodeTableSize);
  InodeTable table[len]; ReadInode((char*)table, inode);

  for (int i = 0; i < len; ++i) {
    if (strcmp(table[i].name, path)) continue;
    if (inodeChunk[table[i].id].type == type || type == _ANYTYPE)
      return table[i].id;
  }

  return FAIL;
}



void DeleteInodeTable(Inode* inode, size_t id) {
  int len = CalcSize(inode->size, inodeTableSize);
  InodeTable tables[len]; ReadInode((char*)tables, inode);

  for (int i = 0, j = 0; i < len - 1;i++) {
    if (tables[i].id == id) j = 1;
    if (j == 0) continue;
    tables[i] = tables[i + j];
  }

  for (int i = 0; inode->blocks[i] != EMPTY; ++i) {
    FreeCell(dataBitmapChunk, numberChunk, inode->blocks[i]);
    inode->blocks[i] = EMPTY;
  }

  inode->size = 0;
  UpdateInodeData(inode, (char*)tables, (len - 1) * inodeTableSize);
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





void v_mkdir(char* path) {
  char** splitStr = SplitString(path, "/");

  char* fileName = GetFileName(splitStr);
  int inodeParentID = IsPathExist(splitStr, 1, _DIRECTORY);
  int inodeChildID = IsPathExist(splitStr, 0, _DIRECTORY);

  if (inodeParentID == FAIL || inodeChildID != FAIL) {
    if (inodeParentID == FAIL) printf("Path doesnt not existed.\n");
    if (inodeChildID != FAIL) printf("File already existesd.\n");
    FreeMem(splitStr);
    free(fileName);
    return;
  }
  Inode child = CreateInode(_DIRECTORY);
  Inode parent = inodeChunk[inodeParentID];
  if (child.id == FAIL) {
    pf("Runnout of inode.\n");
    return;
  }
  InodeTable childTable;
  childTable.id = child.id; strcpy(childTable.name, fileName);

  AddLinkToInode(&child, (InodeTable) {
    .id = parent.id, .name = ".."
  });
  AddLinkToInode(&child, (InodeTable) {
    .id = child.id, .name = "."
  });
  UpdateInode(&child);

  AddLinkToInode(&parent, childTable);
  UpdateInode(&parent);

  FreeMem(splitStr);
  free(fileName);
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
  FreeInode(&child);

  Inode parentInode = inodeChunk[parentNumber];
  DeleteInodeTable(&parentInode, dstNumber);

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
  Inode child = CreateInode(_FILE);
  Inode parent = inodeChunk[inodeParentID];
  InodeTable childTable;
  childTable.id = child.id; strcpy(childTable.name, fileName);
  UpdateInode(&child);
  AddLinkToInode(&parent, childTable);
  UpdateInode(&parent);

  FreeMem(splitStr);
  free(fileName);
}
int v_write(char* path, char* content, size_t size) {
  char** splitStr = SplitString(path, "/");
  int dst = IsPathExist(splitStr, 0, _FILE);

  if (dst == FAIL) {
    printf("File doesnt exist: %s\n", path);
    FreeMem(splitStr);
    return FAIL;
  }
  Inode inode = inodeChunk[dst];
  int success = UpdateInodeData(&inode, content, size);
  UpdateInode(&inode);

  FreeMem(splitStr);
  return success;
}
void PrintFileStructure(int inodeNumber, int level) {
  if (level == 0) printf("%d .\n", inodeNumber);
  int len = CalcSize(inodeChunk[inodeNumber].size, inodeTableSize);
  InodeTable tables[len];
  ReadInode((char*)tables, &inodeChunk[inodeNumber]);

  for (int i = 0; i < len; ++i) {
    if (inodeNumber == tables[i].id
      || !strcmp("..", tables[i].name))
      continue;

    if (tables[i].id >= numberInode
      || CheckCell(inodeBitmapChunk, numberInode, tables[i].id) == 0)
      continue;

    Inode inode = inodeChunk[tables[i].id];
    printf("%*d. %s%s\n",
      (level + 1) * 5,
      tables[i].id,
      tables[i].name,
      inode.type == _FILE ? "" : "/"
    );

    if (inode.type == _DIRECTORY)
      PrintFileStructure(tables[i].id, level + 1);
  }
}


void PrintVSFS() {
  pf("\nVSFS data:\n");

  pf("\ninode bitmap: ");
  _Print(inodeBitmapChunk, CalcSize(numberInode, 8));

  pf("\ndata bitmap: ");
  _Print(dataBitmapChunk, CalcSize(numberChunk, 8));

  pf("\nInode: \n");
  for (int i = 0; i < numberInode; ++i) {
    if (CheckCell(inodeBitmapChunk, numberInode, i) == 0) continue;

    Inode inode = inodeChunk[i];
    printf("\n[ id: %-4d type: %-5s blocks: [ ",
      inode.id, inode.type == _FILE ? "file" : "dir");

    for (int j = 0; inode.blocks[j] != EMPTY && j < MaxPointers; ++j)
      printf("%d ", inode.blocks[j]);
    printf("],");
  }

  pf("\n\nData: \n");
  for (int i = 0; i < numberInode; ++i) {
    if (CheckCell(inodeBitmapChunk, numberInode, i) == 0) continue;
    Inode inode = inodeChunk[i];
    if (inode.type == _FILE) {
      pf("Inode %d: [...]\n", i);
      continue;
    }
    size_t len = CalcSize(inode.size, inodeTableSize);
    InodeTable* tables = _ma(inode.size);
    ReadInode((char*)tables, &inode);

    pf("Inode %d: [ ", i);
    for (int j = 0; j < len;++j)
      pf("(%d, %s), ", tables[j].id, tables[j].name);

    pf("]\n");
    free(tables);
  }
}

#include "utils.h"
#include "types.h"

char* data; // 4kb

size_t chunkSize = 512;
size_t diskSize = 256 * 1024;
size_t inodeSize = sizeof(Inode);
size_t inodeTableSize = sizeof(InodeTable);
size_t size_tSize = sizeof(size_t);
size_t intSize = sizeof(int);
size_t charSize = sizeof(char);

size_t numberInode, numberChunk;

char* inodeBitmapChunk, * dataBitmapChunk, * dataChunk;
Inode* inodeChunk;

int rootInode;

Super vss[32];

void InitParam() {
    data = malloc(diskSize);
}
void InitFolder() {

}


// int main() {

// }
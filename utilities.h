#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <time.h>

#define _s fflush(stdin); scanf
#define _rand(x) rand() % x + 1
#define _randRange(start, end, step) _rand((int)((end - start) / step)) * step + start
#define _for(start, end, step) for (uint32_t i = start; i < end; i += step)

// Kich thuoc: 4 + 4 + 4 + 4 + 4 + 4 = 4 * 6 = 24 byte
typedef struct Super Super;
struct Super {
    uint32_t id;                                // ma id
    uint32_t size;                              // bytes
    uint32_t inode_num;                         // so inode
    uint32_t chunk_size;                        // kich thuoc chunk
    uint32_t block;                             // Kich thuoc bo nho nho nhat co the doc
    uint32_t inode_address;                     // dia chi inode dau tien
};

// Kich thuoc: 1 + 15 * 4 + 4 = 65 byte
typedef struct Inode Inode;
struct Inode {
    char type;                                  // Kieu file (D: directory, F: file)
    uint32_t links;                             // Lien ket toi file
    uint32_t blocks[15];                        // Con tro du lieu (12 direct + 1 1-level + 1 2-level + 1 3-level)
};

// Kich thuoc: 60 + 4 = 64 byte
typedef struct InodeData InodeData;
struct InodeData {
    char name[60];                              // Ten file hoac thu muc
    uint32_t inode_id;                          // ma inode
};

// kich thuoc: 1 chunk
typedef struct Bitmap Bitmap;
struct Bitmap { uint8_t* bitmap; };             // bitmap quan ly bo nho trong

// kich thuoc: ko xac dinh
typedef union Chunk Chunk;
union Chunk {};

typedef struct Disk Disk;
struct Disk {
    char* data;
    uint32_t pos;
};

#endif
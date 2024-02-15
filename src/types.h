#pragma once

#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <limits.h>

#define _s                                              sizeof
#define _rand(x)                                        rand() % x + 1
#define _rr(start, end, step)                           _rand((int)((end - start) / step)) * step + start
#define _fm(x)                                          for (int i = 0; x[i] != NULL; free(x[i++])); free(x)

#define max(a, b)                                       a > b ? a : b
#define min(a, b)                                       a < b ? a : b

#define class(_c)                                       typedef struct _c _c; struct _c
#define test(x)                                         printf("\n%s#%d %s\n", __FILE__, __LINE__, x)
#define _t                                              ; test("testing")

#define FAIL                                            UINT_MAX
#define EMPTY                                           UINT_MAX
#define SUCCESS                                         1

#define DIRECTORY                                       1
#define FILE                                            2

#define INODE_MAX_POINTER                               11
#define MAX_FILE_NAME_LENGTH                            28


// TYPES define
#define _id                                             uint32_t
#define _flag                                           uint32_t
#define _file                                           uint32_t
#define _chunk                                          uint32_t
#define _inode                                          uint32_t
#define _type                                           uint32_t

#define _size                                           uint32_t

#define _physic                                         uint8_t*
#define _logic                                          uint32_t

#define _bool                                           uint32_t
#define _data                                           uint8_t*
#define _byte                                           uint8_t

class(Bitmap) { uint32_t size, chunk, number; };

class(Inode) {
    _inode inode_number;                              // Inode's number        
    _type type;                                      // File type (1 = D, 2 = F)
    _size size;                                      // Size of the file
    _size link;                                      // Count links
    _chunk blocks[INODE_MAX_POINTER + 1];                                // Data's address (12 direct pointer)
    // 4 + 4 + 4 * 12 + 4 + 4 = 64 byte
};

class(Super) {
    _physic disk;
    _size diskSize, chunkSize;
    Bitmap inodeBm, dataBm;
    _chunk iStartChunk, dStartChunk;
    _inode rootInode;
};

class(InodeTable) {
    _inode inode_number;
    char name[MAX_FILE_NAME_LENGTH];
};

class(Buffer) {
    _size size, offset;
    _data data;
};

class(_File) {
    _size offset;
    Inode inode;
    Buffer* buffer;
};

#ifndef _AAAAA_
#define _AAAAA_



#endif


#endif
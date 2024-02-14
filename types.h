#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

#define _s                                              sizeof
#define _rand(x)                                        rand() % x + 1
#define _rr(start, end, step)                           _rand((int)((end - start) / step)) * step + start

#define _fm(x)                                          for (int i = 0; x[i] != NULL; free(x[i++])); free(x)
#define GenArr(init,end,change)                         for (init; end; change)

#define max(a, b)                                       a > b ? a : b
#define min(a, b)                                       a < b ? a : b

#define class(_c)                                       typedef struct _c _c; struct _c
#define test(x)                                         printf("\n%d %s\n", __LINE__, x)
#define _t                                              ; test("testing")

#define FAIL                                            UINT_MAX
#define EMPTY                                           UINT_MAX
#define SUCCESS                                         1

#define DIRECTORY                                       1
#define FILE                                            2

#define INODE_MAX_POINTER                               12
#define MAX_FILE_NAME_LENGTH                            28


// TYPES define
#define _index                                          uint32_t
#define _inode                                          Inode*
#define _chunk                                          uint8_t  
#define _flag                                           uint32_t
#define _file                                           uint32_t

#define _size                                           uint32_t

#define _physic                                         uint8_t*
#define _logic                                          uint32_t

#define _bool                                           uint32_t
#define _data                                           uint8_t*


class(Bitmap) { uint32_t size, chunk; };

class(Inode) {
    uint32_t inode_number;                              // Inode's number        
    uint32_t type;                                      // File type (1 = D, 2 = F)
    uint32_t size;                                      // Size of the file
    uint32_t link;                                      // Count links
    uint32_t blocks[INODE_MAX_POINTER];                                // Data's address (12 direct pointer)
    // 4 + 4 + 4 * 12 + 4 + 4 = 64 byte
};

class(InodeTable) {
    uint32_t inode_number;
    char name[MAX_FILE_NAME_LENGTH];
};


#endif
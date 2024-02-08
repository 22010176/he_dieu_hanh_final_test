#ifndef __VSFS_H__
#define __VSFS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "utilities.h"

Super CreateSuper(uint32_t id, uint32_t size, uint32_t inode_num, uint32_t chunk_size, uint32_t block);
Super InputSuper();
Super GenSuper();
void PrintSuper(Super super);

void* v_mkdir(Super super, const char* path, char* data);
void* v_creat(const char* path);
char* v_read(const char* path);
void* v_write(const char* path, char* data);
bool v_unlink(char* path);
bool v_link(char* src, char* dst);

#endif

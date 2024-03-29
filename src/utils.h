#pragma once

#ifndef _UTILS_H_
#define _UTILS_H_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <conio.h>
#include <math.h>


// Memory Allocate
void* _ma(size_t size);
void* _ca(size_t size);
void* _re(void* p, size_t size);


// Utilities
char** SplitString(char* string, char* p);
char** SplitStringExceptLast(char* string, char* p);
uint32_t SizeStringArr(char** string);
char* JoinString(char* result, char** string, char* add);
char* JoinStringExceptLast(char* result, char** string, char* add);
char* GetFileName(char** path);
uint8_t ReadBit(uint8_t x, uint8_t bit);
void _Print(char* address, size_t size);

#endif
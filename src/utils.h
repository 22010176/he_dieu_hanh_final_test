#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <cmath>

std::vector<std::string> SplitString(const std::string& src, const std::string& del, uint32_t skip = 0);
uint32_t ReadBit(uint32_t byte, uint32_t bit);
void PrintMem(uint8_t* data, size_t size);
size_t CalcSize(size_t size1, size_t size2);
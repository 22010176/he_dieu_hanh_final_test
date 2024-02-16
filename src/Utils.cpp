#include "utils.h"

std::vector<std::string> SplitString(const std::string& src, const std::string& del, uint32_t skip) {
    int start, end = -1 * del.size();
    std::vector <std::string> result;
    do {
        start = end + del.size();
        end = src.find(del, start);
        result.push_back(src.substr(start, end - start));
    } while (end != -1);

    for (uint32_t i = 0; i < skip; ++i) result.pop_back();

    return result;
}
uint32_t ReadBit(uint32_t byte, uint32_t bit) { return (byte & (1 << bit)) >> bit; }
void PrintMem(uint8_t* data, size_t size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 8; ++j) std::cout << ReadBit(data[i], j);
        if ((i + 1) % 8 == 0) std::cout << std::endl;
        else std::cout << " ";
    }
}
size_t CalcSize(size_t size1, size_t size2) { return (size_t)ceil((float)size1 / size2); }
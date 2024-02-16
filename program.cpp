#include <bits/stdc++.h>

#include "src/utils.h"
#include "src/StorageManagement.h"
#include "src/Disk.h"
#include "src/Test.h"

int main() {
    Test::_StorageManagement1();

    StorageManagement storage(2048, 1024);
    size_t len = 300;
    uint32_t x[len];

    // PrintMem(storage.GetStorage(), storage.GetDiskSize());
    for (int i = 0; i < len; ++i) x[i] = (i + 1);

    uint32_t chunk = storage.WriteS((uint8_t*)x, sizeof(x));
    uint32_t* rest = (uint32_t*)storage.ReadS(chunk);

    for (int i = 0; i < storage.SizeS(chunk) / 4; ++i) {
        if (rest[i] == x[i]) continue;
        std::cout << rest[i] << " ";
    }
    // storage.GetBitmap().Print();

    storage.PrintS(chunk);
    delete[] rest;
    return 0;
}
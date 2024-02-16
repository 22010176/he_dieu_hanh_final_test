#include "Test.h"

void Test::_Bitmap() {
    uint8_t data[24];
    Bitmap a(data, sizeof(data));

    std::cout << a.GetFreeCell() << std::endl;
    char* x = a.ExportData();

    for (int i = 0; i < a.GetTotalCell(); ++i) a.GetFreeCell();
    a.FreeCell(4);
    a.FreeCell(8);
    a.FreeCell(12);
    a.FreeCell(16);
    a.FreeCell(20);
    a.FreeCell(21);
    a.FreeCell(32);
    a.FreeCell(83);

    Bitmap b(x);


    a.Print();
    b.Print();

    delete[] x;
}
void Test::_Disk() {
    Disk a(1024);
    size_t len = 200;
    uint32_t x[len];
    for (int i = 0; i < len; ++i) x[i] = (i + 1) << 8;
    a.Write(200, (uint8_t*)x, sizeof(x));
    a.Print(200, sizeof(x));

    uint32_t* ab = (uint32_t*)a.Read(200, sizeof(x));

    for (int i = 0; i < len; ++i) std::cout << ab[i] << std::endl;

    a.Free(200, sizeof(x));
    a.Print(200, sizeof(x));

    delete[] ab;
}
void Test::_Inode() {

}
void Test::_Path() {

}
void Test::_StorageManagement1() {
    StorageManagement storage(4096, 512);
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

}
void Test::_Super() {

}
void Test::_Utils() {

}
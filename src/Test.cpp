#include "Test.h"

void Test::_Bitmap() {
    uint8_t data[24];
    Bitmap a(data, sizeof(data));

    std::cout << a.GetFreeCell() << std::endl;
    uint8_t* x = a.ExportData();

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

    uint8_t* sdd = a.ExportRawData();
    uint8_t xee[a.GetBitmapSize()];
    Bitmap c(xee, sdd);


    c.Print();
    a.Print();
    // PrintMem(sdd, 11);
    b.Print();

    delete[] x;
    delete[] sdd;
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
void Test::_Disk2() {
    Disk a(1024);
    size_t len = 200;
    uint32_t x[len];

    for (int i = 0; i < len; ++i) x[i] = (i + 1) << 8;
    a.Write(200, (uint8_t*)x, sizeof(x));

    uint8_t* data = a.ExportData();
    Disk b(data);

    b.Print(200, sizeof(x));
    uint32_t* ab = (uint32_t*)b.Read(200, sizeof(x));
    for (int i = 0; i < len; ++i) std::cout << ab[i] << std::endl;

    delete[] data;
}
void Test::_Inode() {

    Inode a(4, 1);

    a.AddPointer(5);
    a.AddPointer(2);
    a.AddPointer(3);
    a.AddPointer(4);
    a.AddPointer(4);
    a.AddPointer(4);
    a.RemovePointer();
    a.AddPointer(5);
    a.AddPointer(4);
    a.AddPointer(4);

    a.RemovePointer();
    a.RemovePointer();

    uint8_t* data = a.ExportData();

    Inode b(data);

    b.Print();
    a.Print();

    delete[] data;
}
void Test::_InodeTable() {
    InodeTable table(5, "122");

    uint8_t* data = table.ExportData();
    InodeTable d(data);
    d.Print();

    delete[] data;
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
void Test::_StorageManagement2() {
    StorageManagement storage(2048, 256);
    size_t len = 100;
    uint32_t x[len];

    // PrintMem(storage.GetStorage(), storage.GetDiskSize());
    for (int i = 0; i < len; ++i) x[i] = (i + 1);

    std::vector<uint32_t> chunk = storage.WriteM((uint8_t*)x, sizeof(x));
    uint32_t* rest = (uint32_t*)storage.ReadM(chunk);

    for (int i = 0; i < storage.SizeM(chunk) / 4; ++i) {
        // if (rest[i] == x[i]) continue;
        std::cout << rest[i] << " ";
    }
    // storage.GetBitmap().Print();

    storage.PrintM(chunk);
    storage.GetBitmap().Print();

    storage.FreeM(chunk);
    storage.GetBitmap().Print();
    storage.PrintM(chunk);

    delete[] rest;
}
void Test::_StorageManagement3() {
    StorageManagement storage(2048, 256);
    size_t len = 100;
    uint32_t x[len];

    // PrintMem(storage.GetStorage(), storage.GetDiskSize());
    for (int i = 0; i < len; ++i) x[i] = (i + 1);

    std::vector<uint32_t> chunk = storage.WriteM((uint8_t*)x, sizeof(x));

    uint8_t* data = storage.ExportData();
    StorageManagement b(data);
    b.PrintM(chunk);

    uint32_t* rest = (uint32_t*)b.ReadM(chunk);
    std::cout << std::endl;
    for (int i = 0; i < b.SizeM(chunk) / 4; ++i) {
        // if (rest[i] == x[i]) continue;
        std::cout << rest[i] << " ";
    }

    delete[] data;
    delete[] rest;
}
void Test::_Super() {
    Super s(1024, 128, 156);
    s.Print();

    uint8_t* dat = s.ExportData();
    Super a(dat);
    a.Print();

    delete[] dat;
}
void Test::_Utils() {

}
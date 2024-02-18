#include "Test.h"

void Test::_Bitmap() {
    DEBUG(uint8_t data[24]);
    DEBUG(Bitmap a(data, sizeof(data)));

    DEBUG(std::cout << a.GetFreeCell() << std::endl);
    DEBUG(uint8_t * x = a.ExportData());

    DEBUG(for (int i = 0; i < a.GetTotalCell(); ++i) a.GetFreeCell());
    DEBUG(a.FreeCell(4));
    DEBUG(a.FreeCell(8));
    DEBUG(a.FreeCell(12));
    DEBUG(a.FreeCell(16));
    DEBUG(a.FreeCell(20));
    DEBUG(a.FreeCell(21));
    DEBUG(a.FreeCell(32));
    DEBUG(a.FreeCell(83));

    DEBUG(Bitmap b(x));

    DEBUG(uint8_t * sdd = a.ExportRawData());
    DEBUG(uint8_t xee[a.GetBitmapSize()]);
    DEBUG(Bitmap c(xee, sdd));


    DEBUG(c.Print());
    DEBUG(a.Print());
    // DEBUG(// PrintMem(sdd, 11));
    DEBUG(b.Print());

    DEBUG(delete[] x);
    DEBUG(delete[] sdd);
}
void Test::_Disk() {
    DEBUG(Disk a(1024));
    DEBUG(size_t len = 200);
    DEBUG(uint32_t x[len]);
    DEBUG(for (int i = 0; i < len; ++i) x[i] = (i + 1) << 8);
    DEBUG(a.Write(200, (uint8_t*)x, sizeof(x)));
    DEBUG(a.Print(200, sizeof(x)));

    DEBUG(uint32_t * ab = (uint32_t*)a.Read(200, sizeof(x)));

    DEBUG(for (int i = 0; i < len; ++i) std::cout << ab[i] << std::endl);

    DEBUG(a.Free(200, sizeof(x)));
    DEBUG(a.Print(200, sizeof(x)));
    DEBUG(delete[] ab);
}
void Test::_Disk2() {
    DEBUG(Disk a(1024));
    DEBUG(size_t len = 200);
    DEBUG(uint32_t x[len]);

    DEBUG(for (int i = 0; i < len; ++i) x[i] = (i + 1) << 8);
    DEBUG(a.Write(200, (uint8_t*)x, sizeof(x)));

    DEBUG(uint8_t * data = a.ExportData());
    DEBUG(Disk b(data));

    DEBUG(b.Print(200, sizeof(x)));
    DEBUG(uint32_t * ab = (uint32_t*)b.Read(200, sizeof(x)));
    DEBUG(for (int i = 0; i < len; ++i) std::cout << ab[i] << std::endl);

    DEBUG(delete[] data);
}
void Test::_Inode() {

    DEBUG(Inode a(4, 1));

    DEBUG(a.AddPointer(5));
    DEBUG(a.AddPointer(2));
    DEBUG(a.AddPointer(3));
    DEBUG(a.AddPointer(4));
    DEBUG(a.AddPointer(4));
    DEBUG(a.AddPointer(4));
    DEBUG(a.RemovePointer());
    DEBUG(a.AddPointer(5));
    DEBUG(a.AddPointer(4));
    DEBUG(a.AddPointer(4));

    DEBUG(a.RemovePointer());
    DEBUG(a.RemovePointer());
    DEBUG(uint8_t data[Inode::GetExportSize()]);

    DEBUG(Inode b(a.ExportData(data)));

    DEBUG(b.Print());
    DEBUG(a.Print());

    DEBUG(delete[] data);
}
void Test::_InodeTable() {
    DEBUG(InodeTable table(5, "122"));

    DEBUG(uint8_t * data = table.ExportData());
    DEBUG(InodeTable d(data));
    DEBUG(d.Print());

    DEBUG(delete[] data);
}
void Test::_Path() {

}
void Test::_StorageManagement1() {
    DEBUG(StorageManagement storage(4096, 512));
    DEBUG(size_t len = 300);
    DEBUG(uint32_t x[len]);

    // PrintMem(storage.GetStorage(), storage.GetDiskSize());
    DEBUG(for (int i = 0; i < len; ++i) x[i] = (i + 1));

    DEBUG(uint32_t chunk = storage.WriteS((uint8_t*)x, sizeof(x)));
    DEBUG(uint32_t * rest = (uint32_t*)storage.ReadS(chunk));

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
    DEBUG(for (int i = 0; i < len; ++i) x[i] = (i + 1));

    DEBUG(std::vector<uint32_t> chunk = storage.WriteM((uint8_t*)x, sizeof(x)));
    DEBUG(uint32_t * rest = (uint32_t*)storage.ReadM(chunk));

    DEBUG(for (int i = 0; i < storage.SizeM(chunk) / 4; ++i)) {
        // if (rest[i] == x[i]) continue;
        DEBUG(std::cout << rest[i] << " ");
    }
    // storage.GetBitmap().Print();

    DEBUG(storage.PrintM(chunk));
    DEBUG(storage.GetBitmap().Print());

    DEBUG(storage.FreeM(chunk));
    DEBUG(storage.GetBitmap().Print());
    DEBUG(storage.PrintM(chunk));

    DEBUG(delete[] rest);
}
void Test::_StorageManagement3() {
    DEBUG(StorageManagement storage(2048, 256));
    DEBUG(size_t len = 100);
    DEBUG(uint32_t x[len]);

    // PrintMem(storage.GetStorage(), storage.GetDiskSize());
    DEBUG(for (int i = 0; i < len; ++i) x[i] = (i + 1));

    DEBUG(std::vector<uint32_t> chunk = storage.WriteM((uint8_t*)x, sizeof(x)));

    DEBUG(uint8_t * data = storage.ExportData());
    DEBUG(StorageManagement b(data));
    DEBUG(b.PrintM(chunk));

    DEBUG(uint32_t * rest = (uint32_t*)b.ReadM(chunk));
    DEBUG(std::cout << std::endl);
    DEBUG(for (int i = 0; i < b.SizeM(chunk) / 4; ++i)) {
        // if (rest[i] == x[i]) continue;
        DEBUG(std::cout << rest[i] << " ");
    }

    DEBUG(delete[] data);
    DEBUG(delete[] rest);
}
void Test::_StorageManagement3(StorageManagement& a) {
    DEBUG(a.PrintS());
    DEBUG(size_t len = 100);
    DEBUG(uint32_t x[len]);

    // DEBUG(// PrintMem(storage.GetStorage(), storage.GetDiskSize()));
    DEBUG(for (int i = 0; i < len; ++i) x[i] = (i + 1));

    DEBUG(std::vector<uint32_t> chunk = a.WriteM((uint8_t*)x, sizeof(x)));


    DEBUG(uint32_t * rest = (uint32_t*)a.ReadM(chunk));

    DEBUG(for (int i = 0; i < a.SizeM(chunk) / 4; ++i)) {
        // if (rest[i] == x[i]) continue;
        DEBUG(std::cout << rest[i] << " ");
    }
    // a.GetBitmap().Print();

    DEBUG(a.PrintM(chunk));
    DEBUG(a.GetBitmap().Print());

    DEBUG(a.FreeM(chunk));
    DEBUG(a.GetBitmap().Print());
    DEBUG(a.PrintM(chunk));

    DEBUG(uint8_t * data = a.ExportData());
    DEBUG(StorageManagement b(data));
    DEBUG(b.PrintM(chunk));

    DEBUG(rest = (uint32_t*)b.ReadM(chunk));
    DEBUG(std::cout << std::endl);
    DEBUG(for (int i = 0; i < b.SizeM(chunk) / 4; ++i)) {
        // if (rest[i] == x[i]) continue;
        DEBUG(std::cout << rest[i] << " ");
    }

    DEBUG(delete[] data);
    DEBUG(delete[] rest);
}
void Test::_StorageManagement3(StorageManagement*& a) {
    DEBUG(a->PrintS());
    DEBUG(size_t len = 100);
    DEBUG(uint32_t x[len]);

    // PrintMem(storage->GetStorage(), storage->GetDiskSize());
    DEBUG(for (int i = 0; i < len; ++i) x[i] = (i + 1));

    DEBUG(std::vector<uint32_t> chunk = a->WriteM((uint8_t*)x, sizeof(x)));


    DEBUG(uint32_t * rest = (uint32_t*)a->ReadM(chunk));

    DEBUG(for (int i = 0; i < a->SizeM(chunk) / 4; ++i)) {
        // if (rest[i] == x[i]) continue;
        std::cout << rest[i] << " ";
    }
    // a->GetBitmap()->Print();

    DEBUG(a->PrintM(chunk));
    DEBUG(a->GetBitmap().Print());

    DEBUG(a->FreeM(chunk));
    DEBUG(a->GetBitmap().Print());
    DEBUG(a->PrintM(chunk));

    // DEBUG(uint8_t * data = a->ExportData());
    // DEBUG(StorageManagement b(data));
    // DEBUG(b.PrintM(chunk));

    // DEBUG(rest = (uint32_t*)a.ReadM(chunk));
    // DEBUG(std::cout << std::endl);
    // DEBUG(for (int i = 0; i < a.SizeM(chunk) / 4; ++i)) {
    //     // if (rest[i] == x[i]) continue;
    //     DEBUG(std::cout << rest[i] << " ");
    // }

    // // DEBUG(delete[] data);
    // DEBUG(delete[] rest);
}
void Test::_Super() {
    DEBUG(Super s(1024, 128, 156));
    DEBUG(s.Print());

    DEBUG(uint8_t * dat = s.ExportData());
    DEBUG(Super a(dat));
    DEBUG(a.Print());

    DEBUG(delete[] dat);
}
void Test::_Utils() {

}
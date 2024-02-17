#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#include "Bitmap.h"
#include "Disk.h"
#include "Inode.h"
#include "Path.h"
#include "StorageManagement.h"
#include "Super.h"
#include "Utils.h"

struct Test {
    static void _Bitmap();
    static void _Disk();
    static void _Disk2();
    static void _Inode();
    static void _InodeTable();
    static void _Path();
    static void _StorageManagement1();
    static void _StorageManagement2();
    static void _StorageManagement3();
    static void _Super();
    static void _Utils();
};

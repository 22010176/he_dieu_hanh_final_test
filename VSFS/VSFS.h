#pragma once

#include "../src/Super.h"

class VSFS {
private:
public:
    VSFS(Super super);
    VSFS(uint8_t data[24]);
    ~VSFS();
};


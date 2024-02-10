#include "vsfs.h"

Super CreateSuper(uint32_t id, uint32_t size, uint32_t inode_num, uint32_t chunk_size, uint32_t block) {
    return (Super) { id, size, inode_num, chunk_size, block };
}

Super InputSuper() {
    Super result;
    result.id = _rand(1000);

    printf("Nhap vao kich thuoc bo nho: ");
    _s("%d", &result.size);

    printf("Nhap vao so luong inode: ");
    _s("%d", &result.inode_num);

    printf("Nhap vao kich thuoc 1 chunk: ");
    _s("%d", &result.chunk_size);

    printf("Nhap vao kich thuoc bo nho co the doc: ");
    _s("%d", &result.block);

    return result;
}

Super GenSuper() {
    Super result;

    result.id = _rand(1000);
    result.size = _randRange(2048, 1024 * 1024, 1024);          // 2KB -> 1MB 
    result.inode_num = _randRange(4, 64, 4);                    // 4 inode -> 64 inode
    result.chunk_size = _randRange(128, 1024, 64);              // 128B -> 512B
    result.block = _randRange(128, 512, 64);                    // 128B -> 512B

    return result;
}

void PrintSuper(Super super) {
    printf("id:%u\n size:%u\n inode:%u\n chunk size:%u\n block:%u\n block:%u\n", super.id, super.size, super.inode_num, super.chunk_size, super.block);
}

void* v_mkdir(Super super, const char* path, char* data) {}
void* v_creat(const char* path) {}
void* v_write(const char* path, char* data) {}
char* v_read(const char* path) {}
bool v_unlink(char* path) {}
bool v_link(char* src, char* dst) {}

void _main(void* data) {

}
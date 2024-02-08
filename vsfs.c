#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct vsfs vsfs;
struct vsfs {};

typedef struct inode inode;
struct inode {

};

typedef struct chunk chunk;
struct chunk {
    inode inode;
    char data[sizeof(inode)];
};

char storage[1024 * 4 * 64];



void* v_mkdir();
void* v_creat();
void* v_read();
void* v_write();
void v_unlink();
void v_link();

int main() {



    return 0;
}


void* v_mkdir() {  }
void* v_creat() {  }
void* v_read() {  }
void* v_write() {  }
void v_unlink() {  }
void v_link() {  }
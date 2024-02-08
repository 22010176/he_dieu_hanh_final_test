#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vsfs.h"
#include "fss.h"

Super super;
char* data;

int main() {
    srand(time(NULL));

    super = GenSuper();
    data = malloc(super.size);

    char command[100];
    while (1) {
        printf("Nhap lenh: ");
        fflush(stdin); gets(command);

        printf("%s\n", command);

        printf("Thoat? (Y/N)\n");
        _s("%c", command);
        if (command[0] == 'Y' || command[0] == 'y') break;
    }


    free(data);
    return 0;
}


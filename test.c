#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "tarfs.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
        return -1;

    tarfs_init_from_file(argv[1]);

    int fd = tarfs_open("a.text", 0);
    char buf[16];
    tarfs_read(fd, buf, 16);

    printf("Content from tarfs: %s\n", buf);

    return 0;
}

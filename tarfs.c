#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "tarfs.h"

static tarfs_inode_t inodes[MAX_INODE];
static tarfs_fds_t tarfs_fds[MAX_FDS];

static int find_empty_fds()
{
    for (int i = 0; i < MAX_FDS; i++) {
        if (tarfs_fds[i].inode == NULL)
            return i;
    }

    return -1;
}

ssize_t tarfs_init(char *addr)
{
    // initialize fds
    for (int i = 0; i < MAX_FDS; i++) {
        tarfs_fds[i].inode = NULL;
        tarfs_fds[i].cursor = 0;
    }

    // initialize the inode pool
    for (int i = 0; i < MAX_INODE; i++) {
        inodes[i].data = NULL;
    }

    // parse the tarball
    int inode_curr = 0;
    tar_header_t *header = (tar_header_t *) addr;
    while (header->name[0]) {
        int size;
        sscanf(header->size, "%o", &size);

        tarfs_inode_t node = {
            .name = (char *) header->name,
            .size = size,
            .attribute = (char) header->typeflag,
            .data = (uint8_t *) header + 512,
        };

        printf("=== The %dth file ===\n", inode_curr);
        printf(
            "name:\t%s\n"
            "size\t%d\n"
            "type\t'%c'\n",
            header->name, size, header->typeflag);

        inodes[inode_curr++] = node;

        if (size > 0) {
            header += (size / 512) + 1;
        }
        header++;
    }
}

ssize_t tarfs_init_from_file(const char *path)
{
    // initialize fds
    for (int i = 0; i < MAX_FDS; i++) {
        tarfs_fds[i].inode = NULL;
        tarfs_fds[i].cursor = 0;
    }

    // initialize the inode pool
    for (int i = 0; i < MAX_INODE; i++) {
        inodes[i].data = NULL;
    }

    // parse the tarball
    int fd = open(path, O_RDONLY);

    int inode_curr = 0;
    tar_header_t header;
    while (read(fd, &header, 512) && header.name[0]) {
        int size;
        sscanf(header.size, "%o", &size);

        tarfs_inode_t node = {
            .name = strdup(header.name),
            .size = size,
            .attribute = header.typeflag,
            .data = malloc(size),
        };

        read(fd, node.data, size);

        printf("=== The %dth file ===\n", inode_curr);
        printf(
            "name:\t%s\n"
            "size\t%d\n"
            "type\t'%c'\n",
            header.name, size, header.typeflag);

        inodes[inode_curr++] = node;
    }
}

ssize_t tarfs_open(const char *path, int flags)
{
    tarfs_inode_t *node = NULL;
    for (int i = 0; i < MAX_FDS; i++) {
        if (!strcmp(path, inodes[i].name)) {
            node = &inodes[i];
            break;
        }
    }

    if (node == NULL)
        return -1;

    int idx = find_empty_fds();
    if (idx < 0)
        return -1;

    tarfs_fds[idx].inode = node;
    tarfs_fds[idx].cursor = 0;

    return idx;
}

ssize_t tarfs_read(int fd, void *buf, size_t count)
{
    tarfs_fds_t *f = &tarfs_fds[fd];
    if (f->inode == NULL)
        return -1;

    size_t size = f->inode->size;

    if ((f->cursor + count) > size)
        count = size - f->cursor;

    memcpy(buf, f->inode->data + f->cursor, count);
    f->cursor += count;

    return count;
}

ssize_t tarfs_write(int fd, void *buf, size_t count)
{
    tarfs_fds_t *f = &tarfs_fds[fd];
    if (f->inode == NULL)
        return -1;

    size_t size = f->inode->size;

    if ((f->cursor + count) > size)
        count = size - f->cursor;

    memcpy(f->inode->data + f->cursor, buf, count);
    f->cursor += count;

    return count;
}

ssize_t tarfs_seek(int fd, off_t offset, int whence)
{
    tarfs_fds_t *f = &tarfs_fds[fd];
    if (f->inode == NULL)
        return -1;

    uint32_t size = f->inode->size;
    uint32_t origin;

    switch (whence) {
    case SEEK_SET:
        origin = 0;
        break;
    case SEEK_CUR:
        origin = f->cursor;
        break;
    case SEEK_END:
        origin = size;
        break;
    default:
        origin = -1;
    }

    offset += origin;

    if (offset < 0)
        return -1;
    if (offset > size)
        offset = size;

    f->cursor = offset;

    return offset;
}

ssize_t tarfs_close(int fd)
{
    tarfs_fds_t *f = &tarfs_fds[fd];
    f->inode = NULL;
    f->cursor = 0;

    return 0;
}

#ifndef TARFS_H
#define TARFS_H

#include <stdint.h>

typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char padding[255];
} tar_header_t;

#define MAX_INODE 16
#define MAX_FDS 16

typedef struct {
    char *name;
    size_t size;
    char attribute;
    uint8_t *data;
} tarfs_inode_t;

typedef struct {
    tarfs_inode_t *inode;
    uint32_t cursor;
} tarfs_fds_t;

ssize_t tarfs_init(char *addr);
ssize_t tarfs_init_from_file(const char *path);

ssize_t tarfs_open(const char *path, int flags);
ssize_t tarfs_read(int fd, void *buf, size_t count);
ssize_t tarfs_write(int fd, void *buf, size_t count);
ssize_t tarfs_seek(int fd, off_t offset, int whence);
ssize_t tarfs_close(int fd);

#endif /* TARFS_H */

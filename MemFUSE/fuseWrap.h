#ifndef FUSEWRAP_H
#define FUSEWRAP_H

#define FUSE_USE_VERSION 26

#include "memfs.h"

extern "C" {
#include <fuse.h>
#include <fcntl.h>
}
#include <cerrno>

int memfs_getattr(const char *path, struct stat *buf);
int memfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t off, struct fuse_file_info *fi);
int memfs_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi);
int memfs_open(const char *path, struct fuse_file_info *fi);
void * memfs_init(struct fuse_conn_info *conn);

extern struct fuse_operations memfs_oper;

#endif // FUSEWRAP_H

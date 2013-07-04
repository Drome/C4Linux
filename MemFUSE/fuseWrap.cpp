#include "fuseWrap.h"

struct fuse_operations memfs_oper;

int memfs_getattr(const char *path, struct stat *buf) {
    return ((MemFUSE::MemFS *) (fuse_get_context()->private_data))->getattr(path, buf);
}

int memfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t off, struct fuse_file_info *fi) {
    return ((MemFUSE::MemFS *) (fuse_get_context()->private_data))->readdir(path, buf, filler, off, fi);
}

int memfs_read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi) {
    return ((MemFUSE::MemFS *) (fuse_get_context()->private_data))->read(path, buf, size, off, fi);
}

int memfs_open(const char *path, struct fuse_file_info *fi) {
    return ((MemFUSE::MemFS *) (fuse_get_context()->private_data))->open(path, fi);
}

void * memfs_init(struct fuse_conn_info *conn) {
    (void) conn; // Now I'm "using" conn...
    return fuse_get_context()->private_data;
}

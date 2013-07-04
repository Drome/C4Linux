#ifndef MEMFS_H
#define MEMFS_H

#define FUSE_USE_VERSION 26

#include <cstring>
#include <cstdlib>
extern "C" {
#include <unistd.h>
#include <fuse.h>
}

#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <exception>

namespace MemFUSE {
    void init(); // Still no designated initializers in C++0x...

    class bad_init : std::exception {
        virtual const char* what() const noexcept { return "MemFS not initialized."; }
    };

    class bad_mount : std::exception {
        virtual const char* what() const noexcept { return "Mounting failed."; }
    };

    class bad_fuse : std::exception {
        virtual const char* what() const noexcept { return "Could not create fuse handle."; }
    };

    class bad_path : std::exception {
        virtual const char* what() const noexcept { return "Invalid path."; }
    };

    enum class FSResourceType {
        FILE,
        DIR
    };

    class File;
    class Directory;

    struct FSResource {
        union {
            Directory * dir;
            File * file;
        };
        FSResourceType fstype;
    };

    class File {
    public:
        File() : __name(""), __size(0), __offset(0), __buff(nullptr) {}
        File(std::string name) : __name(name) {}
        File(std::string name, void *buff, size_t size, size_t offset = 0) : __name(name), __size(size), __offset(offset), __buff(buff) {}

        std::string getName() { return __name; }
        size_t getSize() { return __size; }
        void setName(const std::string& n) { __name = n; }
        void setBuffer(void * b, size_t s) { __buff = b; __size = s; __offset = 0; }

        size_t read(void *dst, size_t nbytes) { nbytes = std::min(nbytes, __size - __offset); memcpy(dst, (char *)__buff+__offset, nbytes); __offset += nbytes; return nbytes; }
        size_t pread(void *dst, size_t nbytes, size_t offset) { nbytes = std::min(nbytes, __size - offset); memcpy(dst, (char *)__buff+offset, nbytes); return nbytes; }

    private:
        std::string __name;
        size_t __size;
        size_t __offset;
        void *__buff;
    };

    class Directory {
    public:
        Directory() : __name("") {}
        Directory(std::string name) : __name(name) {}

        std::string getName() { return __name; }

        void addFile(std::unique_ptr<File> &file); // Transferring ownership of file!
        void addFile(std::unique_ptr<File> &&file);
        void removeFile(std::string name);
        File * getFile(std::string name);
        std::vector<std::unique_ptr<File>>::const_iterator getFileIterator() { return __files.begin(); } // No auto + decltype because of constness... sad...
        size_t getFileCount() { return __files.size(); }

        void addDir(std::unique_ptr<Directory> &dir);
        void addDir(std::unique_ptr<Directory> &&dir);
        void removeDir(std::string name);
        Directory * getDir(std::string name);
        std::vector<std::unique_ptr<Directory>>::const_iterator getDirIterator() { return __dirs.begin(); }
        size_t getDirCount() { return __dirs.size(); }

    private:
        std::vector<std::unique_ptr<File>> __files;
        std::vector<std::unique_ptr<Directory> > __dirs;
        std::string __name;
    };

    class MemFS {
    public:
        MemFS(std::string mountpoint);
        ~MemFS();

        void mount();
        void unmount();

        Directory * getRoot() { return rootDir.get(); }

        int getattr(const char *path, struct stat *buf);
        int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t off, struct fuse_file_info *fi);
        int read(const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fi);
        int open(const char *path, struct fuse_file_info *fi);

        FSResource resolvePath(const std::string &path);

    private:
        bool fused;
        std::string mountpath;
        std::unique_ptr<Directory> rootDir;
        struct fuse *fuse;
        struct fuse_chan *ch;
        std::thread * fuse_thread;
    };
}

#endif // MEMFS_H

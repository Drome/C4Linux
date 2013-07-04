#include "memfs.h"
#include "fuseWrap.h"

namespace MemFUSE {
    static bool fsInitialized = false;

    void init() {
        memfs_oper.getattr = memfs_getattr;
        memfs_oper.readdir = memfs_readdir;
        memfs_oper.read = memfs_read;
        memfs_oper.open = memfs_open;
        memfs_oper.init = memfs_init;
        fsInitialized = true;
    }

    void Directory::addFile(std::unique_ptr<File> &file) {
        __files.push_back(std::move(file));
    }

    void Directory::addFile(std::unique_ptr<File> &&file) {
        __files.push_back(std::forward<std::unique_ptr<File>>(file));
    }

    void Directory::removeFile(std::string name) {
        auto it = __files.begin();
        auto end = __files.end();
        while(it != end) {
            if((*it)->getName() == name) { __files.erase(it); return; }
            it++;
        }
    }

    File * Directory::getFile(std::string name) {
        for(auto &f : __files) { // Ooooooo, range-based for-loops!
            if(f->getName() == name) return f.get();
        }
        return nullptr;
    }

    // Directory

    void Directory::addDir(std::unique_ptr<Directory> &dir) {
        __dirs.push_back(std::move(dir));
    }

    void Directory::addDir(std::unique_ptr<Directory> &&dir) {
        __dirs.push_back(std::forward<std::unique_ptr<Directory>>(dir));
    }

    void Directory::removeDir(std::string name) {
        auto it = __dirs.begin();
        auto end = __dirs.end();
        while(it != end) {
            if((*it)->getName() == name) { __dirs.erase(it); return; }
            it++;
        }
    }

    Directory * Directory::getDir(std::string name) {
        for(auto &d : __dirs) {
            if(d->getName() == name) return d.get();
        }
        return nullptr;
    }

    // MemFS

    MemFS::MemFS(std::string mountpoint) : mountpath(mountpoint), rootDir(new Directory()), fuse_thread(nullptr) {
        if(!fsInitialized) {
            throw bad_init();
        }
        struct fuse_args args = {0,0,0};
        ch = fuse_mount(mountpath.c_str(), &args);
        if(!ch) {
            throw bad_mount();
        }
        fuse = fuse_new(ch, &args, &memfs_oper, sizeof(memfs_oper), this);
        if(!fuse) {
            fuse_unmount(mountpath.c_str(), ch);
            throw bad_fuse();
        }
        fused = true;
    }

    MemFS::~MemFS() {
        unmount();
        if(fuse_thread) {
            if(fuse_thread->joinable()) {
                fuse_thread->join();
                delete fuse_thread;
            }
        }
    }

    void MemFS::mount() { fuse_thread = new std::thread(fuse_loop, fuse); }

    void MemFS::unmount() {
        if(fused) {
            fused = false;
            if(fuse_thread) {
                std::ostringstream unmountCmd;
                unmountCmd << "fusermount -u \"" << mountpath << "\"";
                system(unmountCmd.str().c_str());
            }
            fuse_unmount(mountpath.c_str(), ch);
            fuse_destroy(fuse);
        }
    }


    int MemFS::getattr(const char *path, struct stat *buf) {
        memset(buf, 0, sizeof(struct stat));
        try {
            FSResource res = resolvePath(path);
            if(res.fstype == FSResourceType::DIR) {
                buf->st_mode = S_IFDIR | 0444;
                buf->st_nlink = 2+res.dir->getDirCount()+res.dir->getFileCount();
                buf->st_uid = getuid();
                buf->st_gid = getgid();
            }
            else {
                buf->st_mode = S_IFREG | 0444;
                buf->st_size = res.file->getSize();
                buf->st_nlink = 1;
                buf->st_uid = getuid();
                buf->st_gid = getgid();
            }
        }
        catch (bad_path) { return -ENOENT; }
        return 0;
    }

    int MemFS::readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t off, fuse_file_info *fi) {
        (void) off;
        (void) fi;
        try {
            FSResource res = resolvePath(path);
            if(res.fstype == FSResourceType::FILE) throw bad_path();
            filler(buf, ".", NULL, 0);
            filler(buf, "..", NULL, 0);
            auto fit = res.dir->getFileIterator();
            auto fitend = std::next(fit, res.dir->getFileCount());
            while(fit != fitend) {
                filler(buf, (*fit)->getName().c_str(), NULL, 0);
                fit++;
            }
            auto dit = res.dir->getDirIterator();
            auto ditend = std::next(dit, res.dir->getDirCount());
            while(dit != ditend) {
                filler(buf, (*dit)->getName().c_str(), NULL, 0);
                dit++;
            }
        }
        catch (bad_path) { return -ENOENT; }
        return 0;
    }

    int MemFS::read(const char *path, char *buf, size_t size, off_t off, fuse_file_info *fi) {
        (void) path;
        File * f = (File *) fi->fh;
        return f->pread(buf, size, off);
    }

    int MemFS::open(const char *path, fuse_file_info *fi) {
        if((fi->flags & 3) != O_RDONLY )
              return -EACCES;
        try {
            FSResource res = resolvePath(path);
            if(res.fstype == FSResourceType::FILE) {
                fi->fh = (u_int64_t) res.file;
            }
        }
        catch (bad_path) { return -ENOENT; }
        return 0;
    }

    FSResource MemFS::resolvePath(const std::string &path) {
        std::stringstream pathStream(path);
        char firstChar; pathStream.get(firstChar);
        if(firstChar != '/') throw bad_path(); // Absolute path without root? Nope!
        FSResource res;
        Directory *currentDir = rootDir.get();
        std::string currentPart;
        while(!pathStream.eof()) {
            std::getline(pathStream, currentPart, '/');
            if(!pathStream.eof()) {
                currentDir=currentDir->getDir(currentPart);
                if(!currentDir) {
                    throw bad_path();
                }
            }
            else {
                if(currentPart.length())  { // No / at the end -> file or directory
                    Directory *dirRes = currentDir->getDir(currentPart);
                    if(dirRes) {
                        res.fstype = FSResourceType::DIR;
                        res.dir = dirRes;
                        return res;
                    }
                    File *fileRes = currentDir->getFile(currentPart);
                    if(fileRes) {
                        res.fstype = FSResourceType::FILE;
                        res.file = fileRes;
                        return res;
                    }
                    throw bad_path();
                }
                else { // / at the end -> only directory
                    res.fstype = FSResourceType::DIR;
                    res.dir = currentDir;
                    return res;
                }
            }
            currentPart.clear();
        }
        return res; // Suppress warning about control reaching end without return
    }
}

#pragma once

#include "../src/StorageManagement.h"
#include "../src/Super.h"
#include "../src/Inode.h"
#include "../src/Utils.h"

class VSFS {
private:
    uint8_t* disk;
    uint32_t rootInode;
    size_t diskSize, inodeNum, chunkSize;
    StorageManagement* inodeData, * dataData;

    Inode CreateFolder(uint32_t type);
    bool LinkingFolder(Inode* inode, InodeTable table);
    bool LinkingFolder(Inode* inode, uint32_t inodeId, const std::string& name);
    void UpdateInodeData(Inode* inode, uint8_t* data, size_t size);

    void PrintInodeFromDisk(uint32_t inodeId) const;
public:
    VSFS(Super super);
    VSFS(uint8_t data[24]);

    void SetupParameter();

    StorageManagement* GetInodes() const;
    StorageManagement* GetDatas() const;

    size_t GetDiskSize() const;
    size_t GetInodeNumber() const;
    size_t GetChunkSize() const;

    void InitFileSystem();

    void mkdir(const std::string& path);
    void open(const std::string& path);

    void link(const std::string& _dst, const std::string& _src);
    void unlink(const std::string& _src);

    ~VSFS();
};


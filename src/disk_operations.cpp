/**
 * @file disk_operations.cpp
 * @author Soumodipta Bose (sbose2019@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2021-11-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "file_system.h"
SuperBlock *super_block_init()
{
    SuperBlock *super_block = new SuperBlock;
    super_block->super_blocks = ceil(((float)sizeof(SuperBlock)) / constants_disk_block_size);
    super_block->inodes = ceil(((float)sizeof(Inode) * constants_inode_limit) / constants_disk_block_size);
    super_block->data_blocks = constants_data_block_limit;
    super_block->inode_index = super_block->super_blocks;
    super_block->data_block_index = super_block->super_blocks + super_block->inodes;
    super_block->total_blocks = super_block->super_blocks + super_block->inodes + super_block->data_blocks;
    memset(super_block->inode_map, 0, sizeof(sizeof(super_block->inode_map)));
    memset(super_block->data_block_map, 0, sizeof(sizeof(super_block->data_block_map)));
    return super_block;
}
void Disk::persist_disk_meta_data()
{
    char buffer[sizeof(SuperBlock)];
    char inode_buffer[sizeof(inodes)];
    memset(buffer, 0, sizeof(SuperBlock));
    memcpy(buffer, &super_block, sizeof(SuperBlock));
    disk_stream.seekp(0, ios::beg);
    disk_stream.write(buffer, sizeof(SuperBlock));
    disk_stream.seekp(super_block->inode_index * constants_disk_block_size, ios::beg);
    memset(inode_buffer, 0, sizeof(inodes));
    memcpy(inode_buffer, &inodes, sizeof(inodes));
    disk_stream.write(inode_buffer, sizeof(inodes));
}
void Disk::load_disk_meta_data()
{
    char buffer[sizeof(SuperBlock)];
    memset(buffer, 0, sizeof(SuperBlock));
    char inode_buffer[sizeof(inodes)];
    memset(inode_buffer, 0, sizeof(inodes));
    memset(super_block, 0, sizeof(SuperBlock));
    disk_stream.seekg(0, ios::beg);
    disk_stream.read(buffer, sizeof(SuperBlock));
    memcpy(super_block, buffer, sizeof(SuperBlock));
    disk_stream.seekg(super_block->inode_index * constants_disk_block_size, ios::beg);
    disk_stream.read(inode_buffer, sizeof(inodes));
    memcpy(inodes, inode_buffer, sizeof(inodes));
}
FileMetaData *Disk::load_file_meta_data(Inode inode)
{
    FileMetaData *meta = new FileMetaData;
    int byte_position = (data_block_starting_index + inode.meta_data_block) * constants_disk_block_size;
    disk_stream.seekg(byte_position, ios::beg);
    char buffer[sizeof(FileMetaData)];
    memset(buffer, 0, sizeof(FileMetaData));
    disk_stream.read(buffer, sizeof(FileMetaData));
    memcpy(meta, buffer, sizeof(FileMetaData));
    return meta;
}
void Disk::save_file_meta_data(Inode inode, FileMetaData *meta)
{
    int byte_position = (data_block_starting_index + inode.meta_data_block) * constants_disk_block_size;
    char buffer[sizeof(FileMetaData)];
    memset(buffer, 0, sizeof(FileMetaData));
    memcpy(buffer, meta, sizeof(FileMetaData));
    disk_stream.seekp(byte_position, ios::beg);
    disk_stream.write(buffer, sizeof(SuperBlock));
}
void Disk::clear_file_meta_data(Inode inode)
{
}
int Disk::read_disk_block(int disk_block, char *buffer)
{
    int byte_position = (data_block_starting_index + disk_block) * constants_disk_block_size;
    disk_stream.seekg(byte_position, ios::beg);
    disk_stream.read(buffer, sizeof(constants_disk_block_size));
    return 0;
}
int Disk::write_disk_block(int disk_block, char *buffer, int bytes)
{
    int byte_position = (data_block_starting_index + disk_block) * constants_disk_block_size;
    disk_stream.seekp(byte_position, ios::beg);
    disk_stream.write(buffer, sizeof(bytes));
    return 0;
}
/**
 * @brief Populate fileinfo map  [filename]->inode
 * 
 */
void Disk::load_files_info()
{
    for (auto i : allocated_inodes)
    {
        FileMetaData *meta = load_file_meta_data(inodes[i]);
        string file_name(meta->file_name);
        file_info[file_name] = i;
    }
}
bool Disk::mount_disk()
{
    if (access(disk_name.c_str(), F_OK) == -1)
    {
        highlight_red("!! Disk does not exist !!\n");
        return false;
    }
    disk_stream.open(disk_name, ios::in | ios::out | ios::binary);
    super_block = super_block_init();
    load_disk_meta_data();
    for (int i = 0; i < constants_inode_limit; i++)
    {
        if (super_block->inode_map[i])
            allocated_inodes.push_back(i);
        else
            free_inodes.push_back(i);
    }
    for (int i = 0; i < constants_data_block_limit; i++)
    {
        if (super_block->data_block_map[i])
            allocated_data_blocks.push_back(i);
        else
            free_data_blocks.push_back(i);
    }
    inode_starting_index = super_block->inode_index;
    data_block_starting_index = super_block->data_block_index;
    load_files_info();
    return true;
}
int Disk::unmount_disk()
{
    persist_disk_meta_data();
    disk_stream.close();
}
void allocate_disk_space(string path, int size)
{
    fstream file;
    file.open(path, ios::out | ios::binary);
    file.seekp(size - 1, ios::end);
    file.put('\0');
    file.close();
}
/**
 * @brief Create a disk object
 * 
 * @param disk_name 
 */
void create_disk(string disk_name)
{
    if (access(disk_name.c_str(), F_OK) != -1)
    {
        highlight_red("A disk with this name already exists\n");
        return;
    }
    //First allocate 512MB and create an empty disk file
    allocate_disk_space(disk_name, constants_disk_size);
    Disk disk = Disk(disk_name);
    disk.disk_stream.open(disk_name, ios::out | ios::binary);
    disk.super_block = super_block_init();
    for (int i = 0; i < disk.super_block->inodes; i++)
    {
        disk.inodes[i].meta_data_block = -1;
        memset(disk.inodes[i].pointers, -1, sizeof(disk.inodes[i].pointers));
    }
    disk.persist_disk_meta_data();
    disk.disk_stream.close();
    highlight_blue("<New disk created>\n");
}

void Disk::create_file()
{
    string file_name;
    highlight_purple("Enter file name >>\n");
    cin >> file_name;
    if (file_name.size() > constants_file_name_size)
    {
        highlight_yellow("< File name crossed length limit >");
        return;
    }
    if (file_info.find(file_name) != file_info.end())
    {
        highlight_red("< File already exists in disk >\n");
        return;
    }
    if (free_inodes.empty())
    {
        highlight_red("< Maximum file limit reached >");
        return;
    }
    int new_inode = free_inodes.back();
    //set allocation bit to true
    super_block->inode_map[new_inode] = true;
    free_inodes.pop_back();
    if (free_inodes.empty())
    {
        highlight_red("< Maximum data block limit reached >");
        return;
    }
    int new_meta_data_block = free_data_blocks.back();
    //set allocation bit to true
    super_block->data_block_map[new_meta_data_block] = true;
    free_data_blocks.pop_back();
    inodes[new_inode].meta_data_block = new_meta_data_block;
    FileMetaData *new_metadata = new FileMetaData;
    memset(new_metadata->file_name, 0, sizeof(constants_file_name_size));
    memcpy(new_metadata->file_name, file_name.c_str(), sizeof(file_name));
    new_metadata->file_size = 0;
    save_file_meta_data(inodes[new_inode], new_metadata);
    file_info[file_name] = new_inode;
}
void Disk::delete_file()
{
}
void Disk::disk_operations()
{
    unmount_disk();
}
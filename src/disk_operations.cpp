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
void Disk::clear_inode(Inode *inode)
{
    inode->meta_data_block = -1;
    memset(inode->pointers, -1, sizeof(inode->pointers));
}
void Disk::persist_disk_meta_data()
{
    disk_stream.seekp(0, ios::beg);
    disk_stream.write((char *)super_block, sizeof(SuperBlock));
    disk_stream.seekp(super_block->inode_index * constants_disk_block_size, ios::beg);
    disk_stream.write((char *)inodes, sizeof(inodes));
}
void Disk::load_disk_meta_data()
{
    char inode_buffer[sizeof(inodes)];
    memset(super_block, 0, sizeof(SuperBlock));
    memset(inodes, 0, sizeof(inodes));
    disk_stream.seekg(0, ios::beg);
    disk_stream.read((char *)super_block, sizeof(SuperBlock));
    disk_stream.seekg(super_block->inode_index * constants_disk_block_size, ios::beg);
    disk_stream.read((char *)inodes, sizeof(inodes));
}
FileMetaData *Disk::load_file_meta_data(Inode inode)
{
    FileMetaData *meta = new FileMetaData;
    int byte_position = (data_block_starting_index + inode.meta_data_block) * constants_disk_block_size;
    disk_stream.seekg(byte_position, ios::beg);
    disk_stream.read((char *)meta, sizeof(FileMetaData));
    return meta;
}
void Disk::save_file_meta_data(Inode inode, FileMetaData *meta)
{
    int byte_position = (data_block_starting_index + inode.meta_data_block) * constants_disk_block_size;
    disk_stream.seekp(byte_position, ios::beg);
    disk_stream.write((char *)meta, sizeof(SuperBlock));
}
void Disk::clear_file_meta_data(Inode inode)
{
}
int Disk::read_disk_block(int disk_block, char *buffer)
{
    char disk_buffer[constants_disk_block_size];
    memset(disk_buffer, 0, constants_disk_block_size);
    int byte_position = (data_block_starting_index + disk_block) * constants_disk_block_size;
    disk_stream.seekg(byte_position, ios::beg);
    disk_stream.read(disk_buffer, constants_disk_block_size);
    memcpy(buffer, disk_buffer, constants_disk_block_size);
    return 0;
}
int Disk::write_disk_block(int disk_block, char *buffer)
{
    char disk_buffer[constants_disk_block_size];
    memset(disk_buffer, 0, constants_disk_block_size);
    memcpy(disk_buffer, buffer, constants_disk_block_size);
    int byte_position = (data_block_starting_index + disk_block) * constants_disk_block_size;
    disk_stream.seekp(byte_position, ios::beg);
    disk_stream.write(disk_buffer, constants_disk_block_size);
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
        highlight_red(">> Disk does not exist <<\n");
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
        file_descriptor_reserve.push_back(i);
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
    reverse(file_descriptor_reserve.begin(), file_descriptor_reserve.end());
    load_files_info();
    return true;
}
int Disk::unmount_disk()
{
    if (!open_files.empty())
    {
        highlight_red(">> Files are currently open, Close first <<");
        return 0;
    }
    persist_disk_meta_data();
    disk_stream.close();
    return 1;
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
        highlight_red(">> A disk with this name already exists <<\n");
        return;
    }
    //First allocate 512MB and create an empty disk file
    allocate_disk_space(disk_name, constants_disk_size);
    Disk disk = Disk(disk_name);
    disk.disk_stream.open(disk_name, ios::in | ios::out | ios::binary);
    disk.super_block = super_block_init();
    for (int i = 0; i < constants_inode_limit; i++)
    {
        disk.clear_inode(&disk.inodes[i]);
    }
    disk.persist_disk_meta_data();
    disk.disk_stream.seekp(constants_disk_size, ios::beg);
    disk.disk_stream.close();
    highlight_blue(">> New disk created <<\n");
}
void Disk::disk_operations()
{
    while (true)
    {
        line();
        cout << "\033[33m";
        cout << "1.\tCreate file" << endl;
        cout << "2.\tOpen file" << endl;
        cout << "3.\tRead file" << endl;
        cout << "4.\tWrite file" << endl;
        cout << "5.\tAppend file" << endl;
        cout << "6.\tClose file" << endl;
        cout << "7.\tDelete file" << endl;
        cout << "8.\tList of files" << endl;
        cout << "9.\tList of opened files" << endl;
        cout << "10.\tUnmount Disk" << endl;
        cout << "\033[0m";
        int choice;
        line();
        cin >> choice;
        switch (choice)
        {
        case 1:
            create_file();
            break;
        case 2:
            open_file();
            break;
        case 3:
            read_file();
            break;
        case 4:
            write_file();
            break;
        case 5:
            append_file();
            break;
        case 6:
            close_file();
            break;
        case 7:
            delete_file();
            break;
        case 8:
            list_files();
            break;
        case 9:
            list_open_files();
            break;
        case 10:
            unmount_disk();
            return;
            break;
        default:
            highlight_red(">> Wrong input <<\n");
        }
    }
}
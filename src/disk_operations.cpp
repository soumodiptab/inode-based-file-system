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
#include "terminal_utils.h"
SuperBlock *super_block_init()
{
    SuperBlock *super_block = new SuperBlock;

    return super_block;
}
int Disk::mount_disk()
{
}
int Disk::unmount_disk()
{
}
void allocate_disk_space(string path, int size)
{
    fstream file;
    file.open(path, ios::out | ios::binary);
    file.seekp(size - 1, ios::end);
    file.put('\0');
    file.close();
}
void create_disk(string disk_name)
{
    if (access(disk_name.c_str(), F_OK) != -1)
    {
        highlight_red("A disk with this name already exists\n");
        return;
    }
    //First allocate 512MB and create an empty file
    allocate_disk_space(disk_name, constants_disk_size);
    Disk disk = Disk(disk_name);
    disk.disk_stream.open(disk_name, ios::out | ios::binary);
    disk.super_block = super_block_init();
    disk.disk_stream.close();
    highlight_blue("New disk created\n");
}
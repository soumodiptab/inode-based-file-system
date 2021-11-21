/**
 * @file file_system.h
 * @author Soumodipta Bose (sbose2019@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2021-11-21
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <fstream>
#include <bits/stdc++.h>
/***************************************************************************************************************************/
/*                                                           Constants                                                     */
/***************************************************************************************************************************/
using namespace std;
typedef struct Inode Inode;
typedef struct SuperBlock SuperBlock;
typedef struct FileMetaData FileMetaData;
const int constants_file_name_size = 25;
/**
 * @brief For now going with simple direct pointers
 * 
 */
const int constants_inode_pointers = 15;
/**
 * @brief default: 4KB disk allocation unit
 * 
 */
const int constants_disk_block_size = 4096;
/**
 * @brief default: 512MB disk size
 * 
 */
const int constants_disk_size = 536870912;

/***************************************************************************************************************************/
/*                                                           Entities                                                      */
/***************************************************************************************************************************/

struct Inode
{
    int pointers[constants_inode_pointers];
};
struct FileMetaData
{
    char file_name[constants_file_name_size];
    unsigned int file_size;
};
struct SuperBlock
{
};
/**
 * @brief Initialize the superblock parameters
 * 
 */
SuperBlock *super_block_init();
class File
{
public:
    int inode;
    int file_descriptor;
    int meta_data_block;
    string name;
    // 0 - read 1 - write 2 - append
    int mode;
    int last_byte;
    vector<pair<int, char *>> disk_blocks;
    int load(int inode);
    int read();
};
class Disk
{
public:
    fstream disk_stream;
    string disk_name;
    SuperBlock *super_block;
    vector<int> allocated_inodes;
    vector<int> free_inodes;
    vector<int> free_data_blocks;
    vector<int> file_descriptor;
    // <filename| inode>
    unordered_map<string, int> file_info;
    // <filedescriptor| File>
    unordered_map<int, File> open_files;
    Disk()
    {
    }
    Disk(string name)
    {
        this->disk_name = name;
    }
    /**
     * @brief Will fetch the first block of each file and map them
     * 
     */
    void load_files_info();
    int read_disk_block(int disk_block, char *buffer);
    int write_disk_block(int disk_block, char *buffer, int bytes);
    int mount_disk();
    int create_file(string file_name);
    int open_file();
    int read_file();
    int write_file();
    int delete_file();
    int append_file();
    int close_file();
    // persist all metadata
    int unmount_disk();
    void disk_operations();
};
/***************************************************************************************************************************/
/*                                                     API Declarations                                                    */
/***************************************************************************************************************************/
void allocate_disk_space(string path, int size);
void create_disk(string);
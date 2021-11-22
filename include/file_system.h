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
const int constants_inode_limit = 100;
const int constants_data_block_limit = 1500;
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
    int meta_data_block;
    int pointers[constants_inode_pointers];
};
struct FileMetaData
{
    char file_name[constants_file_name_size];
    unsigned int file_size;
};
struct SuperBlock
{
    int super_blocks;
    int inodes;
    int inode_index;
    int data_blocks;
    int data_block_index;
    int total_blocks;
    bool inode_map[constants_inode_limit];
    bool data_block_map[constants_data_block_limit];
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
    string file_name;
    int file_descriptor;
    int meta_data_block;
    FileMetaData *metadata;
    // 1 - read 2 - write 3 - append
    int mode;
    int last_byte;
    vector<pair<int, char *>> disk_blocks;
};
class Disk
{
public:
    fstream disk_stream;
    string disk_name;
    SuperBlock *super_block;
    int inode_starting_index;
    int data_block_starting_index;
    Inode inodes[constants_inode_limit];
    vector<int> allocated_inodes;
    vector<int> free_inodes;
    vector<int> allocated_data_blocks;
    vector<int> free_data_blocks;
    vector<int> file_descriptor_reserve;
    // <filename| inode>
    unordered_map<string, int> file_info;
    // filename | filedescriptor
    unordered_map<string, int> open_file_name_map;
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
    void persist_disk_meta_data();
    void load_disk_meta_data();
    void load_files_info();
    void clear_inode(Inode *inode);
    FileMetaData *load_file_meta_data(Inode inode);
    void save_file_meta_data(Inode inode, FileMetaData *filemeta);
    void clear_file_meta_data(Inode inode);
    int read_disk_block(int disk_block, char *buffer);
    int write_disk_block(int disk_block, char *buffer, int bytes);
    bool mount_disk();
    void create_file();
    void open_file();
    void read_file();
    void write_file();
    void delete_file();
    void append_file();
    void close_file();
    void list_files();
    void list_open_files();

    // persist all metadata
    int unmount_disk();
    void disk_operations();
};
/***************************************************************************************************************************/
/*                                                     API Declarations                                                    */
/***************************************************************************************************************************/
void allocate_disk_space(string path, int size);
void create_disk(string);
void highlight_red(string message);
void highlight_green(string message);
void highlight_blue(string message);
void highlight_cyan(string message);
void highlight_yellow(string message);
void highlight_purple(string message);
void line();
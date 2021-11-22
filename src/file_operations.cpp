#include "file_system.h"
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
    string file_name;
    highlight_purple("Enter file name >>\n");
    cin >> file_name;
    if (file_info.find(file_name) == file_info.end())
    {
        highlight_red("File does not exist\n");
        return;
    }
    if (open_file_name_map.find(file_name) != open_file_name_map.end())
    {
        highlight_red("File is open , close first\n");
        return;
    }
    int target_inode = file_info[file_name];
    allocated_inodes.erase(find(allocated_inodes.begin(), allocated_inodes.end(), target_inode));
    free_inodes.push_back(target_inode);
    clear_inode(&inodes[target_inode]);
    super_block->inode_map[target_inode] = false;
    file_info.erase(file_name);
}
void Disk::open_file()
{
    string file_name;
    highlight_purple("Enter file name >>\n");
    cin >> file_name;
    if (file_info.find(file_name) == file_info.end())
    {
        highlight_red("File does not exist\n");
        return;
    }
    if (open_file_name_map.find(file_name) != open_file_name_map.end())
    {
        highlight_red("File is already open\n");
        return;
    }
    int mode;
    highlight_purple("Enter mode:\n1.Read\n2.Write\n3.Append\n");
    cin >> mode;
    if (mode != 1 || mode != 2 || mode != 3)
    {
        highlight_red("Incorrect mode selected\n");
        return;
    }
    int new_file_descriptor = file_descriptor_reserve.back();
    int target_inode = file_info[file_name];
    open_file_name_map[file_name] = new_file_descriptor;
    file_descriptor_reserve.pop_back();
    File new_file = File();
    open_files[new_file_descriptor] = new_file;
    new_file.metadata = load_file_meta_data(inodes[target_inode]);
    new_file.inode = target_inode;
    new_file.file_name = string(new_file.metadata->file_name);
    new_file.last_byte = new_file.metadata->file_size % constants_disk_block_size;
    new_file.file_descriptor = new_file_descriptor;
    new_file.mode = mode;
    for (int i = 0; i < constants_inode_pointers; i++)
    {
        if (inodes[target_inode].pointers[i] == -1)
        {
            break;
        }
        int disk_block = inodes[target_inode].pointers[i];
        char *buffer = new char[constants_disk_block_size];
        read_disk_block(disk_block, buffer);
        new_file.disk_blocks.push_back(make_pair(disk_block, buffer));
    }
}
void Disk::close_file()
{
    int file_descriptor;
    highlight_purple("Enter file name >>\n");
    cin >> file_descriptor;
    if (open_files.find(file_descriptor) == open_files.end())
    {
        highlight_red("File is not open\n");
        return;
    }
    file_descriptor_reserve.push_back(file_descriptor);
    File &file = open_files[file_descriptor];
    int target_inode = file.inode;
    // if mode =1 dont do any persisting
    if (open_files[file_descriptor].mode > 1)
    {
        save_file_meta_data(inodes[target_inode], file.metadata);
        memset(inodes[target_inode].pointers, -1, sizeof(inodes[target_inode].pointers));
        for (int i = 0; i < file.disk_blocks.size(); i++)
        {
            int disk_block = file.disk_blocks[i].first;
            char *buffer = file.disk_blocks[i].second;
            inodes[target_inode].pointers[i] = disk_block;
            write_disk_block(disk_block, buffer, constants_disk_block_size);
        }
    }
    open_file_name_map.erase(file.file_name);
    open_files.erase(file_descriptor);
}
void Disk::read_file()
{
     int file_descriptor;
    highlight_purple("Enter file descriptor >>\n");
    cin >> file_descriptor;
    if (open_files.find(file_descriptor) == open_files.end())
    {
        highlight_red("File is not open\n");
        return;
    }
    if (open_files[file_descriptor].mode != 1)
    {
        highlight_red("File is not open in read mode\n");
        return;
    }
    
}
void Disk::write_file()
{
    int file_descriptor;
    highlight_purple("Enter file descriptor >>\n");
    cin >> file_descriptor;
    if (open_files.find(file_descriptor) == open_files.end())
    {
        highlight_red("File is not open\n");
        return;
    }
    if (open_files[file_descriptor].mode != 2)
    {
        highlight_red("File is not open in write mode\n");
        return;
    }
    highlight_purple("Enter text for file >>\n");
    string text;
    getline(cin, text, static_cast<char>(EOF));
    File &file = open_files[file_descriptor];
    int write_blocks = ceil(((float)text.size()) / constants_disk_block_size);
    int last_byte = text.size() % constants_disk_block_size;
    file.last_byte = last_byte;
    char buffer[write_blocks * constants_disk_block_size];
    memset(buffer, 0, write_blocks * constants_disk_block_size);
    memcpy(buffer, text.c_str(), text.size());
    if (write_blocks > constants_inode_pointers)
    {
        highlight_red("Byte limit reached\n");
        return;
    }

    int offset = 0;
    for (int i = 0; i < write_blocks; i++)
    {
        memcpy(file.disk_blocks[i].second, buffer + offset, constants_disk_block_size);
        offset += constants_disk_block_size;
    }
    //free the datablocks
    if (write_blocks < file.disk_blocks.size())
    {
        int free = file.disk_blocks.size() - write_blocks;
        while (free--)
        {
            pair<int, char *> data_info = file.disk_blocks.back();
            file.disk_blocks.pop_back();
            delete[] data_info.second;
            free_data_blocks.push_back(data_info.first);
            super_block->data_block_map[data_info.first] = false;
        }
    }
    highlight_green("Bytes have been written to the file\n");
}
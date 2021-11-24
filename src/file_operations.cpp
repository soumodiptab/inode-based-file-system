#include "file_system.h"
string multiline_input()
{
    string input;
    string line;
    cin.ignore(1, '\n');
    while (getline(cin, line))
    {
        if (line == "$")
            break;

        input += line + "\n";
    }
    return input;
}
void Disk::create_file()
{
    string file_name;
    highlight_purple("Enter file name >>\n");
    cin >> file_name;
    if (file_name.size() > constants_file_name_size)
    {
        highlight_yellow(">> File name crossed length limit <<");
        return;
    }
    if (file_info.find(file_name) != file_info.end())
    {
        highlight_red(">> File with name already exists on disk <<\n");
        return;
    }
    if (free_inodes.empty())
    {
        highlight_red(">> Maximum file limit reached <<");
        return;
    }
    int new_inode = free_inodes.back();
    //set allocation bit to true
    super_block->inode_map[new_inode] = true;
    free_inodes.pop_back();
    if (free_data_blocks.empty())
    {
        highlight_red(">> Maximum data block limit reached <<");
        return;
    }
    int new_meta_data_block = free_data_blocks.back();
    //set allocation bit to true
    super_block->data_block_map[new_meta_data_block] = true;
    free_data_blocks.pop_back();
    inodes[new_inode].meta_data_block = new_meta_data_block;
    FileMetaData *new_metadata = new FileMetaData;
    memset(new_metadata, 0, sizeof(FileMetaData));
    memcpy(new_metadata->file_name, file_name.c_str(), sizeof(file_name));
    new_metadata->file_size = 0;
    save_file_meta_data(inodes[new_inode], new_metadata);
    file_info[file_name] = new_inode;
    highlight_green(">> A new file [ " + file_name + " ] has been created on disk <<\n");
}
/**
 * @brief De-allocate inode information from superblock and inode array
 * 
 */
void Disk::delete_file()
{
    string file_name;
    highlight_purple("Enter file name >>\n");
    cin >> file_name;
    if (file_info.find(file_name) == file_info.end())
    {
        highlight_red(">> File does not exist <<\n");
        return;
    }
    if (open_file_name_map.find(file_name) != open_file_name_map.end())
    {
        highlight_red(">> File is currently open , close first <<\n");
        return;
    }
    int target_inode = file_info[file_name];
    allocated_inodes.erase(find(allocated_inodes.begin(), allocated_inodes.end(), target_inode));
    free_inodes.push_back(target_inode);
    if (inodes[target_inode].meta_data_block != -1)
    {
        allocated_data_blocks.erase(find(allocated_data_blocks.begin(), allocated_data_blocks.end(), inodes[target_inode].meta_data_block));
        free_data_blocks.push_back(inodes[target_inode].meta_data_block);
        super_block->data_block_map[inodes[target_inode].meta_data_block] = false;
    }
    for (int i = 0; i < constants_inode_pointers; i++)
    {
        if (inodes[target_inode].pointers[i] == -1)
            break;
        allocated_data_blocks.erase(find(allocated_data_blocks.begin(), allocated_data_blocks.end(), inodes[target_inode].pointers[i]));
        free_data_blocks.push_back(inodes[target_inode].pointers[i]);
        super_block->data_block_map[inodes[target_inode].pointers[i]] = false;
    }
    clear_inode(&inodes[target_inode]);
    super_block->inode_map[target_inode] = false;
    file_info.erase(file_name);
    highlight_green(">> File has been deleted <<\n");
}
/**
 * @brief Open file with available file-descriptors
 * 
 */
void Disk::open_file()
{
    string file_name;
    highlight_purple("Enter file name >>\n");
    cin >> file_name;
    if (file_info.find(file_name) == file_info.end())
    {
        highlight_red(">> File does not exist <<\n");
        return;
    }
    if (open_file_name_map.find(file_name) != open_file_name_map.end())
    {
        highlight_red(">> File is already open <<\n");
        return;
    }
    int mode;
    highlight_purple("Enter mode:\n1.Read\n2.Write\n3.Append\n");
    cin >> mode;
    if (mode != 1 && mode != 2 && mode != 3)
    {
        highlight_red(">> Incorrect mode selected <<\n");
        return;
    }
    int new_file_descriptor = file_descriptor_reserve.back();
    int target_inode = file_info[file_name];
    open_file_name_map[file_name] = new_file_descriptor;
    file_descriptor_reserve.pop_back();
    File new_file = File();
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
    open_files[new_file_descriptor] = new_file;
    string modec = (mode == 1) ? "Read" : ((mode == 2) ? "Write" : "Append");
    highlight_green(">> " + new_file.file_name + " has been opened  in " + modec + " mode with file-descriptor: [" + to_string(new_file_descriptor) + "] <<\n");
}
/**
 * @brief Persists file data to disk, Exiting without closing file will cause file to lose session changes
 */
void Disk::close_file()
{
    int file_descriptor;
    highlight_purple("Enter file descriptor >>\n");
    cin >> file_descriptor;
    if (open_files.find(file_descriptor) == open_files.end())
    {
        highlight_red(">> No files open with given file-descriptor <<\n");
        return;
    }
    file_descriptor_reserve.push_back(file_descriptor);
    File &file = open_files[file_descriptor];
    int target_inode = file.inode;
    // if mode = 1(read) dont do any persisting
    if (open_files[file_descriptor].mode > 1)
    {
        save_file_meta_data(inodes[target_inode], file.metadata);
        memset(inodes[target_inode].pointers, -1, sizeof(inodes[target_inode].pointers));
        for (int i = 0; i < file.disk_blocks.size(); i++)
        {
            int disk_block = file.disk_blocks[i].first;
            char *buffer = file.disk_blocks[i].second;
            inodes[target_inode].pointers[i] = disk_block;
            write_disk_block(disk_block, buffer);
        }
    }
    highlight_green(">> " + file.file_name + " has been closed <<\n");
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
        highlight_red(">> No files open with given file-descriptor <<\n");
        return;
    }
    if (open_files[file_descriptor].mode != 1)
    {
        highlight_red(">> File is not open in read mode <<\n");
        return;
    }
    File &file = open_files[file_descriptor];
    highlight_blue("Contents of the file :\n");
    line();
    cout << "\033[35m";
    for (int i = 0; i < file.disk_blocks.size(); i++)
    {
        printf("%s", file.disk_blocks[i].second);
    }
    cout << "\033[0m";
}
void Disk::write_file()
{
    int file_descriptor;
    highlight_purple("Enter file descriptor >>\n");
    cin >> file_descriptor;
    if (open_files.find(file_descriptor) == open_files.end())
    {
        highlight_red(">> No files open with given file-descriptor <<\n");
        return;
    }
    if (open_files[file_descriptor].mode != 2)
    {
        highlight_red(">> File is not open in write mode <<\n");
        return;
    }
    highlight_purple("Enter text for file >>\n");
    string text = multiline_input();
    File &file = open_files[file_descriptor];
    int write_blocks = ceil(((float)text.size()) / constants_disk_block_size);
    file.metadata->file_size = text.size();
    int last_byte = text.size() % constants_disk_block_size;
    file.last_byte = last_byte;
    char buffer[write_blocks * constants_disk_block_size];
    memset(buffer, 0, write_blocks * constants_disk_block_size);
    memcpy(buffer, text.c_str(), text.size());
    if (write_blocks > constants_inode_pointers)
    {
        highlight_red(">> Byte limit reached <<\n");
        return;
    }
    //free the datablocks
    if (write_blocks <= file.disk_blocks.size())
    {
        int offset = 0;
        for (int i = 0; i < write_blocks; i++)
        {
            memcpy(file.disk_blocks[i].second, buffer + offset, constants_disk_block_size);
            offset += constants_disk_block_size;
        }
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
    else
    {
        int offset = 0;
        int i = 0;
        for (; i < file.disk_blocks.size(); i++)
        {
            memcpy(file.disk_blocks[i].second, buffer + offset, constants_disk_block_size);
            offset += constants_disk_block_size;
        }
        for (; i < write_blocks; i++)
        {
            int new_disk_block = free_data_blocks.back();
            free_data_blocks.pop_back();
            super_block->data_block_map[new_disk_block] = true;
            char *new_buffer = new char[constants_disk_block_size];
            memcpy(new_buffer, buffer + offset, constants_disk_block_size);
            offset += constants_disk_block_size;
            file.disk_blocks.push_back(make_pair(new_disk_block, new_buffer));
        }
    }
    highlight_green(">> " + to_string(text.size()) + "B have been written to " + file.file_name + " <<\n");
}
void Disk::append_file()
{
    int file_descriptor;
    highlight_purple("Enter file descriptor >>\n");
    cin >> file_descriptor;
    if (open_files.find(file_descriptor) == open_files.end())
    {
        highlight_red(">> File is not open <<\n");
        return;
    }
    if (open_files[file_descriptor].mode != 3)
    {
        highlight_red(">> File is not open in append mode <<\n");
        return;
    }
    highlight_purple("Enter text for file >>\n");
    string text = multiline_input();
    File &file = open_files[file_descriptor];
    int write_blocks = ceil(((float)text.size()) / constants_disk_block_size);
    char buffer[write_blocks * constants_disk_block_size];
    memset(buffer, 0, write_blocks * constants_disk_block_size);
    memcpy(buffer, text.c_str(), text.size());
    int remaining_bytes = constants_disk_block_size - file.last_byte;
    int offset = file.last_byte;
    if (file.last_byte != 0)
    {
        memcpy(file.disk_blocks[file.disk_blocks.size() - 1].second + offset, buffer, remaining_bytes);
        offset += remaining_bytes;
    }
    if (text.size() > remaining_bytes)
    {
        while (offset < text.size())
        {
            int new_data_block = free_data_blocks.back();
            free_data_blocks.pop_back();
            super_block->data_block_map[new_data_block] = true;
            char *data_block_buffer = new char[constants_disk_block_size];
            memset(data_block_buffer, 0, constants_disk_block_size);
            memcpy(data_block_buffer, buffer + offset, constants_disk_block_size);
            offset += constants_disk_block_size;
            file.disk_blocks.push_back(make_pair(new_data_block, data_block_buffer));
        }
    }
    file.metadata->file_size += text.size();
    highlight_green(">> " + to_string(text.size()) + "B have been appended to " + file.file_name + " <<\n");
    highlight_green(">> Total file size: " + to_string(file.metadata->file_size) + "B <<\n");
}
void Disk::list_files()
{
    if (file_info.empty())
    {
        highlight_red(">> No files present in Disk <<\n");
        return;
    }
    highlight_blue("Files on Disk:\n");
    for (auto file : file_info)
    {
        FileMetaData *meta = load_file_meta_data(inodes[file.second]);
        highlight_cyan(file.first + "\t" + to_string(meta->file_size) + "B\n");
    }
}
void Disk::list_open_files()
{
    if (open_files.empty())
    {
        highlight_red(">> No files currently open <<\n");
        return;
    }
    highlight_blue("Open files:\n");
    for (auto file : open_files)
    {
        int mode = file.second.mode;
        char modec = (mode == 1) ? 'R' : ((mode == 2) ? 'W' : 'A');
        highlight_cyan(to_string(file.first) + "\t->\t" + file.second.file_name + "\t" + modec + "\n");
    }
}
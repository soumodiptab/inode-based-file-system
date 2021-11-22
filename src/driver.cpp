/**
 * @file driver.cpp
 * @author Soumodipta Bose (sbose2019@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2021-11-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "file_system.h"
void highlight_red(string message)
{
    cout << "\033[31m";
    cout << message;
    cout << "\033[0m";
}
void highlight_green(string message)
{
    cout << "\033[32m";
    cout << message;
    cout << "\033[0m";
}
void highlight_blue(string message)
{
    cout << "\033[34m";
    cout << message;
    cout << "\033[0m";
}
void highlight_cyan(string message)
{
    cout << "\033[36m";
    cout << message;
    cout << "\033[0m";
}
void highlight_yellow(string message)
{
    cout << "\033[33m";
    cout << message;
    cout << "\033[0m";
}
void highlight_purple(string message)
{
    cout << "\033[35m";
    cout << message;
    cout << "\033[0m";
}
void line()
{
    cout << "=================================================================================" << endl;
}
void main_menu()
{
    int choice;
    string disk_name;
    while (true)
    {

        cout << "Simple File System" << endl;
        line();
        highlight_yellow("1. Create Disk\n");
        highlight_yellow("2. Mount Disk\n");
        highlight_yellow("3. Exit\n");
        line();
        cin >> choice;
        if (choice == 1)
        {
            cout << "Enter disk name" << endl;
            cin >> disk_name;
            create_disk(disk_name);
        }
        else if (choice == 2)
        {
            cout << "Enter disk name" << endl;
            cin >> disk_name;
            Disk disk = Disk(disk_name);
            if (disk.mount_disk())
                disk.disk_operations();
        }
        else if (choice == 3)
            exit(0);
        else
            highlight_red("!! Wrong Choice !!\n");
    }
}
int main()
{
    main_menu();
    return 0;
}
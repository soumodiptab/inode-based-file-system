#include <bits/stdc++.h>
using namespace std;
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
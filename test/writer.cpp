#include <bits/stdc++.h>
using namespace std;
int main()
{
    string s;
    int a;
    getline(cin, s, static_cast<char>(EOF)); // EOF stands for Ctrl-D
    if (cin.fail())
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin >> a;
    cout << a << endl;
    cout << s << endl;
    return 0;
}
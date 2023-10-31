
#include <iostream>

namespace pigeon
{
    __declspec(dllimport) void myExportedFunction();
}
int main()
{
    pigeon::myExportedFunction();
}
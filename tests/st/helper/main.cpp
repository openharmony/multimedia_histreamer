#include <iostream>
#include "crashdump/common/crash_handler_register.h"

int RunTests(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    std::cout << "Running RunTests() from " << __FILE__ << std::endl;
    crash_handler_register(std::wstring(L"\\dumper.exe"));
    return RunTests(argc, argv);
}

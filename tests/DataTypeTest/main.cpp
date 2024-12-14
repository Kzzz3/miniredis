#include <cassert>

#include "hashcmd_test.hpp"
#include "listcmd_test.hpp"
#include "setcmd_test.hpp"
#include "stringcmd_test.hpp"
#include "zsetcmd_test.hpp"

int main()
{
    try
    {
        // Test each data type separately
        int num = 100000;
        TestStringCommands(num);
        TestHashCommands(num);
        TestListCommands(num);
        TestSetCommands(num);
        TestZSetCommands(num);

        cout << "All tests completed" << endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
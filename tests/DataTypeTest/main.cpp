#include <cassert>

#include "hashcmd_test.hpp"
#include "listcmd_test.hpp"
#include "randomcmd_test.hpp"
#include "setcmd_test.hpp"
#include "stringcmd_test.hpp"
#include "zsetcmd_test.hpp"

int main()
{
    try
    {
        // Test each data type separately
        TestStringCommands();
        TestHashCommands();
        TestListCommands();
        TestSetCommands();
        TestZSetCommands();

        // Multi-threaded random test
        TestMultiThreadRandomCommands();

        cout << "All tests completed" << endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
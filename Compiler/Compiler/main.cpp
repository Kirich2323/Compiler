#include <iostream>

#include "Scanner.h"
#include "Tests.h"
#include "error.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc == 3) {
        if (!strcmp(argv[2], "-l")) {
            Scanner scanner(argv[1]);
            cout << scanner.getTokensString();
        }
    }
    else if (argc == 2) {
        if (!strcmp(argv[1], "-t")) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }
    else {
        cout << BadArgumentNumber().what();
    }
    return 0;
}
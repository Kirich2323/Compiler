#include <iostream>

#include "Scanner.h"
#include "Tests.h"
#include "error.h"
#include "Parser.h"

using namespace std;

int main(int argc, char *argv[]) {
    try {
        if (argc == 3) {
            if (!strcmp(argv[2], "-l")) {
                Scanner scanner(argv[1]);
                cout << scanner.getTokensString();
            }
            else if (!strcmp(argv[2], "-p")) {
                Parser parser(argv[1]);
                //parser.setSymbolCheck(false);
                //cout << parser.getNodeTreeStr() << endl;
                //cout << parser.getDeclStr() << endl;
                cout << parser.getStmtStr() << endl;
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
    }
    catch (BaseException e) {
        cout << e.what() << endl;
    }
    return 0;
}
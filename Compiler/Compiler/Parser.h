#pragma once

#include <vector>
#include <set>
#include "Scanner.h"
#include "SynNode.h"

enum class Priority {
    Lowest = 0,
    Third = 1,
    Second = 2,
    Highest = 3
};

class Parser {
public:
    Parser(const char*);
    std::string getNodeTreeStr();
    std::vector<PNode> parseCommaSeparated();
private:
    PNode parseExpr(int);
    PNode parseFactor();
    PNode parseIdentifier(bool = true);
    bool checkPriority(int, TokenType);
    std::vector<std::set<TokenType>> _priorityTable;
    Scanner _scanner;
};
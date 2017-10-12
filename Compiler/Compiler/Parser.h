#pragma once

#include <vector>
#include <set>
#include <fstream>
#include "Scanner.h"
#include "SynNode.h"
#include "Symbol.h"
#include "TypeChecker.h"
#include "Const.h"

enum class Priority {
    Lowest = 0,
    Third = 1,
    Second = 2,
    Highest = 3
};

class Parser {
public:
    Parser(const char*, bool isSymbolCheck = true);
    std::string getNodeTreeStr();
    std::string getDeclStr();
    std::string getProgStr();
    std::string getStmtStr();
    std::string getAsmStr();
    std::vector<PNode> parseCommaSeparated();
    void setSymbolCheck(bool isCheck);
private:

    typedef Const(*computeUnOp)(Const);
    typedef Const(*computeBinOp)(Const, Const);

    PNode parseExpr(int);
    PNode parseFactor();
    PNode parseIdentifier(bool = true);

    PNode parseStatement();
    PNode parseCompoundStatement(std::string name);   

    PNode parseWhileStatement();
    PNode parseIfStatement();
    PNode parseForStatement();
    PNode parseRepeatStatement();
    PNode parseIdentifierStatement();
    PNode parseWrite();
    PNode parseWriteln();
    PNode parseBreak();
    PNode parseContinue();
    PNode parse();
    std::vector<PNode> getArgsArray(TokenType terminatingType);

    SymbolPtr parseRecord();
    SymbolPtr parseArray();
    SymbolPtr parseSubrange();
    SymbolPtr parsePointer();
    SymbolPtr parseConst(std::string identifier = "");    

    void parseDeclaration(int depth, bool isGlobal);
    void parseVarDeclaration(bool isGlobal);
    void parseConstDeclaration();
    void parseTypeDeclaration();
    void parseFuncDeclaration(int depth);
    void parseProcDeclaration(int depth);
    void generateProc(SymbolPtr symbol, int depth);
    void parseStatementSequence(BlockNode* block);
    SymbolPtr parseType();
    SymTablePtr parseParams(SymbolPtr proc);
    SymTablePtr parseProcBody(SymbolPtr proc, int depth);

    std::vector<TokenPtr> parseIdentifiers();

    double getFrac(double value);
    void expectType(SymbolType type, PNode expr, TokenPtr tok);
    Const ComputeConstantExpression(PNode node);

    TokenPtr getToken();
    TokenPtr getNextToken();
    bool checkPriority(int, TokenType);
    bool checkSymbolType(SymbolPtr symbol, SymbolType expectedType, TokenPtr token);

    std::string _progName;
    std::vector<std::set<TokenType>> _priorityTable;
    std::map<SymbolPtr, PNode> _procedureBodies;
    std::map<TokenType, computeUnOp> _computableUnOps;
    std::map<TokenType, computeBinOp> _computableBinOps;
    Scanner _scanner;
    SymTableStackPtr _symTables;
    bool _isSymbolCheck;
    TypeChecker _typeChecker;
    AsmCode _code;
    PNode _root;
};
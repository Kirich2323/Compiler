#pragma once

#include <vector>
#include <set>
//#include <map>
#include "Scanner.h"
#include "SynNode.h"
#include "Symbol.h"
#include "TypeChecker.h"

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
    std::vector<PNode> parseCommaSeparated();
    void setSymbolCheck(bool isCheck);
private:

    typedef Const(Parser::*computeUnOp)(Const);
    typedef Const(Parser::*computeBinOp)(Const, Const);

    PNode parseExpr(int);
    PNode parseFactor();
    PNode parseIdentifier(bool = true);

    PNode parseStatement();
    PNode parseCompoundStatement(std::string name);
    void parseStatementSequence(BlockNode* block);

    PNode parseWhileStatement();
    PNode parseIfStatement();
    PNode parseForStatement();
    PNode parseJumpStatement();
    PNode parseRepeatStatement();
    PNode parseCall(PNode expr);
    PNode parseIdentifierStatement();
    std::vector<PNode> getArgsArray(TokenType terminatingType);

    SymbolPtr parseRecord();
    SymbolPtr parseArray();
    SymbolPtr parseSubrange();
    SymbolPtr parsePointer();
    SymbolPtr parseConst(std::string identifier = "");

    PNode parse();

    void parseDeclaration(int depth, bool isGlobal);
    void parseVarDeclaration(bool isGlobal);
    void parseConstDeclaration();
    void parseTypeDeclaration();
    void parseFuncDeclaration(int depth);
    void parseProcDeclaration(int depth);
    SymbolPtr parseType();
    SymTablePtr parseParams(SymbolPtr proc);
    SymTablePtr parseProcBody(SymbolPtr proc, int depth);

    std::vector<TokenPtr> parseIdentifiers();

    double getFrac(double value);
    void expectType(SymbolType type, PNode expr, TokenPtr tok);
    //double ComputeUnaryOrd(double arg);
    Const ComputeUnaryAddition(Const arg);
    Const ComputeUnarySubtraction(Const arg);
    Const ComputeUnaryNeagtion(Const arg);
    Const ComputeBinaryMultiplication(Const left, Const right);
    Const ComputeBinaryDivision(Const left, Const);
    Const ComputeBinaryDiv(Const left, Const right);
    Const ComputeBinaryMod(Const left, Const right);
    Const ComputeBinaryAnd(Const left, Const right);
    Const ComputeBinaryShl(Const left, Const right);
    Const ComputeBinaryShr(Const left, Const right);
    Const ComputeBinaryAddition(Const left, Const right);
    Const ComputeBinarySubtraction(Const left, Const right);
    Const ComputeBinaryOr(Const left, Const right);
    Const ComputeBinaryXor(Const left, Const right);
    Const ComputeBinaryEq(Const left, Const right);
    Const ComputeBinaryNe(Const left, Const right);
    Const ComputeBinaryLt(Const left, Const right);
    Const ComputeBinaryGt(Const left, Const right);
    Const ComputeBinaryLe(Const left, Const right);
    Const ComputeBinaryGe(Const left, Const right);
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
};
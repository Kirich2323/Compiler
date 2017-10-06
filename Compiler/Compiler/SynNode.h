#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Token.h"
#include "Symbol.h"


enum class BinOpType {
    Add,
    Sub,
    Mult,
    Div
};

enum class SynNodeType {
    Node,
    Number,
    IntegerNumber,
    RealNumber,
    Identifier,
    BinaryOp,
    UnaryOp,
    //AssignOp,
    RecordAcces,
    Call,
    ArrayIndex,
    Block,
    IfStmt,
    WhileStmt,
    ForStmt,
    RepeatStmt,
    Empty
};

class SynNode {
public:
    SynNode(SynNodeType type);
    virtual std::string toString(std::string, bool last) = 0;
    SynNodeType getNodeType();
    virtual SymbolType getType();
    bool operator == (SynNodeType type);
    bool operator != (SynNodeType type);
protected:
    SynNodeType _type;
    std::string makeIndent(std::string, bool);
    void updateIndentAndStr(std::string&, std::string&, bool);
};
typedef std::shared_ptr<SynNode> PNode;

class OpNode : public SynNode {
public:
    OpNode(TokenPtr tok, SynNodeType type);
    TokenType getOpType();
protected:
    TokenType _op;
    std::string _sign;
};

class UnaryNode : public OpNode {
public:
    UnaryNode(TokenPtr, PNode);
    std::string toString(std::string, bool);
    PNode getArg();
    SymbolType getType() override;
private:
    PNode _arg;
};
//typedef std::shared_ptr<UnaryNode> UnaryNodePtr; //todo try to delete

class BinOpNode : public OpNode {
public:
    BinOpNode(TokenPtr, const PNode&, const PNode&);
    std::string toString(std::string, bool last);
    PNode getLeft();
    PNode getRight();
    SymbolType getType() override;
private:
    PNode _left, _right;
};
typedef std::shared_ptr<BinOpNode> BinOpNodePtr;

class IntConstNode : public SynNode {
public:
    IntConstNode(int);
    std::string toString(std::string, bool);
    SymbolType getType() override;
    int getValue();
private:
    int _value;
};

class RealConstNode : public SynNode {
public:
    RealConstNode(double);
    std::string toString(std::string, bool);
    SymbolType getType() override;
    double getValue();
private:
    double _value;
};

class IdentifierNode : public SynNode {
public:
    IdentifierNode(std::string name, SymbolPtr symbol);
    std::string getName();
    std::string toString(std::string, bool);
    SymbolType getType() override;
    SymbolPtr getSymbol();
private:
    SymbolPtr _symbol;
    std::string _name;
};
typedef std::shared_ptr<IdentifierNode> IdentifierNodePtr;

class RecordAccessNode : public SynNode {
public:
    RecordAccessNode(const PNode&, const PNode&, SymbolPtr symbol = nullptr);
    std::string toString(std::string, bool);
    SymbolPtr getSymbol();
    PNode getRight();
private:
    SymbolPtr _symbol;
    PNode _left, _right;
};

class ArrayIndexNode : public SynNode {
public:
    ArrayIndexNode(const PNode&, const std::vector<PNode>&, SymbolPtr symbol);
    std::string toString(std::string, bool);
    SymbolPtr getSymbol();
private:
    std::vector<PNode> _args;
    PNode _arr;
    SymbolPtr _symbol;
};

class IfNode : public SynNode {
public:
    IfNode(PNode cond, PNode then, PNode else_block);
    std::string toString(std::string, bool);
private:
    PNode _cond, _then, _else;
};

class WhileNode : public SynNode {
public:
    WhileNode(PNode cond, PNode block);
    std::string toString(std::string, bool);
private:
    PNode _cond, _block;
};

class ForNode : public SynNode {
public:
    ForNode(SymbolPtr sym, PNode initial_exp, PNode final_exp, PNode body, bool isTo);
    std::string toString(std::string, bool);
private:
    PNode _initial, _final, _body;
    bool _isTo;
    SymbolPtr _symbol;
};

class RepeatNode : public SynNode {
public:
    RepeatNode(PNode cond, PNode body);
    std::string toString(std::string, bool);
private:
    PNode _cond, _body;
};

class BlockNode : public SynNode {
public:
    BlockNode(std::string& name);
    BlockNode(std::string&& name);
    BlockNode(std::vector<PNode>& statements, std::string& name);
    std::string toString(std::string, bool);
    void addStatement(PNode& statement);
private:
    std::string _name;
    std::vector<PNode> _statements;
};

class EmptyNode : public SynNode {
public:
    EmptyNode();
    std::string toString(std::string, bool);
};

class AssignmentNode : public BinOpNode {
public:
    AssignmentNode(TokenPtr t, PNode left, PNode right);
};

class CallNode : public SynNode {
public:
    CallNode(PNode expr, std::vector<PNode> args, SymbolPtr symbol);
    SymbolPtr getSymbol();
    std::string toString(std::string, bool);
private:
    PNode _expr;
    std::vector<PNode> _args;
    SymbolPtr _symbol;
};
#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Token.h"


enum class BinOpType {
    Add,
    Sub,
    Mult,
    Div
};

class SynNode {
public:
    virtual std::string toString(std::string, bool last) = 0;
protected:
    std::string makeIndent(std::string, bool);
    void updateIndentAndStr(std::string&, std::string&, bool);
};

typedef std::shared_ptr<SynNode> PNode;

class BinOpNode : public SynNode {
public:
    BinOpNode(TokenPtr, const PNode&, const PNode&);
    std::string toString(std::string, bool last);
private:
    std::string _sign;
    TokenType _op;
    PNode _left, _right;
};

class IntConstNode : public SynNode {
public:
    IntConstNode(int);
    std::string toString(std::string, bool);
private:
    int _value;
};

class RealNumberNode : public SynNode {
public:
    RealNumberNode(double);
    std::string toString(std::string, bool);
private:
    double _value;
};

class IdentifierNode : public SynNode {
public:
    IdentifierNode(std::string);
    std::string toString(std::string, bool);
private:
    std::string _name;
};

class RecordAccessNode : public SynNode {
public:
    RecordAccessNode(const PNode&, const PNode&);
    std::string toString(std::string, bool);
private:
    PNode _left, _right;
};

class ArrayIndexNode : public SynNode {
public:
    ArrayIndexNode(const PNode&, const std::vector<PNode>&);
    std::string toString(std::string, bool);
private:
    std::vector<PNode> _args;
    PNode _left;
};

class UnaryNode : public SynNode {
public:
    UnaryNode(TokenPtr, PNode);
    std::string toString(std::string, bool);
private:
    PNode _node;
    std::string _sign;
};
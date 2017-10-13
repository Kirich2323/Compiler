#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Token.h"
#include "Symbol.h"
#include "AsmGen.h"

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
    String,
    Identifier,
    BinaryOp,
    UnaryOp,
    AssignOp,
    RecordAccess,
    Call,
    ArrayIndex,
    Block,
    IfStmt,
    WhileStmt,
    ForStmt,
    RepeatStmt,
    Empty,
    Break,
    Continue
};

class SynNode {
public:
    SynNode(SynNodeType type);
    virtual std::string toString(std::string, bool last) = 0;
    SynNodeType getNodeType();
    virtual SymbolType getType();
    virtual void generate(AsmCode& asmCode) {} //make abstract
    virtual void generateLValue(AsmCode& asmCode) {} //make abstract
    virtual int getSize();
    virtual bool isLocal() { return false; }
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
    virtual void generate(AsmCode& asmCode);
private:
    PNode _arg;
};

class BinOpNode : public OpNode {
public:
    BinOpNode(TokenPtr, const PNode&, const PNode&);
    std::string toString(std::string, bool last);
    void generate(AsmCode& asmCode);
    void generateInt(AsmCode& asmCode);
    void generateReal(SymbolType leftType, SymbolType rightType, AsmCode& asmCode);
    void generateIntRelation(AsmCode& asmCode);
    void generateRealRelation(AsmCode& asmCode);
    void generateBoolean(AsmCode& asmCode);
    PNode getLeft();
    PNode getRight();
    SymbolType getType() override;
protected:
    PNode _left, _right;
};
typedef std::shared_ptr<BinOpNode> BinOpNodePtr;

class IntConstNode : public SynNode {
public:
    IntConstNode(int);
    std::string toString(std::string, bool);
    void generate(AsmCode& asmCode);
    SymbolType getType() override;
    int getValue();
private:
    int _value;
};

class RealConstNode : public SynNode {
public:
    RealConstNode(double);
    std::string toString(std::string, bool) override;
    void generate(AsmCode& asmCode) override;
    SymbolType getType() override;
    double getValue();
private:
    double _value;
};

class StringConstNode : public SynNode {
public:
    StringConstNode(std::string value);
    std::string toString(std::string indent, bool last);
    SymbolType getType() override;
    std::string getValue();
private:
    std::string _value;
};

class IdentifierNode : public SynNode {
public:
    IdentifierNode(std::string name, SymbolPtr symbol);
    std::string getName();
    std::string toString(std::string, bool);
    SymbolType getType() override;
    SymbolPtr getSymbol();
    int getSize() override;
    void generate(AsmCode& asmCode) override;
    void generateLValue(AsmCode& asmCode) override;
    bool isLocal() override;
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
    int getSize() override;
    SymbolType getType() override;
    void generate(AsmCode& asmCode) override;
    void generateLValue(AsmCode& asmCode) override;
    bool isLocal() override;
private:
    SymbolPtr _symbol;
    PNode _left, _right;
};

class ArrayIndexNode : public SynNode {
public:
    ArrayIndexNode(const PNode&, const std::vector<PNode>&, SymbolPtr symbol);
    std::string toString(std::string, bool);
    SymbolPtr getSymbol();
    SymbolType getType() override;
    void generate(AsmCode& asmCode);
    void generateLValue(AsmCode& asmCode) override;
    bool isLocal() override;
private:
    void generateIdx(AsmCode& asmCode);
    std::vector<PNode> _args;
    PNode _arr;
    SymbolPtr _symbol;
};

class IfNode : public SynNode {
public:
    IfNode(PNode cond, PNode then, PNode else_block);
    std::string toString(std::string, bool);
    void generate(AsmCode& asmCode);
private:
    PNode _cond, _then, _else;
};

class WhileNode : public SynNode {
public:
    WhileNode(PNode cond, PNode block);
    std::string toString(std::string, bool);
    void generate(AsmCode& asmCode);
private:
    PNode _cond, _block;
};

class ForNode : public SynNode {
public:
    ForNode(SymbolPtr sym, PNode initial_exp, PNode final_exp, PNode body, bool isTo);
    std::string toString(std::string, bool);
    void generate(AsmCode& asmCode);
private:
    PNode _initial, _final, _body;
    bool _isTo;
    SymbolPtr _symbol;
};

class RepeatNode : public SynNode {
public:
    RepeatNode(PNode cond, PNode body);
    std::string toString(std::string, bool);
    void generate(AsmCode& asmCode);
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
    void generate(AsmCode& asmCode);
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
    void generate(AsmCode& asmCode);
};

class CallNode : public SynNode {
public:
    CallNode(PNode expr, std::vector<PNode> args, SymbolPtr symbol = SymbolPtr(nullptr));
    SymbolPtr getSymbol();
    SymbolType getType() override;
    std::string toString(std::string, bool);
    int getSize() override;
    void generate(AsmCode& asmCode) override;
    void generateLValue(AsmCode& asmCode) override;
    bool isLocal() override { return true; }
protected:
    PNode _expr;
    std::vector<PNode> _args;
    SymbolPtr _symbol;
};

class WriteNode : public CallNode {
public:
    WriteNode(PNode expr, std::vector<PNode> args);
    void generate(AsmCode& asmCode) override;
    std::string toString(std::string indent, bool last);
};

class WritelnNode : public WriteNode {
public:
    WritelnNode(PNode expr, std::vector<PNode> args);
    std::string toString(std::string indent, bool last);
    void generate(AsmCode& asmCode) override;
};

class BreakNode : public SynNode {
public:
    BreakNode();
    void generate(AsmCode& asmCode) override;
    std::string toString(std::string indent, bool last);
};

class ContinueNode : public SynNode {
public:
    ContinueNode();
    void generate(AsmCode& asmCode) override;
    std::string toString(std::string indent, bool last);
};
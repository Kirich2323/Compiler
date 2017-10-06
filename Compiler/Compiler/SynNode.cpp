#include "SynNode.h"

SynNode::SynNode(SynNodeType type) : _type(type) {}

SynNodeType SynNode::getNodeType() {
    return _type;
}

SymbolType SynNode::getType() {
    return SymbolType::None;
}

bool SynNode::operator==(SynNodeType type) {
    return type == _type;
}

bool SynNode::operator!=(SynNodeType type) {
    return type != _type;
}

std::string SynNode::makeIndent(std::string indent, bool last) {
    return indent + (last ? "\\- " : "|- ");
}

void SynNode::updateIndentAndStr(std::string& str, std::string& indent, bool last) {
    if (last) {
        str += "\\- ";
        indent += "   ";
    }
    else {
        str += "|- ";
        indent += "|  ";
    }
}

BinOpNode::BinOpNode(TokenPtr t, const PNode& left, const PNode& right) :
    OpNode(t, SynNodeType::BinaryOp),
    _left(left),
    _right(right) {}

std::string BinOpNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += _sign;
    str += "\n" + _left->toString(indent, false);
    str += "\n" + _right->toString(indent, true);
    return str;
}

PNode BinOpNode::getLeft() {
    return _left;
}

PNode BinOpNode::getRight() {
    return _right;
}

SymbolType BinOpNode::getType() {
    return SymbolType();
}

IntConstNode::IntConstNode(int value) : SynNode(SynNodeType::IntegerNumber), _value(value) {}

std::string IntConstNode::toString(std::string indent, bool last) {
    return makeIndent(indent, last) + std::to_string(_value);
}

SymbolType IntConstNode::getType() {
    return SymbolType::TypeInteger;
}

int IntConstNode::getValue() {
    return _value;
}

IdentifierNode::IdentifierNode(std::string name, SymbolPtr symbol) : SynNode(SynNodeType::Identifier), _name(name), _symbol(symbol) {}

std::string IdentifierNode::getName() {
    return _name;
}

std::string IdentifierNode::toString(std::string indent, bool last) {
    return makeIndent(indent, last) + _name;
}

SymbolType IdentifierNode::getType() {
    SymbolType type = _symbol->getType();
    if (type == SymbolType::Proc || type == SymbolType::TypeRecord)
        return type;
    else if (type == SymbolType::Func)
        return std::dynamic_pointer_cast<SymProcBase>(_symbol)->getArgs()->getSymbol("result")->getVarType();
    return _symbol->getVarType();
}

SymbolPtr IdentifierNode::getSymbol() {
    return _symbol;
}

RealConstNode::RealConstNode(double value) : SynNode(SynNodeType::RealNumber), _value(value) {}

std::string RealConstNode::toString(std::string indent, bool last) {
    return makeIndent(indent, last) + std::to_string(_value);
}

SymbolType RealConstNode::getType() {
    return SymbolType::TypeReal;
}

double RealConstNode::getValue() {
    return _value;
}

RecordAccessNode::RecordAccessNode(const PNode& left, const PNode& right, SymbolPtr symbol) :
    SynNode(SynNodeType::RecordAcces),
    _left(left),
    _right(right),
    _symbol(symbol) {}

std::string RecordAccessNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += '.';
    str += "\n" + _left->toString(indent, false);
    str += "\n" + _right->toString(indent, true);
    return str;
}

SymbolPtr RecordAccessNode::getSymbol() {
    return _symbol;
}

PNode RecordAccessNode::getRight() {
    return _right;
}

ArrayIndexNode::ArrayIndexNode(const PNode& left, const std::vector<PNode>& args, SymbolPtr symbol) :
    SynNode(SynNodeType::ArrayIndex),
    _arr(left),
    _args(args),
    _symbol(symbol) {}

std::string ArrayIndexNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += "[]";
    str += "\n" + _arr->toString(indent, false);
    for (int i = 0; i < _args.size() - 1; ++i) {
        str += "\n" + _args[i]->toString(indent, false);
    }
    str += "\n" + _args.back()->toString(indent, true);
    return str;
}

SymbolPtr ArrayIndexNode::getSymbol() {
    return _symbol;
}

UnaryNode::UnaryNode(TokenPtr t, PNode node) : OpNode(t, SynNodeType::UnaryOp),
_arg(node) {}

std::string UnaryNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += _sign;
    str += "\n" + _arg->toString(indent, true);
    return str;
}

PNode UnaryNode::getArg() {
    return _arg;
}

SymbolType UnaryNode::getType() {
    return _arg->getType();
}

IfNode::IfNode(PNode cond, PNode then, PNode else_block) :
    SynNode(SynNodeType::IfStmt),
    _cond(cond), _then(then), _else(else_block) {}

std::string IfNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += "if";
    str += "\n" + _cond->toString(indent, false);
    bool else_exist = _else != nullptr;
    str += "\n" + _then->toString(indent, !else_exist);
    if (else_exist)
        str += "\n" + _else->toString(indent, true);
    return str;
}


WhileNode::WhileNode(PNode cond, PNode block) :
    SynNode(SynNodeType::WhileStmt),
    _cond(cond), _block(block) {}

std::string WhileNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += "while";
    str += "\n" + _cond->toString(indent, false);
    str += "\n" + _block->toString(indent, true);
    return str;
}

ForNode::ForNode(SymbolPtr sym, PNode initial_exp, PNode final_exp, PNode body, bool isTo) :
    SynNode(SynNodeType::ForStmt),
    _initial(initial_exp), _final(final_exp), _body(body),
    _symbol(sym), _isTo(isTo) {}

std::string ForNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += "for";
    str += "\n" + makeIndent(indent, false) + _symbol->getName();
    str += "\n" + makeIndent(indent, false) + (_isTo ? "to" : "Downto");
    str += "\n" + _initial->toString(indent, false);
    str += "\n" + _final->toString(indent, false);
    str += "\n" + _body->toString(indent, true);
    return str;
}

RepeatNode::RepeatNode(PNode cond, PNode body) :
    SynNode(SynNodeType::RepeatStmt),
    _cond(cond), _body(body) {}

std::string RepeatNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += "repeat";
    str += "\n" + _body->toString(indent, false);
    str += "\n" + _cond->toString(indent, true);
    return str;
}

BlockNode::BlockNode(std::string& name) :
    SynNode(SynNodeType::Block),
    _name(name) {}

BlockNode::BlockNode(std::string&& name) :
    SynNode(SynNodeType::Block), _name(name) {}

BlockNode::BlockNode(std::vector<PNode>& statements, std::string & name) :
    SynNode(SynNodeType::Block),
    _statements(statements), _name(name) {}

std::string BlockNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += _name;
    for (int i = 0; i < (int)_statements.size() - 1; ++i)
        str += "\n" + _statements[i]->toString(indent, false);
    if (_statements.size() > 0)
        str += "\n" + _statements.back()->toString(indent, true);
    else 
        str += "\n" + EmptyNode().toString(indent, true);
    return str;
}

void BlockNode::addStatement(PNode& statement) {
    _statements.push_back(statement);
}

EmptyNode::EmptyNode() :
    SynNode(SynNodeType::Empty) {}

std::string EmptyNode::toString(std::string indent, bool last) {
    return makeIndent(indent, last) + "empty node";
}

OpNode::OpNode(TokenPtr tok, SynNodeType type) : SynNode(type), _op(tok->getType()), _sign(tok->getText()) {}

TokenType OpNode::getOpType() {
    return _op;
}

AssignmentNode::AssignmentNode(TokenPtr t, PNode left, PNode right) : BinOpNode(t, left, right) {}

CallNode::CallNode(PNode expr, std::vector<PNode> args, SymbolPtr symbol) : SynNode(SynNodeType::Call),
_expr(expr), _args(args), _symbol(symbol) {}

SymbolPtr CallNode::getSymbol() {
    return _symbol;
}

std::string CallNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += "call";
    bool haveArgs = _args.size() != 0;
    str += "\n" + _expr->toString(indent, !haveArgs);
    for (int i = 0; i < (int)_args.size() - 1; ++i) {
        str += "\n" + _args[i]->toString(indent, false);
    }
    if (haveArgs)
        str += "\n" + _args.back()->toString(indent, true);
    return str;
}
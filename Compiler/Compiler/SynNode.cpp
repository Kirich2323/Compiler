#include "SynNode.h"

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
    _op(t->getType()),
    _sign(t->getText()),
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

IntConstNode::IntConstNode(int value) : _value(value) {}

std::string IntConstNode::toString(std::string indent, bool last) {
    return makeIndent(indent, last) + std::to_string(_value);
}

IdentifierNode::IdentifierNode(std::string name) : _name(name) {}

std::string IdentifierNode::toString(std::string indent, bool last) {
    return makeIndent(indent, last) + _name;
}

RealNumberNode::RealNumberNode(double value) : _value(value) {}

std::string RealNumberNode::toString(std::string indent, bool last) {
    return makeIndent(indent, last) + std::to_string(_value);
}

RecordAccessNode::RecordAccessNode(const PNode& left, const PNode& right) :
    _left(left),
    _right(right) {}

std::string RecordAccessNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += '.';
    str += "\n" + _left->toString(indent, false);
    str += "\n" + _right->toString(indent, true);
    return str;
}

ArrayIndexNode::ArrayIndexNode(const PNode& left, const std::vector<PNode>& args) : _left(left), _args(args) {}

std::string ArrayIndexNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += "[]";
    str += "\n" + _left->toString(indent, false);
    for (int i = 0; i < _args.size() - 1; ++i) {
      str += "\n" + _args[i]->toString(indent, false);
    }
    str += "\n" + _args.back()->toString(indent, true);   
    return str;
}

UnaryNode::UnaryNode(TokenPtr t, PNode node) : _node(node), _sign(t->getText()) {}

std::string UnaryNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += _sign;
    str += "\n" + _node->toString(indent, true);
    return str;
}

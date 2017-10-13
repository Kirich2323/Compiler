#include "SynNode.h"
#include "TypeChecker.h"

SynNode::SynNode(SynNodeType type) : _type(type) {}

SynNodeType SynNode::getNodeType() {
    return _type;
}

SymbolType SynNode::getType() {
    return SymbolType::None;
}

int SynNode::getSize() {
    return 0;
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

void BinOpNode::generate(AsmCode & asmCode) {
    _left->generate(asmCode);
    _right->generate(asmCode);
    SymbolType leftType = _left->getType();
    SymbolType rightType = _right->getType();
    SymbolType castedType = TypeChecker::tryCast(leftType, rightType);
    switch (castedType) {
        case SymbolType::TypeInteger:  generateInt(asmCode); break;
        case SymbolType::TypeReal:     generateReal(leftType, rightType, asmCode);  break;
        case SymbolType::TypeBoolean:  break;
    }
}

void BinOpNode::generateInt(AsmCode & asmCode) {
    asmCode.addCmd(POP, RBX);
    asmCode.addCmd(POP, RAX);
    switch (_op) {
        case TokenType::Add:
            asmCode.addCmd(ADD, RAX, RBX);
            break;
        case TokenType::Sub:
            asmCode.addCmd(SUB, RAX, RBX);
            break;
        case TokenType::Mul:
            asmCode.addCmd(IMUL, RBX);
            break;
        case TokenType::Div:
            asmCode.addCmd(XOR, RDX, RDX);
            asmCode.addCmd(IDIV, RBX);
            break;
        case TokenType::Mod:
            asmCode.addCmd(XOR, RDX, RDX);
            asmCode.addCmd(IDIV, RBX);
            asmCode.addCmd(MOV, RAX, RDX);
            break;
        case TokenType::And:
            asmCode.addCmd(AND, RAX, RBX);
            break;
        case TokenType::Or:
            asmCode.addCmd(OR, RAX, RBX);
            break;
        case TokenType::Xor:
            asmCode.addCmd(XOR, RAX, RBX);
        case TokenType::Shl:
            asmCode.addCmd(MOV, RCX, RBX);
            asmCode.addCmd(SHL, RAX, CL);
            break;
        case TokenType::Shr:
            asmCode.addCmd(MOV, RCX, RBX);
            asmCode.addCmd(SHR, RAX, CL);
            break;
        default:
            generateIntRelation(asmCode);
            break;
    }
    asmCode.addCmd(PUSH, RAX);
}

void BinOpNode::generateReal(SymbolType leftType, SymbolType rightType, AsmCode & asmCode) {
    asmCode.addCmd(POP, RBX);
    asmCode.addCmd(POP, RAX);
    if (leftType == SymbolType::TypeInteger)
        asmCode.addCmd(CVTSI2SD, XMM0, RAX);
    else
        asmCode.addCmd(MOVQ, XMM0, RAX);

    if (rightType == SymbolType::TypeInteger)
        asmCode.addCmd(CVTSI2SD, XMM1, RBX);
    else
        asmCode.addCmd(MOVQ, XMM1, RBX);

    bool isRelation = false;
    switch (_op) {
        case TokenType::Add:
            asmCode.addCmd(ADDSD, XMM0, XMM1);
            break;
        case TokenType::Sub:
            asmCode.addCmd(SUBSD, XMM0, XMM1);
            break;
        case TokenType::Mul:
            asmCode.addCmd(MULSD, XMM0, XMM1);
            break;
        case TokenType::DivReal:
            asmCode.addCmd(DIVSD, XMM0, XMM1);
            break;
        default:
            isRelation = true;
            generateRealRelation(asmCode);
            break;
    }
    if (!isRelation)
        asmCode.addCmd(MOVQ, RAX, XMM0);
    asmCode.addCmd(PUSH, RAX);
}

void BinOpNode::generateIntRelation(AsmCode & asmCode) {
    asmCode.addCmd(CMP, RAX, RBX);
    std::string label1 = asmCode.genLabelName();
    std::string label2 = asmCode.genLabelName();
    switch (_op) {
        case TokenType::Equal:
            asmCode.addCmd(JE, label1);
            break;
        case TokenType::NotEqual:
            asmCode.addCmd(JNE, label1);
            break;
        case TokenType::Less:
            asmCode.addCmd(JL, label1);
            break;
        case TokenType::LessEqual:
            asmCode.addCmd(JLE, label1);
            break;
        case TokenType::GreaterEqual:
            asmCode.addCmd(JGE, label1);
            break;
        case TokenType::Greater:
            asmCode.addCmd(JG, label1);
            break;
    }
    asmCode.addCmd(XOR, RAX, RAX);
    asmCode.addCmd(JMP, label2);
    asmCode.addLabel(label1);
    asmCode.addCmd(MOV, RAX, 1);
    asmCode.addLabel(label2);
}

void BinOpNode::generateRealRelation(AsmCode& asmCode) {
    asmCode.addCmd(COMISD, XMM0, XMM1);
    std::string label1 = asmCode.genLabelName();
    std::string label2 = asmCode.genLabelName();
    switch (_op) {
        case TokenType::Equal:
            asmCode.addCmd(JE, label1);
            break;
        case TokenType::NotEqual:
            asmCode.addCmd(JNE, label1);
            break;
        case TokenType::Less:
            asmCode.addCmd(JB, label1);
            break;
        case TokenType::LessEqual:
            asmCode.addCmd(JBE, label1);
            break;
        case TokenType::GreaterEqual:
            asmCode.addCmd(JAE, label1);
            break;
        case TokenType::Greater:
            asmCode.addCmd(JA, label1);
            break;
    }
    asmCode.addCmd(MOV, RAX, 0);
    asmCode.addCmd(JMP, label2);
    asmCode.addLabel(label1);
    asmCode.addCmd(MOV, RAX, 1);
    asmCode.addLabel(label2);
}

void BinOpNode::generateBoolean(AsmCode & asmCode) {
    asmCode.addCmd(POP, RBX);
    asmCode.addCmd(POP, RAX);
    switch (_op) {
        case TokenType::And:
            asmCode.addCmd(AND, RAX, RBX);
            break;
        case TokenType::Or:
            asmCode.addCmd(OR, RAX, RBX);
            break;
        case TokenType::Xor:
            asmCode.addCmd(XOR, RAX, RBX);
            break;
    }
    std::string falseLabel = asmCode.genLabelName();
    std::string endLabel = asmCode.genLabelName();
    asmCode.addCmd(TEST, RAX, RAX);
    asmCode.addCmd(JZ, falseLabel);
    asmCode.addCmd(MOV, RAX, 0);
    asmCode.addCmd(JMP, endLabel);
    asmCode.addLabel(falseLabel);
    asmCode.addCmd(MOV, RAX, 1);
    asmCode.addLabel(endLabel);
    asmCode.addCmd(PUSH, RAX);
}

PNode BinOpNode::getLeft() {
    return _left;
}

PNode BinOpNode::getRight() {
    return _right;
}

SymbolType BinOpNode::getType() {
    return TypeChecker::tryCast(_left->getType(), _right->getType());
}

IntConstNode::IntConstNode(int value) : SynNode(SynNodeType::IntegerNumber), _value(value) {}

std::string IntConstNode::toString(std::string indent, bool last) {
    return makeIndent(indent, last) + std::to_string(_value);
}

void IntConstNode::generate(AsmCode & asmCode) {
    asmCode.addCmd(MOV, RAX, _value);
    asmCode.addCmd(PUSH, RAX);
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

int IdentifierNode::getSize() {
    switch (_symbol->getType()) {
        case SymbolType::Func:
            return std::dynamic_pointer_cast<SymVar>(std::dynamic_pointer_cast<SymProcBase>(_symbol)->getArgs()->getSymbol("result"))->getSize();
        default:
            return _symbol->getSize();
    }
}

void IdentifierNode::generate(AsmCode & asmCode) {
    _symbol->generate(asmCode);
}

void IdentifierNode::generateLValue(AsmCode & asmCode) {
    _symbol->generateLValue(asmCode);
}

bool IdentifierNode::isLocal() {
    return !(_symbol->getType() == SymbolType::VarGlobal || _symbol->getType() == SymbolType::VarParam);
}

RealConstNode::RealConstNode(double value) : SynNode(SynNodeType::RealNumber), _value(value) {}

std::string RealConstNode::toString(std::string indent, bool last) {
    return makeIndent(indent, last) + std::to_string(_value);
}

void RealConstNode::generate(AsmCode & asmCode) {
    std::string name = asmCode.genVarName();
    asmCode.addData(name, _value);
    asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(name));
    asmCode.addCmd(PUSH, RAX);
}

SymbolType RealConstNode::getType() {
    return SymbolType::TypeReal;
}

double RealConstNode::getValue() {
    return _value;
}

RecordAccessNode::RecordAccessNode(const PNode& left, const PNode& right, SymbolPtr symbol) :
    SynNode(SynNodeType::RecordAccess),
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

int RecordAccessNode::getSize() {
    return _right->getSize();
}

SymbolType RecordAccessNode::getType() {
    return _right->getType();
}

void RecordAccessNode::generate(AsmCode & asmCode) {
    generateLValue(asmCode);
    asmCode.addCmd(POP, RAX);
    asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(RAX));
    asmCode.addCmd(PUSH, RAX);  
}

void RecordAccessNode::generateLValue(AsmCode & asmCode) {
    _left->generateLValue(asmCode);
    asmCode.addCmd(POP, RAX);
    AsmOpType op = _left->isLocal() ? SUB : ADD;
    asmCode.addCmd(op, RAX, std::dynamic_pointer_cast<IdentifierNode>(_right)->getSymbol()->getOffset());
    asmCode.addCmd(PUSH, RAX);
}

bool RecordAccessNode::isLocal() {
    return _left->isLocal();
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
    for (int i = 0; i < _args.size() - 1; ++i)
        str += "\n" + _args[i]->toString(indent, false);
    str += "\n" + _args.back()->toString(indent, true);
    return str;
}

SymbolPtr ArrayIndexNode::getSymbol() {
    return _symbol;
}

SymbolType ArrayIndexNode::getType() {
    SymbolType type = _symbol->getVarType();
    if (type == SymbolType::TypeArray)
        type = std::dynamic_pointer_cast<SymTypeArray>(std::dynamic_pointer_cast<SymVar>(_symbol)->getVarTypeSymbol())->getArrType();
    return type;
}

void ArrayIndexNode::generate(AsmCode & asmCode) {
    generateLValue(asmCode);
    SymbolPtr arrSym = _symbol->getVarTypeSymbol()->getVarTypeSymbol();
    if (arrSym->getSize() <= 8) {
        asmCode.addCmd(POP, RAX);
        asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(RAX));
        asmCode.addCmd(PUSH, RAX);
    }
    else {
        std::string label1 = asmCode.genLabelName();
        std::string label2 = asmCode.genLabelName();
        asmCode.addCmd(POP, RAX);
        asmCode.addCmd(MOV, RBX, arrSym->getSize());
        asmCode.addLabel(label1);
        asmCode.addCmd(CMP, RBX, 0);
        asmCode.addCmd(JLE, label2);        
        asmCode.addCmd(MOV, RDX, asmCode.getAdressOperand(RAX));
        asmCode.addCmd(PUSH, RDX);
        asmCode.addCmd(ADD, RAX, 8);
        asmCode.addCmd(SUB, RBX, 8);
        asmCode.addCmd(JMP, label1);
        asmCode.addLabel(label2);
        //asmCode.addCmd(PUSH, RSP);
    }    
}

void ArrayIndexNode::generateLValue(AsmCode & asmCode) {
    generateIdx(asmCode);
}

bool ArrayIndexNode::isLocal() {
    return _arr->isLocal();
}

void ArrayIndexNode::generateIdx(AsmCode & asmCode) {
    _arr->generateLValue(asmCode);
    SymbolPtr type = std::dynamic_pointer_cast<SymVar>(_symbol)->getVarTypeSymbol();
    int left = 0;
    AsmOpType asmOp;
    asmOp = std::dynamic_pointer_cast<IdentifierNode>(_arr)->isLocal() ? SUB : ADD;
    for (auto arg : _args) {
        if (type->getType() == SymbolType::TypeArray) {
            left = std::dynamic_pointer_cast<SymTypeArray>(type)->getLeft();
            type = std::dynamic_pointer_cast<SymTypeArray>(type)->getTypeSymbol();
        }
        arg->generate(asmCode);
        asmCode.addCmd(POP, RAX);
        asmCode.addCmd(MOV, RBX, left);
        asmCode.addCmd(SUB, RAX, RBX);
        asmCode.addCmd(MOV, RBX, type->getSize());
        asmCode.addCmd(IMUL, RBX);
        asmCode.addCmd(POP, RBX);
        asmCode.addCmd(asmOp, RBX, RAX);
        asmCode.addCmd(PUSH, RBX);
    }
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

void UnaryNode::generate(AsmCode & asmCode) {
    _arg->generate(asmCode);
    if (_arg->getType() == SymbolType::TypeInteger) {
        asmCode.addCmd(POP, RAX);
        switch (_op) {
            case TokenType::Sub:
                asmCode.addCmd(NEG, RAX);
                break;
            case TokenType::Not:
                asmCode.addCmd(XOR, RAX, 1);
        }
        asmCode.addCmd(PUSH, RAX);
    }
    else if (_arg->getType() == SymbolType::TypeReal && _op == TokenType::Sub) {
        unsigned long long signBit = 1;
        signBit <<= 63;
        asmCode.addCmd(POP, RAX);
        asmCode.addCmd(MOV, RBX, std::to_string(signBit));
        asmCode.addCmd(XOR, RAX, RBX);
        asmCode.addCmd(PUSH, RAX);
    }
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

void IfNode::generate(AsmCode & asmCode) {
    _cond->generate(asmCode);
    std::string label1 = asmCode.genLabelName();
    std::string label2 = asmCode.genLabelName();
    asmCode.addCmd(POP, RAX);
    asmCode.addCmd(TEST, RAX, RAX);
    asmCode.addCmd(JZ, label1);
    _then->generate(asmCode);
    asmCode.addCmd(JMP, label2);
    asmCode.addLabel(label1);
    if (_else)
        _else->generate(asmCode);
    asmCode.addLabel(label2);
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

void WhileNode::generate(AsmCode& asmCode) {
    std::string start = asmCode.genLabelName();
    std::string cond = asmCode.genLabelName();
    std::string end = asmCode.genLabelName();
    asmCode.addLoopLabels(cond, end);
    asmCode.addCmd(JMP, cond);
    asmCode.addLabel(start);
    _block->generate(asmCode);
    asmCode.addLabel(cond);
    _cond->generate(asmCode);
    asmCode.addCmd(POP, RAX);
    asmCode.addCmd(TEST, RAX, RAX);
    asmCode.addCmd(JNZ, start);
    asmCode.addLabel(end);
    asmCode.popLoopLabels();
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

void ForNode::generate(AsmCode & asmCode) {
    std::string cond = asmCode.genLabelName();
    std::string body = asmCode.genLabelName();
    std::string inc = asmCode.genLabelName();
    std::string end = asmCode.genLabelName();
    asmCode.addLoopLabels(inc, end);
    _symbol->generateLValue(asmCode);
    _initial->generate(asmCode);
    asmCode.addCmd(POP, RBX);
    asmCode.addCmd(POP, RAX);
    asmCode.addCmd(MOV, asmCode.getAdressOperand(RAX), RBX);
    asmCode.addCmd(JMP, cond);
    asmCode.addLabel(body);
    _body->generate(asmCode);
    asmCode.addLabel(inc);
    _symbol->generateLValue(asmCode);
    asmCode.addCmd(POP, RBX);
    asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(RBX));
    asmCode.addCmd(_isTo ? ADD : SUB, RAX, 1);
    asmCode.addCmd(MOV, asmCode.getAdressOperand(RBX), RAX);
    asmCode.addLabel(cond);
    _final->generate(asmCode);
    _symbol->generateLValue(asmCode);
    asmCode.addCmd(POP, RBX);
    asmCode.addCmd(POP, RAX);
    asmCode.addCmd(MOV, RBX, asmCode.getAdressOperand(RBX));
    asmCode.addCmd(CMP, RBX, RAX);
    asmCode.addCmd(_isTo ? JLE : JGE, body);
    asmCode.addLabel(end);
    asmCode.popLoopLabels();
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

void RepeatNode::generate(AsmCode & asmCode) {
    std::string cond = asmCode.genLabelName();
    std::string body = asmCode.genLabelName();
    std::string end = asmCode.genLabelName();
    asmCode.addLoopLabels(cond, end);
    asmCode.addLabel(body);
    _body->generate(asmCode);
    asmCode.addLabel(cond);
    _cond->generate(asmCode);
    asmCode.addCmd(POP, RAX);
    asmCode.addCmd(TEST, RAX, RAX);
    asmCode.addCmd(JZ, body);
    asmCode.addLabel(end);
    asmCode.popLoopLabels();
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

void BlockNode::generate(AsmCode & asmCode) {
    for (auto stmt : _statements)
        stmt->generate(asmCode);
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

void AssignmentNode::generate(AsmCode & asmCode) {
    _right->generate(asmCode);
    _left->generateLValue(asmCode);
    asmCode.addCmd(POP, RAX);
    if (_left->getSize() <= 8) {
        asmCode.addCmd(POP, RBX);
        asmCode.addCmd(MOV, asmCode.getAdressOperand(RAX), RBX);
    }
    else {
        std::string labelBegin = asmCode.genLabelName();
        std::string labelEnd = asmCode.genLabelName();
        asmCode.addCmd(MOV, RBX, _left->getSize());
        asmCode.addCmd(LEA, RAX, asmCode.getAdressOperand(RAX, _left->getSize() - 8));
        asmCode.addLabel(labelBegin);
        asmCode.addCmd(CMP, RBX, 0);
        asmCode.addCmd(JLE, labelEnd);
        asmCode.addCmd(POP, RDX);
        asmCode.addCmd(MOV, asmCode.getAdressOperand(RAX), RDX);
        asmCode.addCmd(SUB, RAX, 8);
        asmCode.addCmd(SUB, RBX, 8);
        asmCode.addCmd(JMP, labelBegin);
        asmCode.addLabel(labelEnd);
    }
}

CallNode::CallNode(PNode expr, std::vector<PNode> args, SymbolPtr symbol) : SynNode(SynNodeType::Call),
_expr(expr), _args(args), _symbol(symbol) {}

SymbolPtr CallNode::getSymbol() {
    return _symbol;
}

SymbolType CallNode::getType() {
    return std::dynamic_pointer_cast<SymProcBase>(_symbol)->getArgs()->getSymbol("result")->getVarType();
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

int CallNode::getSize() {
    return std::dynamic_pointer_cast<SymProcBase>(_symbol)->getArgs()->getSymbol("result")->getSize();
}

void CallNode::generate(AsmCode & asmCode) {
    int size = 0;
    SymTablePtr table = std::dynamic_pointer_cast<SymProcBase>(_symbol)->getArgs();
    if (_symbol->getType() == SymbolType::Func) {
        size = table->getSymbol("result")->getSize();
        asmCode.addCmd(SUB, RSP, size);
    }
    for (int i = 0; i < _args.size(); ++i) {
        if (table->getSymbols()[i]->getType() == SymbolType::Param)
            _args[i]->generate(asmCode);
        else
            _args[i]->generateLValue(asmCode);
    }
    asmCode.addCmd(CALL, _symbol->getName() + std::to_string(std::dynamic_pointer_cast<SymProcBase>(_symbol)->getDepth()));
    asmCode.addCmd(ADD, RSP, table->getSize() - size);
}

void CallNode::generateLValue(AsmCode & asmCode) {
    generate(asmCode);
    asmCode.addCmd(MOV, RAX, RSP);
    asmCode.addCmd(ADD, RAX, std::dynamic_pointer_cast<SymProcBase>(_symbol)->getArgs()->getSymbol("result")->getSize() - 8);
    asmCode.addCmd(PUSH, RAX);
}

WriteNode::WriteNode(PNode expr, std::vector<PNode> args) : CallNode(expr, args) {}

void WriteNode::generate(AsmCode & asmCode) {
    for (auto arg : _args) {
        switch (arg->getType()) {
            case SymbolType::TypeInteger:
                arg->generate(asmCode);
                asmCode.addWriteInt();
                break;
            case SymbolType::TypeReal:
                arg->generate(asmCode);
                asmCode.addWriteFloat();
                break;
            case SymbolType::TypeString:
                asmCode.addWriteString(std::dynamic_pointer_cast<StringConstNode>(arg)->getValue());
                break;
        }
    }
}

std::string WriteNode::toString(std::string indent, bool last) {
    return CallNode::toString(indent, last);
}

StringConstNode::StringConstNode(std::string value) : SynNode(SynNodeType::String), _value(value) {}

std::string StringConstNode::toString(std::string indent, bool last) {
    std::string str = indent;
    updateIndentAndStr(str, indent, last);
    str += "'" + _value + "'";
    return str;
}

SymbolType StringConstNode::getType() {
    return SymbolType::TypeString;
}

std::string StringConstNode::getValue() {
    return _value;
}

WritelnNode::WritelnNode(PNode expr, std::vector<PNode> args) : WriteNode(expr, args) {}

std::string WritelnNode::toString(std::string indent, bool last) {
    return WriteNode::toString(indent, last);
}

void WritelnNode::generate(AsmCode & asmCode) {
    WriteNode::generate(asmCode);
    asmCode.addWriteln();
}

BreakNode::BreakNode() : SynNode(SynNodeType::Break) {}

void BreakNode::generate(AsmCode & asmCode) {
    std::string label = asmCode.getBreak();
    if (!label.empty())
        asmCode.addCmd(JMP, label);
}

std::string BreakNode::toString(std::string indent, bool last) {
    return indent + "break";
}

ContinueNode::ContinueNode() : SynNode(SynNodeType::Continue) {}

void ContinueNode::generate(AsmCode & asmCode) {
    std::string label = asmCode.getContinue();
    if (!label.empty())
        asmCode.addCmd(JMP, label);
}

std::string ContinueNode::toString(std::string indent, bool last) {
    return indent + "continue";
}

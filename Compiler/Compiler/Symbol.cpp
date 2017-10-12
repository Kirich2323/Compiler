#include "Symbol.h"

Symbol::Symbol(SymbolType type, std::string name, int size) : _type(type), _name(name), _size(size) {}

SymbolType Symbol::getType() {
    return _type;
}

size_t Symbol::getSize() {
    return _size;
}

size_t Symbol::getOffset() {
    return _offset;
}

void Symbol::setOffset(size_t offset) {
    _offset = offset;
}

std::string Symbol::getName() {
    return _name;
}

bool Symbol::isType() {
    return false;
}

std::string Symbol::toString(int depth) {
    std::stringstream sstream;
    sstream << std::string(_spaces * depth, ' ') << std::left << std::setw(_firstColumnWidth) << _name;
    return sstream.str();
}

SymbolType Symbol::getVarType() {
    return SymbolType::None;
}

void Symbol::generate(AsmCode & asmCode) {}

void Symbol::generateLValue(AsmCode & asmCode) {}

void Symbol::generateDecl(AsmCode & asmCode) {}

SymType::SymType(SymbolType type, std::string name, int size) : Symbol(type, name, size) {}

bool SymType::isType() {
    return true;
}

size_t SymType::getSize() {
    return _size;
}

std::string SymType::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << "type ";
    return sstream.str();
}

SymVar::SymVar(std::string name, SymbolPtr varType, SymbolType type, Const* init) : Symbol(type, name), _varType(varType), _init(init) {}

std::string SymVar::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth)
        << "var " << std::setw(_thirdColumnWidth) << _varType->getName();
    if (_init != nullptr) {
        sstream << std::setw(_thirdColumnWidth);
        if (_varType->getType() == SymbolType::TypeInteger)
            sstream << _init->getValue<int>();
        else if (_varType->getType() == SymbolType::TypeReal)
            sstream << _init->getValue<double>();
    }
    return sstream.str();
}

SymbolType SymVar::getVarType() {
    SymbolType type = _varType->getType();
    if (type == SymbolType::TypeAlias)
        type = std::dynamic_pointer_cast<SymTypeAlias>(_varType)->getRefSymbol()->getType();
    return type;
}

void SymVar::generate(AsmCode & asmCode) {
    if (getType() == SymbolType::VarGlobal) {
        if (getSize() == 8) {
            asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(asmCode.getVarName(_name)));
            asmCode.addCmd(PUSH, RAX);
        }
        else {
            generateMemoryCopy(asmCode, AsmCmdPtr(new AsmCmd(MOV, AsmOperandPtr(new AsmReg(RAX)), AsmOperandPtr(new AsmStringImmediate(asmCode.getVarName(_name))))), ADD);
        }
    }
    else {
        if (getSize() == 8) {
            asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(RBP, -(int)getOffset() - 8));
            asmCode.addCmd(PUSH, RAX);
        }
        else {
            generateMemoryCopy(asmCode, AsmCmdPtr(new AsmCmd(LEA, AsmOperandPtr(new AsmReg(RAX)), asmCode.getAdressOperand(RBP, -(int)getOffset() - 8))), SUB);
        }
    }    
}

void SymVar::generateLValue(AsmCode & asmCode) {
    if (getType() == SymbolType::VarGlobal) {
        asmCode.addCmd(PUSH, asmCode.getVarName(_name));
    }
    else {
        asmCode.addCmd(LEA, RAX, asmCode.getAdressOperand(RBP, -(int)getOffset() - 8));
        asmCode.addCmd(PUSH, RAX);
    }
}

size_t SymVar::getSize() {
    return _varType->getSize();
}

SymbolPtr SymVar::getVarTypeSymbol() {
    SymbolPtr tmp = _varType;
    while (tmp->getType() == SymbolType::TypeAlias)
        tmp = std::dynamic_pointer_cast<SymTypeAlias>(tmp)->getRefSymbol();
    return tmp;
}

void SymVar::generateMemoryCopy(AsmCode & asmCode, AsmCmdPtr cmdMemory, AsmOpType opType) {
    std::string labelBegin = asmCode.genLabelName();
    std::string labelEnd = asmCode.genLabelName();
    asmCode.addCmd(cmdMemory);
    asmCode.addCmd(MOV, RBX, getSize());
    asmCode.addCmd(XOR, RCX, RCX);
    asmCode.addLabel(labelBegin);
    asmCode.addCmd(CMP, RCX, RBX);
    asmCode.addCmd(JGE, labelEnd);
    asmCode.addCmd(MOV, RDX, asmCode.getAdressOperand(RAX));
    asmCode.addCmd(PUSH, RDX);
    asmCode.addCmd(opType, RAX, 8);
    asmCode.addCmd(ADD, RCX, 8);
    asmCode.addCmd(JMP, labelBegin);
    asmCode.addLabel(labelEnd);
}

void SymVar::generateDecl(AsmCode & asmCode) {
    std::string varName = asmCode.getVarName(_name);
    SymbolType type = _varType->getType();
    type = type == SymbolType::TypeAlias ? std::dynamic_pointer_cast<SymTypeAlias>(_varType)->getRefType() : type;
    switch (type) {
        case SymbolType::TypeInteger:
            asmCode.addData(varName, _init == nullptr ? 0 : _init->getValue<int>());
            break;
        case SymbolType::TypeReal:
            asmCode.addData(varName, _init == nullptr ? 0.0 : _init->getValue<double>());
            break;
        case SymbolType::TypeRecord:
        case SymbolType::TypeArray:
            asmCode.addArrayData(varName, getSize());
            break;
    }
}

void SymTable::add(SymbolPtr symb) {
    if (!symb->isType()) {
        symb->setOffset(_size);
        _size += symb->getSize();
    }
    _symbolNames[symb->getName()] = _symbols.size();
    _symbols.push_back(symb);
}

bool SymTable::have(std::string name) {
    return _symbolNames.find(name) != _symbolNames.end();
}

SymbolPtr SymTable::getSymbol(std::string name) {
    return _symbols[_symbolNames[name]];
}

std::vector<SymbolPtr> SymTable::getSymbols() {
    return _symbols;
}

std::unordered_map<std::string, int> SymTable::getSymbolNames() {
    return _symbolNames;
}

void SymTable::checkUnique(TokenPtr token) {
    if (have(token->getText()))
        throw Duplicate(token->getLine(), token->getCol(), token->getText());
}

size_t SymTable::getSize() {
    return _size;
}

std::string SymTable::toString(int depth) {
    std::string str;
    for (int i = 0; i < (int)_symbols.size() - 1; ++i)
        str += _symbols[i]->toString(depth) + "\n";
    if (_symbols.size() > 0)
        str += _symbols.back()->toString(depth);
    return str;
}

SymTableStack::SymTableStack(SymTablePtr table) {
    if (table != nullptr)
        addTable(table);
}

void SymTableStack::addTable(SymTablePtr table) {
    _symTables.push_back(table);
}

SymTablePtr SymTableStack::top() {
    return _symTables.back();
}

void SymTableStack::pop() {
    _symTables.pop_back();
}

bool SymTableStack::haveSymbol(std::string name) {
    return findTableBySymbol(name) != nullptr;
}

SymbolPtr SymTableStack::getSymbol(TokenPtr token, bool isSymbolCheck) {
    std::string name = token->getText();
    SymTablePtr table = findTableBySymbol(name);
    if (table != nullptr) {
        return findTableBySymbol(name)->getSymbol(name);
    }
    else {
        if (isSymbolCheck)
            throw WrongSymbol(token->getLine(), token->getCol(), token->getText());
        else
            return SymbolPtr(new SymType(SymbolType::TypeInteger, "integer"));
    }
}

SymTablePtr SymTableStack::findTableBySymbol(const std::string& symbol) {
    for (auto i = _symTables.rbegin(); i != _symTables.rend(); ++i)
        if ((*i)->have(symbol))
            return *i;

    return nullptr;
}

SymTypeRecord::SymTypeRecord(SymTablePtr table) : SymType(SymbolType::TypeRecord, "record"), _symTable(table) {}

std::string SymTypeRecord::toString(int depth) {
    std::stringstream sstream;
    if (_symTable->getSymbols().size() > 0)
        sstream << std::endl << _symTable->toString(depth + 1);
    return sstream.str();
}

SymbolPtr SymTypeRecord::getSymbol(std::string& name) {
    return _symTable->getSymbol(name);
}

SymTablePtr SymTypeRecord::getTable() {
    return _symTable;
}

bool SymTypeRecord::have(std::string & name) {
    return _symTable->have(name);
}

size_t SymTypeRecord::getSize() {
    size_t size = 0;
    for (auto symb : _symTable->getSymbols())
        size += symb->getSize();
    return size;
}

SymTypeArray::SymTypeArray(SymbolPtr elemType, SymTypeSubrangePtr subrange) :
    SymType(SymbolType::TypeArray, "array"), _left(subrange->getLeft()), _right(subrange->getRight()), _elemType(elemType) {}

int SymTypeArray::getDimension() {
    int ans = 1;
    SymbolPtr tmp = _elemType;
    while (tmp->getType() == SymbolType::TypeArray) {
        ans += 1;
        tmp = std::dynamic_pointer_cast<SymTypeArray>(tmp)->_elemType;
    }
    return ans;
}

std::string SymTypeArray::getName() {
    std::stringstream sstream;
    sstream << "array " << "[" << std::to_string(_left) << ".." << std::to_string(_right) << "]" << " of " << _elemType->getName();
    return sstream.str();
}

SymbolType SymTypeArray::getArrType() {
    SymbolPtr tmp = _elemType;
    while (tmp->getType() == SymbolType::TypeArray) {
        tmp = std::dynamic_pointer_cast<SymTypeArray>(tmp)->_elemType;
    }
    return tmp->getType();
}

SymbolPtr SymTypeArray::getTypeSymbol() {
    return _elemType;
}

size_t SymTypeArray::getSize() {
    return (_right - _left + 1) * _elemType->getSize();
}

int SymTypeArray::getLeft() {
    return _left;
}

SymTypeSubrange::SymTypeSubrange(int left, int right) : SymType(SymbolType::TypeSubrange, "subrange"), _left(left), _right(right) {}

int SymTypeSubrange::getLeft() {
    return _left;
}

int SymTypeSubrange::getRight() {
    return _right;
}

std::string SymTypeSubrange::getName() {
    std::stringstream sstream;
    sstream << "subrange " << _left << ".." << _right;
    return sstream.str();
}

SymTypeOpenArray::SymTypeOpenArray(SymbolPtr elemType) : SymType(SymbolType::TypeOpenArray, "open array"), _elemType(elemType) {}

std::string SymTypeOpenArray::getName() {
    std::stringstream sstream;
    sstream << "open array" << " of " << _elemType->getName();
    return sstream.str();
}

SymConst::SymConst(SymbolType type, std::string name) : Symbol(type, name) {}

std::string SymConst::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << getConstTypeStr();
    return sstream.str();
}

SymIntegerConst::SymIntegerConst(std::string name, int value) : SymConst(SymbolType::ConstInteger, name), _value(value) {}

int SymIntegerConst::getValue() {
    return _value;
}

std::string SymIntegerConst::toString(int depth) {
    std::stringstream sstream;
    sstream << SymConst::toString(depth) << std::setw(_thirdColumnWidth) << getValue();
    return sstream.str();
}

SymbolType SymIntegerConst::getVarType() {
    return SymbolType::TypeInteger;
}

void SymIntegerConst::generateDecl(AsmCode & asmCode) {
    asmCode.addData(asmCode.getVarName(_name), _value);
}

std::string SymIntegerConst::getConstTypeStr() {
    return "const integer ";
}

SymRealConst::SymRealConst(std::string name, double value) : SymConst(SymbolType::ConstReal, name), _value(value) {}

double SymRealConst::getValue() {
    return _value;
}

std::string SymRealConst::toString(int depth) {
    std::stringstream sstream;
    sstream << SymConst::toString(depth) << std::setw(_thirdColumnWidth) << getValue();
    return sstream.str();
}

SymbolType SymRealConst::getVarType() {
    return SymbolType::TypeReal;
}

void SymRealConst::generateDecl(AsmCode & asmCode) {
    asmCode.addData(asmCode.getVarName(_name), _value);
}

std::string SymRealConst::getConstTypeStr() {
    return "const float ";
}

SymTypePointer::SymTypePointer(SymbolPtr refType) : SymType(SymbolType::TypeRef, "pointer", 8), _refType(refType) {}

std::string SymTypePointer::getName() {
    return Symbol::getName() + " " + _refType->getName();
}

SymTypeAlias::SymTypeAlias(SymbolPtr type, std::string name) : SymType(SymbolType::TypeAlias, name), _refType(type) {}

std::string SymTypeAlias::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << "type alias " << std::setw(_thirdColumnWidth) << _refType->getName();
    if (_refType->getType() == SymbolType::TypeRecord)
        sstream << _refType->toString(depth);
    return sstream.str();
}

SymbolPtr SymTypeAlias::getRefSymbol() {
    return _refType;
}

SymbolType SymTypeAlias::getRefType() {
    return _refType->getType();
}

size_t SymTypeAlias::getSize() {
    return _refType->getSize();
}

SymFunc::SymFunc(std::string name) : SymProcBase(SymbolType::Func, name) {}

std::string SymFunc::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << "func " << std::endl;
    sstream << SymProcBase::toString(depth);
    return sstream.str();
}

SymParamBase::SymParamBase(SymbolType type, std::string name, SymbolPtr varType, SymbolPtr method, size_t offset) :
    SymVar(name, varType, type), _method(method) {}

SymParam::SymParam(std::string name, SymbolPtr varType, SymbolPtr method, size_t offset) :
    SymParamBase(SymbolType::Param, name, varType, method, offset) {}

std::string SymParam::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << "param " << std::setw(_thirdColumnWidth) << _varType->getName();
    return sstream.str();
}

void SymParam::generate(AsmCode & asmCode) {
    asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(RBP, _offset + 8));
    asmCode.addCmd(PUSH, RAX);
}

void SymParam::generateLValue(AsmCode & asmCode) {
    asmCode.addCmd(LEA, RAX, asmCode.getAdressOperand(RBP, _offset + 8));
    asmCode.addCmd(PUSH, RAX);
}

SymVarParam::SymVarParam(std::string name, SymbolPtr varType, SymbolPtr method, size_t offset) :
    SymParamBase(SymbolType::VarParam, name, varType, method, offset) {}

std::string SymVarParam::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << "var param " << std::setw(_thirdColumnWidth) << _varType->getName();
    return sstream.str();
}

size_t SymVarParam::getSize() {
    return 8;
}

void SymVarParam::generate(AsmCode & asmCode) {
    asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(RBP, _offset + 8));
    asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(RAX));
    asmCode.addCmd(PUSH, RAX);
}

void SymVarParam::generateLValue(AsmCode & asmCode) {
    asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(RBP, _offset + 8));
    asmCode.addCmd(PUSH, RAX);
}

SymFuncResult::SymFuncResult(std::string name, SymbolPtr varType, SymbolPtr method) :
    SymParamBase(SymbolType::FuncResult, name, varType, method, 0) {}

std::string SymFuncResult::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << "func result " << std::setw(_thirdColumnWidth) << _varType->getName();
    return sstream.str();
}

void SymFuncResult::generate(AsmCode & asmCode) {
    asmCode.addCmd(MOV, RAX, asmCode.getAdressOperand(RBP, std::dynamic_pointer_cast<SymProcBase>(_method)->getArgs()->getSize() + 8));
    asmCode.addCmd(PUSH, RAX);
}

void SymFuncResult::generateLValue(AsmCode & asmCode) {
    asmCode.addCmd(LEA, RAX, asmCode.getAdressOperand(RBP, std::dynamic_pointer_cast<SymProcBase>(_method)->getArgs()->getSize() + 8));
    asmCode.addCmd(PUSH, RAX);
}

SymProcBase::SymProcBase(SymbolType type, std::string name) : Symbol(type, name) {}

void SymProcBase::setArgs(SymTablePtr args) {
    _args = args;
}

void SymProcBase::setLocals(SymTablePtr locals) {
    _locals = locals;
}

std::string SymProcBase::toString(int depth) {
    std::stringstream sstream;
    sstream << std::string(_spaces * depth, ' ') << " arguments:\n" << _args->toString(depth + 1);
    if (_args->getSymbols().size() > 0)
        sstream << std::endl;
    sstream << std::string(_spaces * depth, ' ') << " locals:\n" << _locals->toString(depth + 1);
    if (_locals->getSymbols().size() > 0)
        sstream << std::endl;
    return sstream.str();
}

SymTablePtr SymProcBase::getArgs() {
    return _args;
}

SymTablePtr SymProcBase::getLocals() {
    return _locals;
}

void SymProcBase::setDepth(int depth) {
    _depth = depth;
}

int SymProcBase::getDepth() {
    return _depth;
}

void SymProcBase::generate(AsmCode & asmCode) {
    for (auto sym : _locals->getSymbols())
        sym->generate(asmCode);
}

SymProc::SymProc(std::string name) : SymProcBase(SymbolType::Proc, name) {}

std::string SymProc::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << "proc " << std::endl;
    sstream << SymProcBase::toString(depth);
    return sstream.str();
}
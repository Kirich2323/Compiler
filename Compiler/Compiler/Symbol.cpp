#include "Symbol.h"

Symbol::Symbol(SymbolType type, std::string name) : _type(type), _name(name) {}

SymbolType Symbol::getType() {
    return _type;
}

size_t Symbol::getSize() {
    return _size;
}

size_t Symbol::getOffset() {
    return _offset;;
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

SymType::SymType(SymbolType type, std::string name, int offset) : Symbol(type, name) {}

bool SymType::isType() {
    return true;
}

size_t SymType::getSize() {
    return size_t();
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

SymbolPtr SymVar::getVarTypeSymbol() {
    SymbolPtr tmp = _varType;
    while(tmp->getType() == SymbolType::TypeAlias)
        tmp = std::dynamic_pointer_cast<SymTypeAlias>(tmp)->getRefSymbol();
    return tmp;
}

void SymTable::add(SymbolPtr symb) {
    _symbolNames[symb->getName()] = _symbols.size();
    _symbols.push_back(symb);
}

bool SymTable::have(std::string name) {
    return _symbolNames.find(name) != _symbolNames.end();
}

SymbolPtr SymTable::getSymbol(std::string name) {
    //if (have(name))
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
            return SymbolPtr(new SymType(SymbolType::TypeInteger, "integer")); //todo fix
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

SymVarParam::SymVarParam(std::string name, SymbolPtr varType, SymbolPtr method, size_t offset) :
    SymParamBase(SymbolType::VarParam, name, varType, method, offset) {}

std::string SymVarParam::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << "var param " << std::setw(_thirdColumnWidth) << _varType->getName();
    return sstream.str();
}

size_t SymVarParam::getSize() {
    return size_t();
}

SymFuncResult::SymFuncResult(std::string name, SymbolPtr varType, SymbolPtr method) :
    SymParamBase(SymbolType::FuncResult, name, varType, method, 0) {}

std::string SymFuncResult::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << "func result " << std::setw(_thirdColumnWidth) << _varType->getName();
    return sstream.str();
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

SymProc::SymProc(std::string name) : SymProcBase(SymbolType::Proc, name) {}

std::string SymProc::toString(int depth) {
    std::stringstream sstream;
    sstream << Symbol::toString(depth) << std::setw(_secondColumnWidth) << "proc " << std::endl;
    sstream << SymProcBase::toString(depth);
    return sstream.str();
}
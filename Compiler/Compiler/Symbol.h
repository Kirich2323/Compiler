#pragma once
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include "Token.h"
#include "error.h"
#include "Const.h"
#include "AsmGen.h"

enum class SymbolType {
    TypeAlias,
    TypeInteger,
    TypeReal,
    TypeChar,
    TypeRecord,
    TypeArray,
    TypeOpenArray,
    TypeSubrange,
    TypeRef,
    TypeBadType,
    TypeBoolean,
    TypeString,
    VarConst,
    VarGlobal,
    VarLocal,
    ConstInteger,
    ConstReal,
    Func,
    Proc,
    FuncResult,
    Param,
    VarParam,
    None,
};

class Symbol {
public:
    Symbol(SymbolType type, std::string name, int size = 0);
    virtual SymbolType getType();
    virtual size_t getSize();
    virtual size_t getOffset();
    virtual void setOffset(size_t offset);
    virtual std::string getName();
    virtual bool isType();
    virtual std::string toString(int depth);
    virtual SymbolType getVarType();
    virtual void generate(AsmCode& asmCode);
    //virtual void generateValue(AsmCode& asmCode);
    virtual void generateLValue(AsmCode& asmCode);
    virtual void generateDecl(AsmCode& asmCode);
protected:
    size_t _size;
    size_t _offset;
    std::string _name;
    SymbolType _type;
    static const int _spaces = 4;
    static const int _firstColumnWidth = 15;
    static const int _secondColumnWidth = 15;
    static const int _thirdColumnWidth = 15;
};

typedef std::shared_ptr<Symbol> SymbolPtr;

class SymTable {
public:
    void add(SymbolPtr symb);
    bool have(std::string name);
    SymbolPtr getSymbol(std::string name);
    std::vector<SymbolPtr> getSymbols();
    std::unordered_map<std::string, int> getSymbolNames();
    void checkUnique(TokenPtr token);
    size_t getSize();
    std::string toString(int depth);
private:
    size_t _size = 0;
    std::vector<SymbolPtr> _symbols;
    std::unordered_map<std::string, int> _symbolNames;
};

typedef std::shared_ptr<SymTable> SymTablePtr;

class SymTableStack {
public:
    SymTableStack(SymTablePtr table = nullptr);
    void addTable(SymTablePtr table);
    SymTablePtr top();
    void pop();
    bool haveSymbol(std::string name);
    SymbolPtr getSymbol(TokenPtr token, bool isSymbolCheck = true);
    SymTablePtr findTableBySymbol(const std::string& symbol);
private:
    std::vector<SymTablePtr> _symTables;
};

typedef std::shared_ptr<SymTableStack> SymTableStackPtr;

class SymType : public Symbol {
public:
    SymType(SymbolType type, std::string name, int size = 0);
    bool isType();
    size_t getSize() override;
    std::string toString(int depth) override;
};

class SymTypeAlias : public SymType {
public:
    SymTypeAlias(SymbolPtr type, std::string name);
    std::string toString(int depth);
    SymbolPtr getRefSymbol();
    SymbolType getRefType();
    size_t getSize() override;
private:
    SymbolPtr _refType;

};

class SymTypeRecord : public SymType {
public:
    SymTypeRecord(SymTablePtr table);
    std::string toString(int depth);
    SymbolPtr getSymbol(std::string& name);
    SymTablePtr getTable();
    bool have(std::string& name);
    size_t getSize() override;
    //void generate(AsmCode& asmCode) override;
private:
    SymTablePtr _symTable;
};
typedef std::shared_ptr<SymTypeRecord> SymTypeRecordPtr;

class SymTypePointer : public SymType {
public:
    SymTypePointer(SymbolPtr refType);
    std::string getName() override;
private:
    SymbolPtr _refType;
};

class SymTypeSubrange : public SymType {
public:
    SymTypeSubrange(int left, int right);
    int getLeft();
    int getRight();
    std::string getName();
private:
    int _left, _right;
};
typedef std::shared_ptr<SymTypeSubrange> SymTypeSubrangePtr;

class SymTypeArray : public SymType {
public:
    SymTypeArray(SymbolPtr elemType, SymTypeSubrangePtr subrange);
    int getDimension();
    std::string getName();
    SymbolType getArrType();
    SymbolPtr getTypeSymbol();
    size_t getSize() override;
    int getLeft();
private:
    int _left, _right;
    SymbolPtr _elemType;
};

class SymTypeOpenArray : public SymType {
public:
    SymTypeOpenArray(SymbolPtr elemType);
    std::string getName();
private:
    SymbolPtr _elemType;
};

class SymVar : public Symbol {
public:
    SymVar(std::string name, SymbolPtr type, SymbolType varType, Const* init = nullptr);
    std::string toString(int depth) override;
    SymbolType getVarType() override;
    size_t getSize() override;
    SymbolPtr getVarTypeSymbol();
    void generate(AsmCode& asmCode) override;
    void generateLValue(AsmCode& asmCode) override;
    void generateMemoryCopy(AsmCode& asmCode, AsmCmdPtr cmdMemory, AsmOpType opType);
    void generateDecl(AsmCode& asmCode) override;
protected:
    SymbolPtr _varType;
    Const* _init;
};

class SymConst : public Symbol {
public:
    SymConst(SymbolType type, std::string name);
    std::string toString(int depth) override;
protected:
    virtual std::string getConstTypeStr() = 0;
};

class SymIntegerConst : public SymConst {
public:
    SymIntegerConst(std::string name, int value);
    int getValue();
    std::string toString(int depth) override;
    SymbolType getVarType() override;
    void generateDecl(AsmCode& asmCode) override;
private:
    std::string getConstTypeStr() override;
    int _value;
};

class SymRealConst : public SymConst {
public:
    SymRealConst(std::string name, double value);
    double getValue();
    std::string toString(int depth) override;
    SymbolType getVarType() override;
    void generateDecl(AsmCode& asmCode) override;
private:
    std::string getConstTypeStr() override;
    double _value;
};

class SymParamBase : public SymVar {
public:
    SymParamBase(SymbolType type, std::string name, SymbolPtr varType, SymbolPtr method, size_t offset);
protected:
    SymbolPtr _method;
};

class SymParam : public SymParamBase {
public:
    SymParam(std::string name, SymbolPtr varType, SymbolPtr method, size_t offset);
    std::string toString(int depth) override;
    void generate(AsmCode& asmCode) override;
    void generateLValue(AsmCode& asmCode) override;
};

class SymVarParam : public SymParamBase {
public:
    SymVarParam(std::string name, SymbolPtr varType, SymbolPtr method, size_t offset);
    std::string toString(int depth) override;
    size_t getSize() override;
    void generate(AsmCode& asmCode) override;
    void generateLValue(AsmCode& asmCode) override;
};

class SymFuncResult : public SymParamBase {
public:
    SymFuncResult(std::string name, SymbolPtr varType, SymbolPtr method);
    std::string toString(int depth) override;
    void generate(AsmCode& asmCode) override;
    void generateLValue(AsmCode& asmCode) override;
};

class SymProcBase : public Symbol {
public:
    SymProcBase(SymbolType type, std::string name);
    void setArgs(SymTablePtr args);
    void setLocals(SymTablePtr locals);
    std::string toString(int depth) override;
    SymTablePtr getArgs();
    SymTablePtr getLocals();
    void setDepth(int depth);
    int getDepth();
    void generate(AsmCode& asmCode) override;
protected:
    int _depth;
    SymTablePtr _args, _locals;
};
typedef std::shared_ptr<SymProcBase> SymProcBasePtr;

class SymFunc : public SymProcBase {
public:
    SymFunc(std::string name);
    std::string toString(int depth) override;
};

class SymProc : public SymProcBase {
public:
    SymProc(std::string name);
    std::string toString(int depth) override;
};
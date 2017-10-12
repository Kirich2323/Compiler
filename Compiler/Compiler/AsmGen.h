#pragma once
#include <vector>
//#include <set>
#include <map>
#include <memory>
#include <string>
#include <sstream>

enum AsmRegType {
    RAX,
    RBX,
    RCX,
    RDX,
    RBP,
    RSP,
    RSI,
    RDI,
    XMM0,
    XMM1,
    CL
};

enum AsmOpType {
    NONE,
    PUSH,
    POP,
    ADD,
    SUB,
    MUL,
    IMUL,
    IDIV,
    XOR,
    MOV,
    CALL,
    MOVQ,
    ADDSD,
    SUBSD,
    DIVSD,
    MULSD,
    CVTSI2SD,
    LABEL,
    CMP,
    JMP,
    JE,
    JNE,
    JL,
    JLE,
    JGE,
    JG,
    COMISD,
    JB,
    JBE,
    JAE,
    JA,
    AND,
    OR,
    TEST,
    JZ,
    JNZ,
    NEG,
    LEA,
    RET,
    SHL,
    SHR,
};

static std::map<AsmOpType, std::string> asmOpNames = {
    { PUSH,         "push" },
    { POP,           "pop" },
    { ADD,           "add" },
    { SUB,           "sub" },
    { MUL,           "mul" },
    { IMUL,         "imul" },
    { IDIV,         "idiv" },
    { XOR,           "xor" },
    { MOV,           "mov" },
    { CALL,         "call" },
    { MOVQ,         "movq" },
    { ADDSD,       "addsd" },
    { SUBSD,       "subsd" },
    { DIVSD,       "divsd" },
    { MULSD,       "mulsd" },
    { CVTSI2SD, "cvtsi2sd" },
    { LABEL,       "label" },
    { CMP,           "cmp" },
    { JMP,           "jmp" },
    { JE,             "je" },
    { JNE,           "jne" },
    { JL,             "jl" },
    { JLE,           "jle" },
    { JGE,           "jge" },
    { JG,             "jg" },
    { COMISD,     "comisd" },
    { JA,             "ja" },
    { JAE,           "jae" },
    { JBE,           "jbe" },
    { JB,             "jb" },
    { AND,           "and" },
    { OR,             "or" },
    { TEST,         "test" },
    { JZ,             "jz" },
    { JNZ,           "jnz" },
    { NEG,           "neg" },
    { LEA,           "lea" },
    { RET,           "ret" },
    { SHL,           "sal" },
    { SHR,           "shr" },
};

static std::map<AsmRegType, std::string> asmRegNames = {
    { RAX,   "rax" },
    { RBX,   "rbx" },
    { RCX,   "rcx" },
    { RDX,   "rdx" },
    { RBP,   "rbp" },
    { RSP,   "rsp" },
    { RSI,   "rsi" },
    { RDI,   "rdi" },
    { XMM0, "xmm0" },
    { XMM1, "xmm1" },
    { CL,     "cl" },
};

enum class AsmCmdType {
    Cmd,
    Cmd0,
    Cmd1,
    Cmd2,
    Label
};

enum class AsmOperandType {
    Operand,
    Reg,
    IntImmediate,
    StringImmediate,
    Memory
};

class AsmOperand {
public:
    //AsmOperand();
    virtual AsmOperandType getOperandType() = 0;
    virtual bool isImmediate();
    virtual std::string toString() = 0;
};
typedef std::shared_ptr<AsmOperand> AsmOperandPtr;

class AsmImmediate : public AsmOperand {
public:
    bool isImmediate() override;
};

class AsmIntImmediate : public AsmImmediate {
public:
    AsmIntImmediate(int value);
    std::string toString() override;
    AsmOperandType getOperandType() override;
    int getValue();
private:
    int _value;
};

class AsmStringImmediate : public AsmImmediate {
public:
    AsmStringImmediate(std::string value);
    std::string toString() override;
    AsmOperandType getOperandType() override;
private:
    std::string _value;
};

class AsmReg : public AsmOperand {
public:
    AsmReg(AsmRegType reg);
    std::string toString() override;
    AsmOperandType getOperandType() override;
    AsmRegType getRegType();
private:
    AsmRegType _reg;
};

class AsmMemory : public AsmOperand {
public:
    AsmMemory(std::string name, int offset = 0);
    AsmMemory(AsmRegType reg, int offset = 0);
    std::string toString() override;
    AsmOperandType getOperandType() override;
private:
    AsmOperandPtr _operand;
    int _offset;
    std::string _size;
};

class AsmCmd {
public:
    AsmCmd(AsmOpType opType, AsmOperandPtr op1 = AsmOperandPtr(), AsmOperandPtr op2 = AsmOperandPtr());
    virtual AsmOpType getOpType();
    virtual AsmCmdType getCmdType();
    AsmOperandPtr getOperand1();
    AsmOperandPtr getOperand2();
    int getOperands();
    virtual std::string toString();
private:
    AsmOpType _opType;
    AsmOperandPtr _op1, _op2;
    int _operands;
};
typedef std::shared_ptr<AsmCmd> AsmCmdPtr;

class AsmData {
public:
    AsmData(std::string name);
    virtual std::string toString() = 0;
protected:
    std::string _name;
};
typedef std::shared_ptr<AsmData> AsmDataPtr;

class AsmArrayData : public AsmData {
public:
    AsmArrayData(std::string name, int size);
    std::string toString() override;
private:
    int _size;
};

class AsmFloatData : public AsmData {
public:
    AsmFloatData(std::string name, double value);
    std::string toString() override;
private:
    double _value;
};

class AsmIntData : public AsmData {
public:
    AsmIntData(std::string name, int value);
    std::string toString() override;
private:
    int _value;
};

class AsmStringData : public AsmData {
public:
    AsmStringData(std::string name, std::string value);
    std::string toString() override;
private:
    std::string _value;
};

typedef std::shared_ptr<AsmData> AsmDataPtr;

class AsmLabel : public AsmCmd {
public:
    AsmLabel(std::string name);
    std::string toString() override;
    AsmCmdType getCmdType() override;
private:
    std::string _name;
};

class AsmCode {
public:
    AsmCode();
    void addCmd(AsmCmdPtr cmd);
    void addCmd(AsmOpType opType, AsmRegType reg);
    void addCmd(AsmOpType opType, AsmRegType reg, int val);
    void addCmd(AsmOpType opType, AsmRegType reg1, AsmRegType reg2);
    void addCmd(AsmOpType opType, std::string data);
    void addCmd(AsmOpType opType, AsmRegType reg, std::string value);
    void addCmd(AsmOpType opType, AsmRegType reg, AsmOperandPtr operand);
    void addCmd(AsmOpType opType, AsmOperandPtr operand, AsmRegType reg);
    void addCmd(AsmOpType opType);
    std::string genLabelName();
    std::string genVarName();
    std::string toString();
    std::string getVarName(std::string& name);
    void addLabel(std::string& labelName);
    void addData(std::string name, std::string value);
    void addData(std::string name, int value);
    void addData(std::string name, double value);
    void addArrayData(std::string name, int size);
    void addWriteInt();
    void addWriteFloat();
    void addWriteString(std::string str);
    void addWriteln();
    void addLoopLabels(std::string _continue, std::string _break);
    void popLoopLabels();
    std::string getContinue();
    std::string getBreak();
    AsmOperandPtr getAdressOperand(std::string name, int offset = 0);
    AsmOperandPtr getAdressOperand(AsmRegType reg, int offset = 0);
private:
    void addWrite(std::string format);
    std::vector<AsmCmdPtr> _commands;
    std::vector<AsmDataPtr> _data;
    std::vector<std::string> _breakLabels;
    std::vector<std::string> _continueLabels;
    int _labelCount;
    int _namesCount;
    int _depth;
};
#include "AsmGen.h"

AsmCode::AsmCode() : _labelCount(0), _namesCount(0), _depth(0) {
    addData("formatInt", "\"%ld\"");
    addData("formatFloat", "\"%f\"");
    addData("formatNewLine", "10");
}

void AsmCode::addCmd(AsmCmdPtr cmd) {
    _commands.push_back(cmd);
    //std::cout << cmd->toString() << std::endl; //todo delete
}

void AsmCode::addCmd(AsmOpType opType, AsmRegType reg) {
    addCmd(AsmCmdPtr(new AsmCmd(opType, AsmOperandPtr(new AsmReg(reg)))));
}

void AsmCode::addCmd(AsmOpType opType, AsmRegType reg, int val) {
    addCmd(AsmCmdPtr(new AsmCmd(opType, AsmOperandPtr(new AsmReg(reg)), AsmOperandPtr(new AsmIntImmediate(val)))));
}

void AsmCode::addCmd(AsmOpType opType, AsmRegType reg1, AsmRegType reg2) {
    addCmd(AsmCmdPtr(new AsmCmd(opType, AsmOperandPtr(new AsmReg(reg1)), AsmOperandPtr(new AsmReg(reg2)))));
}

std::string AsmCode::genLabelName() {
    return "L" + std::to_string(_labelCount++);
}

std::string AsmCode::genVarName() {
    return "v_" + std::to_string(_namesCount++);
}

std::string AsmCode::toString() {
    std::stringstream sstream;
    sstream << "global  main" << std::endl;
    sstream << "extern  printf" << std::endl;
    sstream << "section .text" << std::endl;
    //sstream << "\tmov rbp, rsp" << std::endl;
    for (auto command : _commands) {
        sstream << command->toString() << std::endl;
    }
    sstream << "\tmov rsp, rbp" << std::endl;
    sstream << "\txor rax, rax" << std::endl;
    sstream << "\tret" << std::endl;

    sstream << "section .data" << std::endl;
    for (auto asmData : _data) {
        sstream << asmData->toString() << std::endl;
    }
    return sstream.str();
}

std::string AsmCode::getVarName(std::string& name) {
    return "v_" + name;
}

void AsmCode::addLabel(std::string& labelName) {
    addCmd(AsmCmdPtr(new AsmLabel(labelName)));
}

void AsmCode::addData(std::string name, std::string value) {
    _data.push_back(AsmDataPtr(new AsmStringData(name, value)));
}

void AsmCode::addData(std::string name, int value) {
    _data.push_back(AsmDataPtr(new AsmIntData(name, value)));
}

void AsmCode::addData(std::string name, double value) {
    _data.push_back(AsmDataPtr(new AsmFloatData(name, value)));
}

void AsmCode::addArrayData(std::string name, int size) {
    _data.push_back(AsmDataPtr(new AsmArrayData(name, size)));
}

void AsmCode::addWriteInt() {
    addCmd(POP, RAX);
    addCmd(MOV, RDX, RAX);
    addCmd(XOR, RAX, RAX);
    addWrite("formatInt");
}

void AsmCode::addWriteFloat() {
    addCmd(POP, RAX);
    addCmd(MOV, RDX, RAX);
    addCmd(XOR, RAX, RAX);
    addWrite("formatFloat");
}

void AsmCode::addWriteString(std::string str) {
    std::string name = genVarName();
    addData(name, "\"" + str + "\"");
    addCmd(XOR, RAX, RAX);
    addWrite(name);
}

void AsmCode::addWriteln() {
    addCmd(XOR, RAX, RAX);
    addWrite("formatNewLine");
}

void AsmCode::addLoopLabels(std::string _continue, std::string _break) {
    _continueLabels.push_back(_continue);
    _breakLabels.push_back(_break);
}

void AsmCode::popLoopLabels() {
    _continueLabels.pop_back();
    _breakLabels.pop_back();
}

std::string AsmCode::getContinue() {
    return _continueLabels.empty() ? "" : _continueLabels.back();
}

std::string AsmCode::getBreak() {
    return _breakLabels.empty() ? "" : _breakLabels.back();
}

AsmOperandPtr AsmCode::getAdressOperand(std::string name, int offset) {
    return AsmOperandPtr(new AsmMemory(name, offset));
}

AsmOperandPtr AsmCode::getAdressOperand(AsmRegType reg, int offset) {
    return AsmOperandPtr(new AsmMemory(reg, offset));
}

void AsmCode::addWrite(std::string format) {
    addCmd(MOV, RCX, format);
    addCmd(SUB, RSP, 8 * 4);
    addCmd(CALL, "printf");
    addCmd(ADD, RSP, 8 * 4);
}

void AsmCode::addCmd(AsmOpType opType, std::string data) {
    addCmd(AsmCmdPtr(new AsmCmd(opType, AsmOperandPtr(new AsmStringImmediate(data)))));
}

void AsmCode::addCmd(AsmOpType opType, AsmRegType reg, std::string value) {
    addCmd(AsmCmdPtr(new AsmCmd(opType, AsmOperandPtr(new AsmReg(reg)), AsmOperandPtr(new AsmStringImmediate(value)))));
}

void AsmCode::addCmd(AsmOpType opType, AsmRegType reg, AsmOperandPtr operand) {
    addCmd(AsmCmdPtr(new AsmCmd(opType, AsmOperandPtr(new AsmReg(reg)), operand)));
}

void AsmCode::addCmd(AsmOpType opType, AsmOperandPtr operand, AsmRegType reg) {
    addCmd(AsmCmdPtr(new AsmCmd(opType, operand, AsmOperandPtr(new AsmReg(reg)))));
}

void AsmCode::addCmd(AsmOpType opType) {
    addCmd(AsmCmdPtr(new AsmCmd(opType)));
}

AsmCmd::AsmCmd(AsmOpType opType, AsmOperandPtr op1, AsmOperandPtr op2) : _opType(opType), _op1(op1), _op2(op2), _operands(1) {
    _operands += _op1 != nullptr;
    _operands += _op2 != nullptr;
}

AsmOpType AsmCmd::getOpType() {
    return _opType;
}

AsmCmdType AsmCmd::getCmdType() {
    return AsmCmdType::Cmd;
}

AsmOperandPtr AsmCmd::getOperand1() {
    return _op1;
}

AsmOperandPtr AsmCmd::getOperand2() {
    return _op2;
}

int AsmCmd::getOperands() {
    return _operands;
}

std::string AsmCmd::toString() {
    std::string str = "\t" + asmOpNames[_opType];
    str += _operands > 1 ? " " + _op1->toString() : "";
    str += _operands > 2 ? ", " + _op2->toString() : "";
    return str;
}

AsmLabel::AsmLabel(std::string name) : AsmCmd(AsmOpType::LABEL), _name(name) {}

std::string AsmLabel::toString() {
    return _name + ":";
}

AsmCmdType AsmLabel::getCmdType() {
    return AsmCmdType::Label;
}

bool AsmImmediate::isImmediate() {
    return true;
}

bool AsmOperand::isImmediate() {
    return false;
}

AsmIntImmediate::AsmIntImmediate(int value) : _value(value) {}

std::string AsmIntImmediate::toString() {
    return std::to_string(_value);
}

AsmOperandType AsmIntImmediate::getOperandType() {
    return AsmOperandType::IntImmediate;
}

int AsmIntImmediate::getValue() {
    return _value;
}

AsmStringImmediate::AsmStringImmediate(std::string value) : _value(value) {}

std::string AsmStringImmediate::toString() {
    return _value;
}

AsmOperandType AsmStringImmediate::getOperandType() {
    return AsmOperandType::StringImmediate;
}

AsmData::AsmData(std::string name) : _name(name) {}

AsmArrayData::AsmArrayData(std::string name, int size) : AsmData(name), _size(size) {}

std::string AsmArrayData::toString() {
    return "\t" + _name + ": times " + std::to_string(_size) + " db 0";
}

AsmFloatData::AsmFloatData(std::string name, double value) : AsmData(name), _value(value) {}

std::string AsmFloatData::toString() {
    return "\t" + _name + ": dq " + std::to_string(_value);
}

AsmIntData::AsmIntData(std::string name, int value) : AsmData(name), _value(value) {}

std::string AsmIntData::toString() {
    return "\t" + _name + ": dq " + std::to_string(_value);
}

AsmStringData::AsmStringData(std::string name, std::string value) : AsmData(name), _value(value) {}

std::string AsmStringData::toString() {
    return "\t" + _name + ": db " + _value + ", 0";
}

AsmReg::AsmReg(AsmRegType reg) : _reg(reg) {}

std::string AsmReg::toString() {
    return asmRegNames[_reg];
}

AsmOperandType AsmReg::getOperandType() {
    return AsmOperandType::Reg;
}

AsmRegType AsmReg::getRegType() {
    return _reg;
}

AsmMemory::AsmMemory(std::string name, int offset) : _offset(offset), _operand(new AsmStringImmediate(name)) {}

AsmMemory::AsmMemory(AsmRegType reg, int offset) : _offset(offset), _operand(new AsmReg(reg)) {}

std::string AsmMemory::toString() {
    std::string op = _offset > 0 ? " + " : " - ";
    std::string sizeStr = !_size.empty() ? _size + " " : "";
    return sizeStr + "[" + _operand->toString() + (!_offset ? "" : (op + std::to_string(abs(_offset)))) + "]";
}

AsmOperandType AsmMemory::getOperandType() {
    return AsmOperandType::Memory;
}

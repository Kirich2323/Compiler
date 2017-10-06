#include "error.h"

BaseException::BaseException(int line = 0, int col = 0, std::string msg = "") : _line(line), _col(col) {
    _msg = "(" + std::to_string(line) + " ; " + std::to_string(col) + "): " + msg;
}

const char* BaseException::what() const throw() {
    return _msg.c_str();
}

BadArgumentNumber::BadArgumentNumber() {
    _msg = "Bad argument number";
}

UnterminatedComment::UnterminatedComment(int line, int col) : BaseException(line, col, "Unterminated comment.") {}

UnterminatedString::UnterminatedString(int line, int col) : BaseException(line, col, "Unterminated string.") {}

InvalidInteger::InvalidInteger(int line, int col) : BaseException(line, col, "Invalid integer.") {}

InvalidReal::InvalidReal(int line, int col) : BaseException(line, col, "Invalid real.") {}

UnexpectedSymbol::UnexpectedSymbol(int line, int col) : BaseException(line, col, "Unexpected symbol.") {}

MissingFile::MissingFile(const std::string& fname) {
    _msg = "File " + fname + " not found.";
}

UnexpectedToken::UnexpectedToken(int line, int col, const std::string& got, const std::string& expected) :
    BaseException(line, col, "Got unexpected token: \"" + got + "\"" + ". Expected: \"" + expected + "\".") {}

InvalidExpression::InvalidExpression(int line, int col) : BaseException(line, col, "Invalid expression.") {}

UnexpectedEndOfFile::UnexpectedEndOfFile(int line, int col) : BaseException(line, col, "Unexpected end of file.") {}

SyntaxError::SyntaxError(int line, int col, std::string& got, std::string& expected) :
    BaseException(line, col, "Syntax error: Got: \"" + got + "\". Expected: \"" + expected + "\".") {}

Duplicate::Duplicate(int line, int col, const std::string & symbolName) : BaseException(line, col, "Duplicate: \"" + symbolName + "\".") {}

WrongSymbol::WrongSymbol(int line, int col, const std::string & name) : BaseException(line, col, "Symbol \"" + name + "\" does not exist.") {}

InvalidConstant::InvalidConstant(int line, int col, const std::string & name) : BaseException(line, col, "Invalid constant: " + name + ".") {}

BadType::BadType(int line, int col, const std::string& got, const std::string& expected) :
    BaseException(line, col, "Type error: Got \"" + got + "\". Expected: \"" + expected + "\".") {}

WrongNumberOfParam::WrongNumberOfParam(int line, int col, std::string& name) :
    BaseException(line, col, "Error: Wrong number of parameters specified for call to \"" + name + "\".") {}

NoMember::NoMember(int line, int col, std::string & member) :
    BaseException(line, col, "Error: identifier idents no member \"" + member + "\".") {}

IllegalQualifier::IllegalQualifier(int line, int col) :
    BaseException(line, col, "Error: Illegal qualifier.") {}

ProcAssignment::ProcAssignment(int line, int col) :
    BaseException(line, col, "Invalid assignment, procedures return no value.") {}

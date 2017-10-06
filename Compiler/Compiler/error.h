#pragma once

#include <exception>
#include <string>
#include <Token.h>

class BaseException {
public:
    BaseException(int, int, std::string);
    virtual const char* what() const throw();
protected:
    int _line, _col;
    std::string _msg;
};

class BadArgumentNumber : public BaseException {
public:
    BadArgumentNumber();
};

class UnterminatedComment : public BaseException {
public:
    UnterminatedComment(int, int);
};

class UnterminatedString : public BaseException {
public:
    UnterminatedString(int, int);
};

class InvalidInteger : public BaseException {
public:
    InvalidInteger(int, int);
};

class InvalidReal : public BaseException {
public:
    InvalidReal(int, int);
};

class UnexpectedSymbol : public BaseException {
public:
    UnexpectedSymbol(int, int);
};

class MissingFile : public BaseException {
public:
    MissingFile(const std::string&);
};

class UnexpectedToken : public BaseException {
public:
    UnexpectedToken(int, int, const std::string&, const std::string& expected = "");
};

class InvalidExpression : public BaseException {
public:
    InvalidExpression(int, int);
};

class UnexpectedEndOfFile : public BaseException {
public:
    UnexpectedEndOfFile(int, int);
};

class SyntaxError : public BaseException {
public:
    SyntaxError(int, int, std::string& got, std::string& expected);
};

class Duplicate : public BaseException {
public:
    Duplicate(int line, int col, const std::string& symbolName);
};

class WrongSymbol : public BaseException {
public:
    WrongSymbol(int line, int col, const std::string& name);
};

class InvalidConstant : public BaseException {
public:
    InvalidConstant(int line, int col, const std::string& name);
};

class BadType : public BaseException {
public:
    BadType(int line, int col, const std::string& got, const std::string& expected);
};

class WrongNumberOfParam : public BaseException {
public:
    WrongNumberOfParam(int line, int col, std::string& name);
};

class NoMember : public BaseException {
public:
    NoMember(int line, int col, std::string& member);
};

class IllegalQualifier : public BaseException {
public:
    IllegalQualifier(int line, int col);
};

class ProcAssignment : public BaseException {
public:
    ProcAssignment(int line, int col);
};
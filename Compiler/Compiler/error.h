#pragma once

#include <exception>
#include <string>

class BaseException {
public:
    BaseException(int, int, std::string);
    virtual const char* what() const throw();
protected:
    int _line, _col;
    std::string _msg;
};

class BadArgumentNumber : public BaseException{
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
    MissingFile(std::string);
};
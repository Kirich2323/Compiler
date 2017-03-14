#pragma once

#include <string>

enum class TokenType {
    And,
    Array,
    Begin,
    Break,
    Case,
    Const,
    Continue,
    Do,
    Downto,
    Else,
    End,
    Exit,
    False,
    For,
    Function,
    Goto,
    If,
    Label,
    Mod,
    Nil,
    Not,
    Of,
    Or,
    Procedure,
    Program,
    Record,
    Repeat,
    Set,
    Shl,
    Shr,
    Then,
    To,
    Type,
    Until,
    Var,
    While,
    Xor,

    Add,
    AddAssignment,
    Sub,
    SubAssignment,
    Mul,
    MulAssignment,
    Div,
    DivReal,
    DivAssignment,
    Assigment,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Hat,

    Comma,
    Semicolon,
    Colon,
    OpeningParenthesis,
    ClosingParenthesis,
    OpeningSquareBracket,
    ClosingSquareBracket,
    DoubleDot,
    Dot,

    Identifier,
    Word,
    IntegerNumber,
    RealNumber,
    String,
    Delimiter,
    Operation,
    EndOfFile
};

class Token {
public:
    Token(int, int, TokenType, std::string);
    ~Token() {}
    int getLine() const;
    int getCol() const;
    std::string getText() const;
    TokenType getType() const;
    virtual std::string getValue() const;
    virtual std::string getTypeString() const = 0;
private:
    std::string _token_name;
    int _line, _col;
    TokenType _type;
    std::string _text;
};

class Identifier : public Token {
public:
    Identifier(int, int, std::string);
    std::string getTypeString() const;
};

class Word : public Token {
public:
    Word(int, int, TokenType, std::string);
    std::string getTypeString() const;
};

class IntegerNumber : public Token {
public :
    IntegerNumber(int, int, std::string);
    IntegerNumber(int, int, std::string, int value);
    std::string getTypeString() const;
    std::string getValue() const;
private:
    int _value;
};

class RealNumber : public Token {
public:
    RealNumber(int, int, std::string);
    std::string getTypeString() const;
    std::string getValue() const;
private:
    double _value;
};

class String : public Token {
public:
    String(int, int, std::string, std::string);
    std::string getTypeString() const;
    std::string getValue() const;
private:
    std::string _value;
};

class Operation : public Token {
public:
    Operation(int, int, TokenType, std::string);
    std::string getTypeString() const;
};

class Delimiter : public Token {
public:
    Delimiter(int, int, TokenType, std::string);
    std::string getTypeString() const;
};

class EndOfFile : public Token {
public:
    EndOfFile(int, int);
    std::string getTypeString() const;
};
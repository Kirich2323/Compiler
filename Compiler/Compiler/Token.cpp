#include "Token.h"

Token::Token(int line, int col, TokenType type, std::string text) :
    _type(type),
    _line(line),
    _col(col),
    _text(text) {}

int Token::getLine() const {
    return _line;
}

int Token::getCol() const {
    return _col;
}

std::string Token::getText() const {
    return _text;
}

TokenType Token::getType() const {
    return _type;
}

std::string Token::getValue() const {
    return _text;
}

bool Token::operator==(TokenType type) {
    return type == _type;
}

Operation::Operation(int line, int col, TokenType type, std::string text) :
    Token(line, col, type, text) {}

std::string Operation::getTypeString() const {
    return "Operation";
}

EndOfFile::EndOfFile(int line, int col) :
    Token(line, col, TokenType::EndOfFile, "End of file") {}

std::string EndOfFile::getTypeString() const {
    return "End of file";
}

Word::Word(int line, int col, TokenType type, std::string text) :
    Token(line, col, type, text) {}

std::string Word::getTypeString() const {
    return "Word";
}

Delimiter::Delimiter(int line, int col, TokenType type, std::string text) :
    Token(line, col, type, text) {}

std::string Delimiter::getTypeString() const {
    return "Delimiter";
}

Identifier::Identifier(int line, int col, std::string text) :
    Token(line, col, TokenType::Identifier, text) {}

std::string Identifier::getTypeString() const {
    return "Identifier";
}

IntegerNumber::IntegerNumber(int line, int col, std::string text) :
    Token(line, col, TokenType::IntegerNumber, text),
    _value(std::stoi(text)) {}

IntegerNumber::IntegerNumber(int line, int col, std::string text, int value) :
    Token(line, col, TokenType::IntegerNumber, text),
    _value(value) {}

std::string IntegerNumber::getTypeString() const {
    return "Integer number";
}

std::string IntegerNumber::getValue() const {
    return std::to_string(_value);
}

RealNumber::RealNumber(int line, int col, std::string text) :
    Token(line, col, TokenType::RealNumber, text),
    _value(std::stod(text)) {}

std::string RealNumber::getTypeString() const {
    return "Real number";
}

std::string RealNumber::getValue() const {
    return std::to_string(_value);
}

String::String(int line, int col, std::string text, std::string value) :
    Token(line, col, TokenType::String, text),
    _value(value) {}

std::string String::getTypeString() const {
    return "String";
}

std::string String::getValue() const {
    return _value;
}

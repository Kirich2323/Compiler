#pragma once

#include <fstream>
#include <string>
#include <memory>
#include <set>
#include <map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "Token.h"
#include "error.h"

class Scanner {
public:
    Scanner(const char*);
    void next();
    std::string getTokenString();
    std::string getTokensString();
    std::map<TokenType, std::string> getTokenNames();
    TokenPtr getToken() const;
    TokenPtr getNextToken();
    void expect(TokenType);
    void expect(TokenPtr tok, TokenType type);
    ~Scanner();
private:
    bool readChar();
    bool isLetter();
    bool isSpace();
    bool isNumber();
    bool isDigit();
    bool isHexadecimalDigit();
    bool isOctalDigit();
    bool isBinaryDigit();
    bool isLetterOrDigit();
    bool isOperation();
    bool isOperation(const std::string&);
    bool isNewLine();
    bool isKeyWord(std::string);
    bool isDelimiter();
    bool isDelimiter(const std::string&);
    void readOperation();
    void readDelimiterOrOperation();
    void readIdentifier();
    void readString();
    void readNumber();
    void readDecimal();
    void readHexadecimal();
    void readOctal();
    void readBinary();
    void readDigits();
    void skipSingleLineComment();
    void skipMultiLineComment();
    void setToken(Token*);
    template<class T>
    void setToken();
    template<class T>
    void throwException();
    std::string getTokenName(TokenPtr tok);
    std::string getTokenName(TokenType type);    
    void setTokenNames();
    std::string _fname;
    std::ifstream _fin;
    int _line = 1;
    int _col = 0;
    int _tokenLine = 1;
    int _tokenCol = 0;
    char _char = ' ';
    bool _eof = false;
    std::string _charQueue;
    std::string _charBuffer;
    TokenPtr _token;
    std::map<std::string, TokenType> _operations;
    std::map<std::string, TokenType> _delimiters;
    std::map<std::string, TokenType> _keywords;
    std::map<TokenType, std::string> _tokenNames;
};

template<class T>
inline void Scanner::setToken() {
    setToken(new T(_tokenLine, _tokenCol, _charBuffer));
}

template<class T>
inline void Scanner::throwException() {
    throw T(_tokenLine, _tokenCol);
}

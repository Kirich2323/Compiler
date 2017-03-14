#include "Scanner.h"

Scanner::Scanner(const char* fname) :
    _line(1),
    _col(0),
    _tokenLine(1),
    _tokenCol(0),
    _char(' '),
    _fin(fname),
    _fname(fname),
    _operations({
        { "+",  TokenType::Add },
        { "+=", TokenType::AddAssignment },
        { "-",  TokenType::Sub },
        { "-=", TokenType::SubAssignment },
        { "*",  TokenType::Mul },
        { "*=", TokenType::MulAssignment },
        { "/",  TokenType::DivReal },
        { "/=", TokenType::DivAssignment },
        { ":=", TokenType::Assigment },
        { "=",  TokenType::Equal },
        { "<",  TokenType::Less },
        { "<=", TokenType::LessEqual },
        { ">",  TokenType::Greater },
        { ">=", TokenType::GreaterEqual },
        { "<>", TokenType::NotEqual },
        { "^",  TokenType::Hat },
   }),
   _delimiters({
       { ".",  TokenType::Dot },
       { "..", TokenType::DoubleDot },
       { ",",  TokenType::Comma },
       { ":",  TokenType::Colon },
       { ";",  TokenType::Semicolon },
       { "(",  TokenType::OpeningParenthesis },
       { ")",  TokenType::ClosingParenthesis },
       { "[",  TokenType::OpeningSquareBracket },
       { "]",  TokenType::ClosingSquareBracket },
   }),
   _keywords({
       { "and",       TokenType::And },
       { "array",     TokenType::Array },
       { "begin",     TokenType::Begin },
       { "break",     TokenType::Break },
       { "case",      TokenType::Case },
       { "const",     TokenType::Const },
       { "continue",  TokenType::Continue },
       { "div",       TokenType::Div },
       { "do",        TokenType::Do },
       { "downto",    TokenType::Downto },
       { "else",      TokenType::Else },
       { "end",       TokenType::End },
       { "exit",      TokenType::Exit },
       { "for",       TokenType::For },
       { "function",  TokenType::Function },
       { "goto",      TokenType::Goto },
       { "if",        TokenType::If },
       { "label",     TokenType::Label },
       { "mod",       TokenType::Mod },
       { "nil",       TokenType::Nil },
       { "not",       TokenType::Not },
       { "of",        TokenType::Of },
       { "or",        TokenType::Or },
       { "procedure", TokenType::Procedure },
       { "program",   TokenType::Program },
       { "record",    TokenType::Record },
       { "repeat",    TokenType::Repeat },
       { "set",       TokenType::Set },
       { "shl",       TokenType::Shl },
       { "shr",       TokenType::Shr },
       { "then",      TokenType::Then },
       { "to",        TokenType::To },
       { "type",      TokenType::Type },
       { "until",     TokenType::Until },
       { "var",       TokenType::Var },
       { "while",     TokenType::While },
       { "xor",       TokenType::Xor },
    })
{
    if (_fin.fail()) {
        throw MissingFile(fname);
    }
}

TokenPtr Scanner::getToken() const
{
    return _token;
}

TokenPtr Scanner::getNextToken()
{
    next();
    return _token;
}

Scanner::~Scanner()
{
    _fin.close();
}

bool Scanner::readChar()
{
    _charBuffer.push_back(_char);
    if (_charQueue.empty()) {
        _char = _fin.get();
        _eof = _fin.eof();
    }
    else {
        _char = _charQueue.front();
        _charQueue.erase(0, 1);
    }

    if (isNewLine()) {
        ++_line;
        _col = -1;
    }
    ++_col;
    return !_eof || _charQueue.size() != 0;
}

bool Scanner::isLetter()
{
    char c = tolower(_char);
    return (c >= 'a' && c <= 'z') || c == '_';
}

bool Scanner::isSpace()
{
    return _char == ' ' || _char == '\t' || _char == '\n';
}

bool Scanner::isNumber() {
    return isDigit() || _char == '&' || _char == '%' || _char == '$';
}

bool Scanner::isDigit()
{
    return _char >= '0' && _char <= '9';
}

bool Scanner::isHexadecimalDigit()
{
    return (_char >= '0' && _char <= '9') || (_char >= 'A' && _char <= 'F');
}

bool Scanner::isOctalDigit()
{
    return _char >= '0' && _char <= '7';
}

bool Scanner::isBinaryDigit()
{
    return _char == '0' || _char == '1';
}

bool Scanner::isLetterOrDigit()
{
    return isLetter() || isDigit();
}

bool Scanner::isOperation()
{
    return isOperation(std::string(1, _char));
}

bool Scanner::isOperation(const std::string & buf)
{
    return _operations.find(buf) != _operations.end();
}

bool Scanner::isNewLine()
{
    return _char == '\n';
}

bool Scanner::isKeyWord(std::string word)
{
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    return _keywords.find(word) != _keywords.end();
}

bool Scanner::isDelimiter()
{
    return isDelimiter(std::string(1, _char));
}

bool Scanner::isDelimiter(const std::string& str)
{
    return _delimiters.find(str) != _delimiters.end();
}

void Scanner::readOperation()
{
    std::string cc(1, _char);
    readChar();
    if (isOperation(cc + _char)) {
        cc += _char;
        readChar();
    }
    setToken(new Operation(_tokenLine, _tokenCol, _operations[cc], cc));
}

void Scanner::readDelimiterOrOperation()
{
    std::string c(1, _char);
    readChar();
    std::string cc = c + _char;
    if (isOperation(cc)) {
        setToken(new Operation(_tokenLine, _tokenCol, _operations[_charBuffer], cc));
        readChar();
    }
    else if (isDelimiter(cc)) {
        setToken(new Delimiter(_tokenLine, _tokenCol, _delimiters[_charBuffer], cc));
        readChar();
    }
    else {
        setToken(new Delimiter(_tokenLine, _tokenCol, _delimiters[c], c));
    }
}

void Scanner::readIdentifier()
{
    while (readChar() && isLetterOrDigit());
    if (isKeyWord(_charBuffer)) {
        setToken(new Word(_tokenLine, _tokenCol, _keywords[_charBuffer], _charBuffer));
    }
    else {
        setToken<Identifier>();
    }
}

void Scanner::readString()
{
    std::string value;
    while (_char == '\'')
    {
        while (readChar() && _char != '\'' && _char != '\n') {
            value += _char;
        }
        if (_char != '\'') {
            throwException<UnterminatedString>();
        }
        readChar();
        if (_char == '\'') {
            value += _char;
        }
        while (_char == '#') {
            std::string escape_number;
            while (readChar() && isDigit()) {
                escape_number += _char;
            }
            value += (char)std::stoi(escape_number);
        }
    }
    setToken(new String(_tokenLine, _tokenCol, _charBuffer, value));
}

void Scanner::readNumber()
{
    char c = _char;
    switch (c) {
    case '$' :
        readHexadecimal();
        break;
    case '%':
        readBinary();
        break;
    case '&':
        readOctal();
        break;
    default:
        readDecimal();
    }
    if (!isSpace() && !_eof && !isDelimiter() && !isOperation()) {
        if (_token->getType() == TokenType::IntegerNumber) {
            throwException<InvalidInteger>();
        }
        else {
            throwException<InvalidReal>();
        }        
    }
}

void Scanner::readDecimal()
{
    while (readChar() && isDigit());
    if (_char == '.') {
        if (readChar() && _char == '.') {
            _charBuffer.pop_back();
            setToken<IntegerNumber>();
            _charQueue = ".";
            --_col;
        }
        else if (isDigit() || isOperation() || _eof || isSpace() || isDelimiter()) {            
            while (isDigit() && readChar());
            if (_char == '.') {
                throwException<InvalidReal>();
            }
            setToken<RealNumber>();
        }
        else {
            throwException<InvalidReal>();
        }
    }
    else if (_char == 'e') {
        double value = std::stod(_charBuffer);
        std::string mantissaStr;
        bool isPlus = true;
        if (readChar() && (_char == '-' || _char == '+')) {
            isPlus = _char == '+';
            readChar();
        }
        if (!isDigit()) {      
            throw InvalidReal(_tokenLine, _tokenCol);
        }
        do {
            mantissaStr += _char;
        } while (readChar() && isDigit());    
        value *= std::pow(10.0, (2 * isPlus - 1) * std::stod(mantissaStr));     
        setToken<RealNumber>();
    }
    else {
        setToken<IntegerNumber>();
    }
}

void Scanner::readHexadecimal()
{
    while (readChar() && isHexadecimalDigit());
    int value = std::stoi(_charBuffer.substr(1), 0, 16);
    setToken(new IntegerNumber(_tokenLine, _tokenCol, _charBuffer, value));
}

void Scanner::readOctal()
{
    while (readChar() && isOctalDigit());
    int value = std::stoi(_charBuffer.substr(1), 0, 8);
    setToken(new IntegerNumber(_tokenLine, _tokenCol, _charBuffer, value));
}

void Scanner::readBinary()
{
    while (readChar() && isBinaryDigit());
    int value = std::stoi(_charBuffer.substr(1), 0, 2);
    setToken(new IntegerNumber(_tokenLine, _tokenCol, _charBuffer, value));
}

void Scanner::readDigits()
{
    while (readChar() && isDigit());
}

void Scanner::skipSingleLineComment()
{
    while (readChar() && !isNewLine());   
}

void Scanner::skipMultiLineComment()
{
    while (readChar() && _char != '}');
    if (_eof) {
        throwException<UnterminatedComment>();
    }
    readChar();
}

void Scanner::setToken(Token* tok)
{
    _token = TokenPtr(tok);
    _charBuffer.clear();
}

void Scanner::next()
{
    while (true) {
        while (isSpace() && readChar());         

        if (_char == '{') {
            _tokenLine = _line;
            _tokenCol = _col;
            skipMultiLineComment();
            continue;
        }

        if (_char == '/') {
            int tmp_line = _line;
            int tmp_col = _col;
            readChar();
            if (_char == '/') {
                skipSingleLineComment();
                continue;
            }
            else {                
                _charQueue.push_back(_char);
                _char = '/';     
                _line = tmp_line;
                _col = tmp_col;
            }
        }

        _tokenLine = _line;
        _tokenCol = _col;
        _charBuffer.clear();

        if (_eof && _charQueue.size() == 0) {
            setToken(new EndOfFile(_line, _col));
        }
        else if (isOperation()) {
            readOperation();            
        } 
        else if (isNumber()) {
            readNumber();           
        }
        else if (_char == '\'') {
            readString();
        }
        else if (isDelimiter()) {
            readDelimiterOrOperation();          
        }
        else if (isLetter()) {
            readIdentifier();          
        }
        else {
            throwException<UnexpectedSymbol>();
        }
        return;
    }
}

std::string Scanner::getTokenString()
{
    std::stringstream sstream;
    sstream << std::left << std::setw(4) << _token->getLine() << " "
            << std::setw(3) << _token->getCol() << " " 
            << std::setw(15) << _token->getText() << " "
            << std::setw(15) <<  _token->getValue() <<  " "
            << _token->getTypeString();

    return sstream.str();
}

std::string Scanner::getTokensString()
{
    std::stringstream sstream;
    try {
        while (getNextToken()->getType() != TokenType::EndOfFile) {
            sstream << getTokenString() << std::endl;
        }
    }
    catch (BaseException e) {
        sstream.clear();
        sstream << e.what();
    }
    return sstream.str();
}

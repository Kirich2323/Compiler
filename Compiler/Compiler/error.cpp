#include "error.h"

BaseException::BaseException(int line = 0, int col = 0, std::string msg = ""): _line(line), _col(col) {
    _msg = "(" + std::to_string(line) + " ; " + std::to_string(col) + "): " + msg;
}

const char* BaseException::what() const throw()
{
    return _msg.c_str();
}

BadArgumentNumber::BadArgumentNumber()
{
    _msg = "Bad argument number";
}

UnterminatedComment::UnterminatedComment(int line, int col) : BaseException(line, col, "Unterminated comment") {}

UnterminatedString::UnterminatedString(int line, int col) : BaseException(line, col, "Unterminated string") {}

InvalidInteger::InvalidInteger(int line, int col) : BaseException(line, col, "Invalid integer") {}

InvalidReal::InvalidReal(int line, int col) : BaseException(line, col, "Invalid real") {}

UnexpectedSymbol::UnexpectedSymbol(int line, int col) : BaseException(line, col, "Unexpected symbol") {}

MissingFile::MissingFile(const std::string& fname)
{
    _msg = "File " + fname + " not found";
}

UnexpectedToken::UnexpectedToken(int line, int col, const std::string& got) :
    BaseException(line, col, "Got unexpected token: " + got) {}
        
InvalidExpression::InvalidExpression(int line, int col) : BaseException(line, col) {}

UnexpectedEndOfFile::UnexpectedEndOfFile(int line, int col) : BaseException(line, col, "Unexpected end of file") {}

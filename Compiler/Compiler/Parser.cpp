#include "Parser.h"

Parser::Parser(const char* fname) : _scanner(fname) {
    _scanner.next();
    _priorityTable = {
        { 
            TokenType::Equal,
            TokenType::NotEqual,
            TokenType::Greater,
            TokenType::GreaterEqual,
            TokenType::Less,
            TokenType::LessEqual
        },
        { 
            TokenType::Add,
            TokenType::Sub,
            TokenType::Or,
            TokenType::Xor
        },
        { 
            TokenType::Mul,
            TokenType::Div,
            TokenType::DivReal,
            TokenType::Mod,            
            TokenType::And,
            TokenType::Shl,
            TokenType::Shr
        },
        { 
            TokenType::Not,
            TokenType::Sub,
            TokenType::Add
        }
    };
}

std::string Parser::getNodeTreeStr() {
    return parseExpr(0)->toString("", true);
}

PNode Parser::parseExpr(int priority) {
    if (priority == (int)Priority::Highest) return parseFactor();
    PNode result = parseExpr(priority + 1);
    TokenPtr t = _scanner.getToken();
    while (checkPriority(priority, t->getType())) {        
        _scanner.next();
        PNode right = parseExpr(priority + 1);
        result = PNode(new BinOpNode(t, result, right));
        t = _scanner.getToken();
    }
    return result;
}

PNode Parser::parseFactor() {
    TokenPtr t = _scanner.getToken();
    if (checkPriority((int)Priority::Highest, t->getType())) {
        _scanner.next();
        PNode node = parseExpr(0);
        return PNode(new UnaryNode(t, node));
    }
    switch (t->getType()) {
    case TokenType::EndOfFile:
        throw UnexpectedEndOfFile(t->getLine(), t->getCol());
    case TokenType::Identifier:        
        return parseIdentifier();
    case TokenType::IntegerNumber:
        _scanner.next();
        return PNode(new IntConstNode(std::stoi(t->getValue())));
    case TokenType::RealNumber:
        _scanner.next();
        return PNode(new RealNumberNode(std::stod(t->getValue())));
    case TokenType::OpeningParenthesis:
    {
        _scanner.next();
        PNode e = parseExpr(0);
        _scanner.expect(TokenType::ClosingParenthesis);
        _scanner.next();
        return e;
    }
    default:
        throw UnexpectedToken(t->getLine(), t->getCol(), t->getText());
    }
}

PNode Parser::parseIdentifier(bool isRecursive) {
    TokenPtr t = _scanner.getToken();
    PNode result = PNode(new IdentifierNode(t->getValue()));    
    while (isRecursive) {
        t = _scanner.getNextToken();       
        if (t->getType() == TokenType::Dot) {
            t = _scanner.getNextToken();
            _scanner.expect(TokenType::Identifier);
            PNode right = parseIdentifier(false);
            result = PNode(new RecordAccessNode(result, right));
        }
        else if (t->getType() == TokenType::OpeningSquareBracket) {          
            std::vector<PNode> args = parseCommaSeparated();
            result = PNode(new ArrayIndexNode(result, args));
            t = _scanner.getToken();
            _scanner.expect(TokenType::ClosingSquareBracket);
        }
        else {
            isRecursive = false;
        }
    }
    return result;
}

std::vector<PNode> Parser::parseCommaSeparated() {
    std::vector<PNode> nodes;
    TokenPtr t;
    do {        
        _scanner.next();
        nodes.push_back(parseExpr(0));
        t = _scanner.getToken();
    } while (t->getType() == TokenType::Comma);
    return nodes;
}

bool Parser::checkPriority(int priority, TokenType tt) {
    return _priorityTable[priority].find(tt) != _priorityTable[priority].end();
}
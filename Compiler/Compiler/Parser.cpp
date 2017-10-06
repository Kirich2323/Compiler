#include "Parser.h"

Parser::Parser(const char* fname, bool isSymbolCheck) :
    _progName("main"),
    _scanner(fname),
    _symTables(SymTableStackPtr(new SymTableStack(SymTablePtr(new SymTable())))),
    _isSymbolCheck(isSymbolCheck) {

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

    _computableUnOps[TokenType::Sub] = &Parser::ComputeUnarySubtraction;
    _computableUnOps[TokenType::Not] = &Parser::ComputeUnaryNeagtion;
    _computableUnOps[TokenType::Add] = &Parser::ComputeUnaryAddition;

    _computableBinOps[TokenType::Sub] = &Parser::ComputeBinarySubtraction;
    _computableBinOps[TokenType::Add] = &Parser::ComputeBinaryAddition;
    _computableBinOps[TokenType::Mul] = &Parser::ComputeBinaryMultiplication;
    _computableBinOps[TokenType::Div] = &Parser::ComputeBinaryDiv;
    _computableBinOps[TokenType::DivReal] = &Parser::ComputeBinaryDivision;
    _computableBinOps[TokenType::Equal] = &Parser::ComputeBinaryEq;
    _computableBinOps[TokenType::NotEqual] = &Parser::ComputeBinaryNe;
    _computableBinOps[TokenType::Greater] = &Parser::ComputeBinaryGt;
    _computableBinOps[TokenType::Less] = &Parser::ComputeBinaryLt;
    _computableBinOps[TokenType::GreaterEqual] = &Parser::ComputeBinaryGe;
    _computableBinOps[TokenType::LessEqual] = &Parser::ComputeBinaryLe;
    _computableBinOps[TokenType::And] = &Parser::ComputeBinaryAnd;
    _computableBinOps[TokenType::Or] = &Parser::ComputeBinaryOr;
    _computableBinOps[TokenType::Xor] = &Parser::ComputeBinaryXor;

    _symTables->top()->add(SymbolPtr(new SymType(SymbolType::TypeChar, "char")));
    _symTables->top()->add(SymbolPtr(new SymType(SymbolType::TypeReal, "float")));
    _symTables->top()->add(SymbolPtr(new SymType(SymbolType::TypeInteger, "integer")));

    _typeChecker = TypeChecker(SymTableStackPtr(_symTables));
}

std::string Parser::getNodeTreeStr() {
    return parseExpr(0)->toString("", true);
}

std::string Parser::getDeclStr() {
    parseDeclaration(0, true);
    return _progName + "\n----------\n" + _symTables->top()->toString(0);
}

std::string Parser::getProgStr() {
    return std::string();
}

std::string Parser::getStmtStr() {
    return parse()->toString("", true);
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
        PNode node = parseExpr((int)Priority::Highest);
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
            return PNode(new RealConstNode(std::stod(t->getValue())));
        case TokenType::OpeningParenthesis:
        {
            _scanner.next();
            PNode e = parseExpr(0);
            _scanner.expect(TokenType::ClosingParenthesis);
            _scanner.next();
            return e;
        }
        default:
            throw InvalidExpression(t->getLine(), t->getCol());
    }
}

PNode Parser::parseIdentifier(bool isRecursive) {
    TokenPtr t = _scanner.getToken();
    TokenPtr indentToken = t;
    SymbolPtr sym = _symTables->getSymbol(t, _isSymbolCheck);
    PNode result(new IdentifierNode(t->getValue(), sym));
    while (isRecursive) {
        t = _scanner.getNextToken();
        switch (t->getType()) {
            case TokenType::Dot:
            {
                t = _scanner.getNextToken();
                _scanner.expect(TokenType::Identifier);
                if (_isSymbolCheck) {
                    if (sym->getVarType() != SymbolType::TypeRecord)
                        throw IllegalQualifier(t->getLine(), t->getCol());
                    _symTables->addTable(std::dynamic_pointer_cast<SymTypeRecord>(std::dynamic_pointer_cast<SymVar>(sym)->getVarTypeSymbol())->getTable());
                    PNode right = parseIdentifier(false);
                    _symTables->pop();
                    SymbolPtr attr = std::dynamic_pointer_cast<SymVar>(sym)->getVarTypeSymbol();
                    SymTypeRecordPtr rec = std::dynamic_pointer_cast<SymTypeRecord>(attr);
                    if (!rec->have(t->getText()))
                        throw NoMember(t->getLine(), t->getCol(), t->getText());
                    sym = rec->getSymbol(t->getText());
                    result = PNode(new RecordAccessNode(result, right, attr));
                }
                else
                    result = PNode(new RecordAccessNode(result, parseIdentifier(false)));
                break;
            }
            case TokenType::OpeningSquareBracket:
            {
                std::vector<PNode> args = parseCommaSeparated();
                if (_isSymbolCheck)
                    if (sym->getVarType() != SymbolType::TypeArray ||
                        args.size() != std::dynamic_pointer_cast<SymTypeArray>(std::dynamic_pointer_cast<SymVar>(sym)->getVarTypeSymbol())->getDimension())
                        throw IllegalQualifier(t->getLine(), t->getCol());
                result = PNode(new ArrayIndexNode(result, args, sym));
                _scanner.expect(TokenType::ClosingSquareBracket);
                break;
            }
            case TokenType::OpeningParenthesis:
            {
                if (sym->getType() != SymbolType::Func && sym->getType() != SymbolType::Proc) {
                    isRecursive = false;
                    break;
                }                
                _scanner.next();
                std::vector<PNode> args = getArgsArray(TokenType::ClosingParenthesis);
                std::vector<SymbolPtr> procArgs = std::dynamic_pointer_cast<SymProcBase>(sym)->getArgs()->getSymbols();
                if (args.size() != procArgs.size() - (sym->getType() == SymbolType::Func) ? 1 : 0)
                    throw WrongNumberOfParam(indentToken->getLine(), indentToken->getCol(), indentToken->getText());
                for (int i = 0; i < args.size(); ++i)
                    expectType(procArgs[i]->getVarType(), args[i], t);
                result = PNode(new CallNode(result, args, sym));
                break;
            }
            default:
                isRecursive = false;
        }
    }
    return result;
}

PNode Parser::parseStatement() {
    TokenType tt = getToken()->getType();
    PNode statement(nullptr);
    switch (tt) {
        case TokenType::Identifier: statement = parseIdentifierStatement(); break;
        case TokenType::Begin: statement = parseCompoundStatement("inner block"); break;
        case TokenType::While: statement = parseWhileStatement(); break;
        case TokenType::If: statement = parseIfStatement(); break;
        case TokenType::For: statement = parseForStatement(); break;
        case TokenType::Repeat: statement = parseRepeatStatement(); break;
        case TokenType::Semicolon: statement = PNode(new EmptyNode()); break;
        case TokenType::End: statement = PNode(new EmptyNode()); break;
        default: statement = parseExpr(0);
    }
    return statement;
}

PNode Parser::parseCompoundStatement(std::string name) {
    BlockNode* block = new BlockNode(name);
    parseStatementSequence(block);
    if (getToken()->getType() != TokenType::End) {
        _scanner.expect(TokenType::Semicolon);
        _scanner.next();
    }
    _scanner.expect(TokenType::End);
    _scanner.next();
    return PNode(block);
}

void Parser::parseStatementSequence(BlockNode* block) {
    do {
        _scanner.next();
        if (getToken()->getType() != TokenType::End)
            block->addStatement(parseStatement());
    } while (getToken()->getType() == TokenType::Semicolon);
}

PNode Parser::parseIfStatement() {
    TokenPtr tok = getNextToken();
    PNode cond = parseExpr(0);
    SymbolType condType = _typeChecker.getExprType(cond);
    if (!_typeChecker.equalTypes(SymbolType::TypeBoolean, condType))
        throw BadType(tok->getLine(), tok->getCol(), _typeChecker.typeNames[condType], _typeChecker.typeNames[SymbolType::TypeBoolean]);
    _scanner.expect(TokenType::Then);
    _scanner.next();
    PNode then = parseStatement();
    PNode els = nullptr;
    if (getToken()->getType() == TokenType::Else) {
        _scanner.next();
        els = parseStatement();
    }
    return PNode(new IfNode(cond, then, els));
}

PNode Parser::parseWhileStatement() {
    TokenPtr tok = getNextToken();
    PNode cond = parseExpr(0);
    expectType(SymbolType::TypeBoolean, cond, tok);
    _scanner.expect(TokenType::Do);
    _scanner.next();
    PNode block = parseStatement();
    return PNode(new WhileNode(cond, block));
}

PNode Parser::parseForStatement() {
    TokenPtr tok = getNextToken();
    _scanner.expect(TokenType::Identifier);
    PNode var = parseIdentifier();
    expectType(SymbolType::TypeInteger, var, tok);
    SymbolPtr varSym = _symTables->getSymbol(tok);
    _scanner.expect(TokenType::Assigment);
    tok = getNextToken();
    PNode initial_exp = parseExpr(0);
    TokenType type = getToken()->getType();
    expectType(SymbolType::TypeInteger, initial_exp, tok);
    if (type == TokenType::To || type == TokenType::Downto) {
        bool isTo = type == TokenType::To;
        tok = getNextToken();
        PNode final_exp = parseExpr(0);
        expectType(SymbolType::TypeInteger, final_exp, tok);
        _scanner.expect(TokenType::Do);
        _scanner.next();
        PNode stmt = parseStatement();
        return PNode(new ForNode(varSym, initial_exp, final_exp, stmt, isTo));
    }
    else {
        TokenPtr t = getToken();
        throw SyntaxError(t->getLine(), t->getCol(), _scanner.getTokenNames()[t->getType()], _scanner.getTokenNames()[TokenType::To]);
    }
}

PNode Parser::parseRepeatStatement() {
    _scanner.next();
    PNode body = parseStatement();
    if (getToken()->getType() == TokenType::Semicolon)
        _scanner.next();
    _scanner.expect(TokenType::Until);
    TokenPtr tok = getNextToken();
    PNode cond = parseExpr(0);
    expectType(SymbolType::TypeBoolean, cond, tok);
    return PNode(new RepeatNode(cond, body));
}

PNode Parser::parseCall(PNode expr) {
    TokenPtr tok = getToken();
    _scanner.next();
    if (tok->getType() == TokenType::OpeningParenthesis) {
        _scanner.next();
        std::vector<PNode> args = parseCommaSeparated();
        expr = PNode(new CallNode(expr, args, _symTables->getSymbol(tok)));
    }
    return expr;
}

PNode Parser::parseIdentifierStatement() {
    PNode expr = parseExpr(0);
    TokenPtr tok = getToken();
    if (tok->getType() == TokenType::Assigment) {
        TokenPtr tmp = getNextToken();
        PNode right = parseExpr(0);
        if (right->getNodeType() == SynNodeType::Call)
            if (std::dynamic_pointer_cast<CallNode>(right)->getSymbol()->getType() == SymbolType::Proc)
                throw ProcAssignment(tmp->getLine(), tmp->getCol());
        expectType(_typeChecker.getExprType(expr), right, tmp);
        return PNode(new AssignmentNode(tok, expr, right));
    }
    else if (expr->getNodeType() != SynNodeType::Call) {
        throw InvalidExpression(tok->getLine(), tok->getCol());
    }
    return expr;
}

std::vector<PNode> Parser::getArgsArray(TokenType terminatingType) {
    std::vector<PNode> indices;
    if (getToken()->getType() != terminatingType)
        indices.push_back(parseExpr(0));
    while (getToken()->getType() != terminatingType) {
        _scanner.expect(TokenType::Comma);
        TokenPtr token = getToken();
        if (getNextToken()->getType() == terminatingType)
            throw InvalidExpression(getToken()->getLine(), getToken()->getCol());
        indices.push_back(parseExpr(0));
    }
    _scanner.next();
    return indices;
}

SymbolPtr Parser::parseRecord() {
    _symTables->addTable(SymTablePtr(new SymTable()));
    if (getToken()->getType() == TokenType::Identifier)
        parseVarDeclaration(true);
    SymbolPtr record = SymbolPtr(new SymTypeRecord(_symTables->top()));
    _symTables->pop();
    _scanner.expect(TokenType::End);
    _scanner.next();
    return record;
}

SymbolPtr Parser::parseArray() {
    TokenPtr token = getToken();
    if (token->getType() == TokenType::OpeningSquareBracket) {
        _scanner.next();
        std::vector<SymbolPtr> subranges = std::vector<SymbolPtr>(1, parseSubrange());
        while (getToken()->getType() == TokenType::Comma) {
            _scanner.next();
            subranges.push_back(parseSubrange());
        }
        _scanner.expect(TokenType::ClosingSquareBracket);
        _scanner.next();
        _scanner.expect(TokenType::Of);
        _scanner.next();
        SymbolPtr type = parseType();
        SymbolPtr result = SymbolPtr(new SymTypeArray(type, std::dynamic_pointer_cast<SymTypeSubrange>(subranges.back())));
        for (int i = subranges.size() - 2; i >= -0; --i)
            result = SymbolPtr(new SymTypeArray(result, std::dynamic_pointer_cast<SymTypeSubrange>(subranges[i])));
        return result;
    }
    else {
        _scanner.expect(TokenType::Of);
        _scanner.next();
        return SymbolPtr(new SymTypeOpenArray(parseType()));
    }
}

SymbolPtr Parser::parseSubrange() {
    TokenPtr token = getToken();
    if (token->getType() == TokenType::Identifier) {
        if (_symTables->haveSymbol(token->getText())) {
            SymbolPtr symbol = _symTables->getSymbol(token, _isSymbolCheck);
            if (symbol->getType() == SymbolType::TypeAlias &&
                std::dynamic_pointer_cast<SymTypeAlias>(symbol)->getRefType() == SymbolType::TypeSubrange) {
                _scanner.next();
                return std::dynamic_pointer_cast<SymTypeAlias>(symbol)->getRefSymbol();
            }
        }
    }
    SymbolPtr left = parseConst();
    checkSymbolType(left, SymbolType::ConstInteger, token);
    _scanner.expect(TokenType::DoubleDot);
    token = getNextToken();
    SymbolPtr right = parseConst();
    checkSymbolType(right, SymbolType::ConstInteger, token);
    int left_v = std::dynamic_pointer_cast<SymIntegerConst>(left)->getValue();
    int right_v = std::dynamic_pointer_cast<SymIntegerConst>(right)->getValue();
    return SymbolPtr(new SymTypeSubrange(left_v, right_v));
}

SymbolPtr Parser::parsePointer() {
    _scanner.expect(TokenType::Identifier);
    TokenPtr token = getNextToken();
    SymbolPtr refType = _symTables->getSymbol(token, _isSymbolCheck);
    return SymbolPtr(new SymTypePointer(refType));
}

PNode Parser::parse() {
    parseDeclaration(0, true);
    _scanner.expect(TokenType::Begin);
    PNode stmt = parseCompoundStatement("main block");
    _scanner.expect(TokenType::Dot);
    return stmt;
}

void Parser::parseDeclaration(int depth, bool isGlobal) {
    if (getToken()->getType() == TokenType::Program) {
        _scanner.next();
        _scanner.expect(TokenType::Identifier);
        _progName = getToken()->getText();
        _scanner.next();
        _scanner.expect(TokenType::Semicolon);
    }
    bool isDeclaration = true;
    while (isDeclaration) {
        TokenPtr token = getToken();
        switch (token->getType()) {
            case TokenType::Var:
                _scanner.next();
                parseVarDeclaration(true);
                break;
            case TokenType::Const:
                _scanner.next();
                parseConstDeclaration();
                break;
            case TokenType::Function:
                _scanner.next();
                parseFuncDeclaration(depth);
                break;
            case TokenType::Procedure:
                _scanner.next();
                parseProcDeclaration(depth);
                break;
            case TokenType::Type:
                _scanner.next();
                parseTypeDeclaration();
                break;
            default:
                isDeclaration = false;
        }
    }
}

void Parser::parseVarDeclaration(bool isGlobal) {
    TokenPtr token = getToken();
    _scanner.expect(TokenType::Identifier);
    while (token->getType() == TokenType::Identifier) {
        std::vector<TokenPtr> identifiers(1, token);
        _symTables->top()->checkUnique(identifiers[0]);
        _scanner.next();
        while (getToken()->getType() == TokenType::Comma) {
            token = getNextToken();
            _symTables->top()->checkUnique(token);
            _scanner.expect(TokenType::Identifier);
            identifiers.push_back(token);
            _scanner.next();
        }
        _scanner.expect(TokenType::Colon);
        _scanner.next();
        SymbolPtr type(parseType());
        Const* value = nullptr;
        if (getToken()->getType() == TokenType::Equal) {
            if (identifiers.size() > 1)
                throw BaseException(getToken()->getLine(), getToken()->getCol(), "Error: Only one variable can be initialized.");
            TokenPtr tok = getNextToken();
            PNode expr = parseExpr(0);
            value = new Const(ComputeConstantExpression(expr));
            if (!_typeChecker.checkExprType(type->getType(), expr))
                throw BadType(tok->getLine(), tok->getCol(), _typeChecker.typeNames[expr->getType()], type->getName());
        }
        _scanner.expect(TokenType::Semicolon);
        token = getNextToken();
        if (isGlobal)
            for (auto it : identifiers)
                _symTables->top()->add(SymbolPtr(new SymVar(it->getText(), type, SymbolType::VarGlobal, value)));
        else
            for (auto it : identifiers)
                _symTables->top()->add(SymbolPtr(new SymVar(it->getText(), type, SymbolType::VarLocal, value)));
    }
}

void Parser::parseConstDeclaration() {
    _scanner.expect(TokenType::Identifier);
    for (TokenPtr tok = getToken(); tok->getType() == TokenType::Identifier; tok = getNextToken()) {
        std::string identifier = tok->getText();
        _symTables->top()->checkUnique(tok);
        _scanner.next();
        SymbolPtr type;
        if (getToken()->getType() == TokenType::Colon) {
            _scanner.next();
            type = parseType();
        }
        _scanner.expect(TokenType::Equal);
        _scanner.next();
        PNode expr = parseExpr(0);
        Const value = ComputeConstantExpression(expr);
        SymbolPtr constSymbol;
        if (type != nullptr) {
            if (_typeChecker.checkExprType(type->getType(), expr)) {
                if (type->getType() == SymbolType::TypeInteger)
                    constSymbol = SymbolPtr(new SymIntegerConst(identifier, (int)(value.value)));
                else if (type->getType() == SymbolType::TypeReal)
                    constSymbol = SymbolPtr(new SymRealConst(identifier, value.value));
            }
            else
                throw BadType(getToken()->getLine(), getToken()->getCol(), _typeChecker.typeNames[expr->getType()], type->getName());
        }
        else {
            if (value.type == exprType::Integer)
                constSymbol = SymbolPtr(new SymIntegerConst(identifier, (int)(value.value)));
            else if (value.type == exprType::Real)
                constSymbol = SymbolPtr(new SymRealConst(identifier, value.value));
        }
        _symTables->top()->add(constSymbol);
        _scanner.expect(TokenType::Semicolon);
    }
}

SymbolPtr Parser::parseConst(std::string identifier) {

    PNode expr = parseExpr(0);
    Const value = ComputeConstantExpression(expr);
    if (value.type == exprType::Integer) {
        return SymbolPtr(new SymIntegerConst(identifier, (int)(value.value)));
    }
    else {
        return SymbolPtr(new SymRealConst(identifier, value.value));
    }
}

void Parser::parseTypeDeclaration() {
    _scanner.expect(TokenType::Identifier);
    for (TokenPtr tok = getToken(); tok->getType() == TokenType::Identifier; tok = getNextToken()) {
        _symTables->top()->checkUnique(tok);
        std::string identifier = tok->getText();
        _scanner.next();
        _scanner.expect(TokenType::Equal);
        _scanner.next();
        SymbolPtr type = parseType();
        if (type->getType() == SymbolType::TypeAlias)
            type = std::dynamic_pointer_cast<SymTypeAlias>(type)->getRefSymbol();
        _symTables->top()->add(SymbolPtr(new SymTypeAlias(type, identifier)));
        _scanner.expect(TokenType::Semicolon);
    }
}

void Parser::parseFuncDeclaration(int depth) {
    _symTables->top()->checkUnique(getToken());
    std::string name = getToken()->getText();
    _scanner.next();
    SymbolPtr func(new SymFunc(name));
    _symTables->top()->add(func);
    _symTables->addTable(SymTablePtr(new SymTable()));
    parseParams(func);
    _scanner.expect(TokenType::Colon);
    _scanner.next();
    _symTables->top()->add(SymbolPtr(new SymFuncResult("result", parseType(), func)));
    _scanner.expect(TokenType::Semicolon);
    std::dynamic_pointer_cast<SymProcBase>(func)->setArgs(_symTables->top());
    _scanner.next();
    std::dynamic_pointer_cast<SymProcBase>(func)->setLocals(parseProcBody(func, depth + 1));
    std::dynamic_pointer_cast<SymProcBase>(func)->setDepth(depth);
    _symTables->pop();
}

void Parser::parseProcDeclaration(int depth) {
    _scanner.expect(TokenType::Identifier);
    _symTables->top()->checkUnique(getToken());
    std::string name = getToken()->getText();
    _scanner.next();
    SymbolPtr proc(new SymProc(name));
    _symTables->top()->add(proc);
    _symTables->addTable(SymTablePtr(new SymTable()));
    std::dynamic_pointer_cast<SymProcBase>(proc)->setArgs(parseParams(proc));
    _scanner.expect(TokenType::Semicolon);
    _scanner.next();
    std::dynamic_pointer_cast<SymProc>(proc)->setLocals(parseProcBody(proc, depth + 1));
    std::dynamic_pointer_cast<SymProc>(proc)->setDepth(depth);
    _symTables->pop();
}

SymbolPtr Parser::parseType() {
    TokenPtr token = getToken();
    SymbolPtr type;
    switch (token->getType()) {
        case TokenType::Identifier:
        {
            SymbolPtr type = _symTables->getSymbol(token, _isSymbolCheck);
            if (type->isType()) {
                _scanner.next();
                return type;
            }
        }
        default:
            return parseSubrange();
        case TokenType::Record:
            _scanner.next();
            return parseRecord();
        case TokenType::Array:
            _scanner.next();
            return parseArray();
        case TokenType::Hat:
            _scanner.next();
            return parsePointer();
    }
}

SymTablePtr Parser::parseParams(SymbolPtr proc) {
    _scanner.expect(TokenType::OpeningParenthesis);
    int offset = 0;
    for (TokenPtr token = getNextToken();
         token->getType() == TokenType::Identifier || token->getType() == TokenType::Var;) {

        bool isVar = token->getType() == TokenType::Var;
        std::vector<TokenPtr> identifiers;
        if (isVar)
            _scanner.next();
        identifiers = parseIdentifiers();
        _scanner.expect(TokenType::Colon);
        _scanner.next();
        SymbolPtr type = parseType();
        for (auto identifier : identifiers) {
            _symTables->top()->checkUnique(identifier);
            SymbolPtr symbol;
            if (isVar)
                symbol = SymbolPtr(new SymVarParam(identifier->getText(), type, proc, 0));
            else
                symbol = SymbolPtr(new SymParam(identifier->getText(), type, proc, 0));
            _symTables->top()->add(symbol);
        }
        offset += identifiers.size() * type->getSize();
        if (getToken()->getType() != TokenType::Semicolon)
            break;
        else {
            token = getNextToken();
            if (token->getType() != TokenType::Var)
                _scanner.expect(TokenType::Identifier);
        }
    }
    _scanner.expect(TokenType::ClosingParenthesis);
    size_t size = _symTables->top()->getSize();
    for (auto symbol : _symTables->top()->getSymbols())
        symbol->setOffset(size - symbol->getOffset());
    _scanner.next();
    return _symTables->top();
}

std::vector<TokenPtr> Parser::parseIdentifiers() {
    _scanner.expect(TokenType::Identifier);
    TokenPtr token = getToken();
    std::vector<TokenPtr> identifiers(1, token);
    while (getNextToken()->getType() == TokenType::Comma) {
        token = getNextToken();
        _scanner.expect(TokenType::Identifier);
        identifiers.push_back(token);
    }
    return identifiers;
}

SymTablePtr Parser::parseProcBody(SymbolPtr proc, int depth) {
    _symTables->addTable(SymTablePtr(new SymTable()));
    parseDeclaration(depth, false);
    _scanner.expect(TokenType::Begin);
    _procedureBodies[proc] = parseCompoundStatement("proc body");
    _scanner.expect(TokenType::Semicolon);
    _scanner.next();
    SymTablePtr locals = _symTables->top();
    _symTables->pop();
    return locals;
}

double Parser::getFrac(double value) {
    return value - (value >= 0 ? floor(value) : ceil(value));
}

void Parser::expectType(SymbolType type, PNode expr, TokenPtr tok) {
    SymbolType varType = _typeChecker.getExprType(expr);
    if (!_typeChecker.equalTypes(type, varType))
        throw BadType(tok->getLine(), tok->getCol(), _typeChecker.typeNames[varType], _typeChecker.typeNames[type]);
}

Const Parser::ComputeUnaryAddition(Const arg) {
    return arg;
}

Const Parser::ComputeUnarySubtraction(Const arg) {
    return -arg;
}

Const Parser::ComputeUnaryNeagtion(Const arg) {
    return !arg;
}

Const Parser::ComputeBinaryMultiplication(Const left, Const right) {
    return left * right;
}

Const Parser::ComputeBinaryDivision(Const left, Const right) {
    return left / right;
}

Const Parser::ComputeBinaryDiv(Const left, Const right) {
    return left / right;
}

Const Parser::ComputeBinaryMod(Const left, Const right) {
    return left % right;
}

Const Parser::ComputeBinaryAnd(Const left, Const right) {
    return left & right;
}

Const Parser::ComputeBinaryShl(Const left, Const right) {
    return left << right;
}

Const Parser::ComputeBinaryShr(Const left, Const right) {
    return left >> right;
}

Const Parser::ComputeBinaryAddition(Const left, Const right) {
    return left + right;
}

Const Parser::ComputeBinarySubtraction(Const left, Const right) {
    return left - right;
}

Const Parser::ComputeBinaryOr(Const left, Const right) {
    return left | right;
}

Const Parser::ComputeBinaryXor(Const left, Const right) {
    return left ^ right;
}

Const Parser::ComputeBinaryEq(Const left, Const right) {
    return left == right;
}

Const Parser::ComputeBinaryNe(Const left, Const right) {
    return left != right;
}

Const Parser::ComputeBinaryLt(Const left, Const right) {
    return left < right;
}

Const Parser::ComputeBinaryGt(Const left, Const right) {
    return left > right;
}

Const Parser::ComputeBinaryLe(Const left, Const right) {
    return left <= right;
}

Const Parser::ComputeBinaryGe(Const left, Const right) {
    return left >= right;
}

exprType castTypes(exprType left, exprType right) {
    if (left == right)
        return left;
    else {
        return exprType::Real;
    }
}

Const Parser::ComputeConstantExpression(PNode node) {
    if (*node == SynNodeType::RecordAcces || *node == SynNodeType::ArrayIndex)
        throw InvalidExpression(getToken()->getLine(), getToken()->getCol());

    if (*node == SynNodeType::UnaryOp) {
        auto it = _computableUnOps.find(std::dynamic_pointer_cast<OpNode>(node)->getOpType());
        if (it == _computableUnOps.end())
            throw InvalidExpression(getToken()->getLine(), getToken()->getCol());
        return  (this->*(it->second))(ComputeConstantExpression(std::dynamic_pointer_cast<UnaryNode>(node)->getArg()));
    }
    else if (*node == SynNodeType::BinaryOp) {
        auto it = _computableBinOps.find(std::dynamic_pointer_cast<OpNode>(node)->getOpType());
        if (it == _computableBinOps.end())
            throw InvalidExpression(getToken()->getLine(), getToken()->getCol());
        return (this->*(it->second))(ComputeConstantExpression(std::dynamic_pointer_cast<BinOpNode>(node)->getLeft()),
                                     ComputeConstantExpression(std::dynamic_pointer_cast<BinOpNode>(node)->getRight()));
    }
    else if (*node == SynNodeType::RealNumber) {
        return Const(exprType::Real, std::dynamic_pointer_cast<RealConstNode>(node)->getValue());
    }
    else if (*node == SynNodeType::IntegerNumber) {
        return Const(exprType::Integer, std::dynamic_pointer_cast<IntConstNode>(node)->getValue());
    }
    else if (*node == SynNodeType::Identifier) {
        SymbolPtr symb = _symTables->top()->getSymbol(std::dynamic_pointer_cast<IdentifierNode>(node)->getName());
        switch (symb->getType()) {
            case SymbolType::ConstInteger:
                return Const(exprType::Integer, std::dynamic_pointer_cast<SymIntegerConst>(symb)->getValue());
            case SymbolType::ConstReal:
                return Const(exprType::Real, std::dynamic_pointer_cast<SymRealConst>(symb)->getValue());
            default:
                throw InvalidExpression(getToken()->getLine(), getToken()->getCol());
        }
    }
}


PNode Parser::parseJumpStatement() {
    return PNode();
}

TokenPtr Parser::getToken() {
    return _scanner.getToken();
}

TokenPtr Parser::getNextToken() {
    return _scanner.getNextToken();
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

void Parser::setSymbolCheck(bool isCheck) {
    _isSymbolCheck = isCheck;
}

bool Parser::checkPriority(int priority, TokenType tt) {
    return _priorityTable[priority].find(tt) != _priorityTable[priority].end();
}

bool Parser::checkSymbolType(SymbolPtr symbol, SymbolType expectedType, TokenPtr token) {
    if (symbol->getType() != expectedType) {
        throw "Expected error";
    }
    return true;
}

Const::Const(exprType type, double value) : type(type), value(value) {}

Const Const::operator==(Const right) {
    return Const(exprType::Integer, value == right.value);
}

Const Const::operator<(Const right) {
    return Const(exprType::Integer, value < right.value);
}

Const Const::operator>(Const right) {
    if (type != right.type && type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)((i64)value > (i64)right.value));
}

Const Const::operator<=(Const right) {
    if (type != right.type && type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)((i64)value <= (i64)right.value));
}

Const Const::operator>=(Const right) {
    if (type != right.type && type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)((i64)value >= (i64)right.value));
}

Const Const::operator!=(Const right) {
    if (type != right.type && type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)((i64)value != (i64)right.value));
}

Const Const::operator!() {
    if (type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)(!((i64)value)));
}

Const Const::operator%(Const right) {
    if (type != right.type && type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)((i64)value % (i64)right.value));
}

Const Const::operator+(Const right) {
    return Const(castTypes(type, right.type), value + right.value);
}

Const Const::operator-(Const right) {
    return Const(castTypes(type, right.type), value - right.value);
}

Const Const::operator*(Const right) {
    return Const(castTypes(type, right.type), value * right.value);
}

Const Const::operator/(Const right) {
    exprType _type = castTypes(type, right.type);
    double val = value / right.value;
    if (_type == exprType::Integer)
        val = (double)(int)val;
    return Const(_type, val);
}

Const Const::operator-() {
    return Const(type, -value);
}

Const Const::operator&(Const right) {
    if (type != right.type && type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)((i64)value & (i64)right.value));
}

Const Const::operator^(Const right) {
    if (type != right.type && type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)((i64)value ^ (i64)right.value));
}

Const Const::operator|(Const right) {
    if (type != right.type && type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)((i64)value | (i64)right.value));
}

Const Const::operator >> (Const right) {
    if (type != right.type && type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)((i64)value >> (i64)right.value));
}

Const Const::operator<<(Const right) {
    if (type != right.type && type != exprType::Integer)
        throw "Bad type"; //todo
    return Const(type, (double)((i64)value << (i64)right.value));
}

#include "Parser.h"

Parser::Parser(const char* fname, bool isSymbolCheck) :
    _progName("main"),
    _scanner(fname),
    _symTables(SymTableStackPtr(new SymTableStack(SymTablePtr(new SymTable())))),
    _isSymbolCheck(isSymbolCheck),
    _code() {

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

    _computableUnOps[TokenType::Sub] = &Const::ComputeUnarySubtraction;
    _computableUnOps[TokenType::Not] = &Const::ComputeUnaryNeagtion;
    _computableUnOps[TokenType::Add] = &Const::ComputeUnaryAddition;

    _computableBinOps[TokenType::Sub] = &Const::ComputeBinarySubtraction;
    _computableBinOps[TokenType::Add] = &Const::ComputeBinaryAddition;
    _computableBinOps[TokenType::Mul] = &Const::ComputeBinaryMultiplication;
    _computableBinOps[TokenType::Div] = &Const::ComputeBinaryDiv;
    _computableBinOps[TokenType::DivReal] = &Const::ComputeBinaryDivision;
    _computableBinOps[TokenType::Equal] = &Const::ComputeBinaryEq;
    _computableBinOps[TokenType::NotEqual] = &Const::ComputeBinaryNe;
    _computableBinOps[TokenType::Greater] = &Const::ComputeBinaryGt;
    _computableBinOps[TokenType::Less] = &Const::ComputeBinaryLt;
    _computableBinOps[TokenType::GreaterEqual] = &Const::ComputeBinaryGe;
    _computableBinOps[TokenType::LessEqual] = &Const::ComputeBinaryLe;
    _computableBinOps[TokenType::And] = &Const::ComputeBinaryAnd;
    _computableBinOps[TokenType::Or] = &Const::ComputeBinaryOr;
    _computableBinOps[TokenType::Xor] = &Const::ComputeBinaryXor;

    _symTables->top()->add(SymbolPtr(new SymType(SymbolType::TypeChar, "char", 1)));
    _symTables->top()->add(SymbolPtr(new SymType(SymbolType::TypeReal, "float", 8)));
    _symTables->top()->add(SymbolPtr(new SymType(SymbolType::TypeInteger, "integer", 8)));

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

std::string Parser::getAsmStr() {
    parse();
    for (auto symbol : _symTables->top()->getSymbols()) {
        symbol->generateDecl(_code);
        if (symbol->getType() == SymbolType::Proc || symbol->getType() == SymbolType::Func)
            generateProc(symbol, 0);
    }
    _code.addLabel(std::string("main"));
    _code.addCmd(MOV, RBP, RSP);
    _root->generate(_code);
    //_code.addCmd(MOV, RBP, RSP);
    std::ofstream tmp("tmp.asm");
    tmp << _code.toString();
    tmp.close();
    system("nasm -f win64 tmp.asm -o tmp.o");
    system("gcc tmp.o -o tmp.exe");
    system("tmp.exe > tmp.out");
    //system("run_asm.bat");
    std::ifstream sout("tmp.out");
    std::string out;
    out = std::string(std::istreambuf_iterator<char>(sout),
                      std::istreambuf_iterator<char>());
    sout.close();
    return out;
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
        case TokenType::String:
            _scanner.next();
            return PNode(new StringConstNode(t->getValue()));
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

PNode Parser::parseOnlyIdentifier() {
    TokenPtr t = _scanner.getToken();
    TokenPtr indentToken = t;
    SymbolPtr sym = _symTables->getSymbol(t, _isSymbolCheck);
    return PNode(new IdentifierNode(t->getValue(), sym));
}

PNode Parser::parseIdentifier() {
    TokenPtr t = _scanner.getToken();
    TokenPtr indentToken = t;
    SymbolPtr sym = _symTables->getSymbol(t, _isSymbolCheck);
    PNode result(new IdentifierNode(t->getValue(), sym));
    bool isDone = false;
    while (!isDone) {
        t = _scanner.getNextToken();
        switch (t->getType()) {
            case TokenType::Dot:
            {
                t = _scanner.getNextToken();
                _scanner.expect(TokenType::Identifier);
                if (_isSymbolCheck) {
                    if (sym->getVarType() != SymbolType::TypeRecord)
                        throw IllegalQualifier(t->getLine(), t->getCol());
                    _symTables->addTable(std::dynamic_pointer_cast<SymTypeRecord>(sym->getVarTypeSymbol())->getTable());
                    PNode right = parseOnlyIdentifier();
                    _symTables->pop();
                    SymbolPtr attr = sym->getVarTypeSymbol();
                    SymTypeRecordPtr rec = std::dynamic_pointer_cast<SymTypeRecord>(attr);
                    if (!rec->have(t->getText()))
                        throw NoMember(t->getLine(), t->getCol(), t->getText());
                    result = PNode(new RecordAccessNode(result, right, sym));
                    sym = rec->getSymbol(t->getText());
                }
                else
                    result = PNode(new RecordAccessNode(result, parseOnlyIdentifier()));
                break;
            }
            case TokenType::OpeningSquareBracket:
            {
                std::vector<PNode> args = parseCommaSeparated();
                SymbolPtr attr = nullptr;
                if (_isSymbolCheck) {
                    if (sym->getVarType() != SymbolType::TypeArray &&  sym->getVarType() != SymbolType::TypeOpenArray ||
                        args.size() != std::dynamic_pointer_cast<SymTypeArray>(std::dynamic_pointer_cast<SymVar>(sym)->getVarTypeSymbol())->getDimension())
                        throw IllegalQualifier(t->getLine(), t->getCol());
                    SymbolType type = sym->getVarTypeSymbol()->getVarType();
                    if (type == SymbolType::TypeRecord || type == SymbolType::TypeArray)
                        attr = sym->getVarTypeSymbol();
                    else
                        attr = sym;
                }
                result = PNode(new ArrayIndexNode(result, args, sym));
                sym = attr;
                _scanner.expect(TokenType::ClosingSquareBracket);
                break;
            }
            case TokenType::OpeningParenthesis:
            {
                auto tmp = sym->getType();
                if (sym->getType() != SymbolType::Func && sym->getType() != SymbolType::Proc) {
                    isDone = true;
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
                isDone = true;
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
        case TokenType::Write: statement = parseWrite(); break;
        case TokenType::Writeln: statement = parseWriteln(); break;
        case TokenType::Break: statement = parseBreak(); break;
        case TokenType::Continue: statement = parseContinue(); break;
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
        if (getToken()->getType() != TokenType::End && getToken()->getType() != TokenType::Until)
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
    BlockNode* body = new BlockNode("repeat block");
    parseStatementSequence(body);
    if (getToken()->getType() == TokenType::Semicolon)
        _scanner.next();
    _scanner.expect(TokenType::Until);
    TokenPtr tok = getNextToken();
    PNode cond = parseExpr(0);
    expectType(SymbolType::TypeBoolean, cond, tok);
    return PNode(new RepeatNode(cond, PNode(body)));
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
    /*else if (expr->getNodeType() != SynNodeType::Call) {
        throw InvalidExpression(tok->getLine(), tok->getCol());*/
        //}
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
    return indices;
}

SymbolPtr Parser::parseRecord() {
    _symTables->addTable(SymTablePtr(new SymTable()));
    if (getToken()->getType() == TokenType::Identifier)
        parseVarDeclaration(false);
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
    _root = stmt;
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
                parseVarDeclaration(isGlobal);
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

void Parser::generateProc(SymbolPtr symbol, int depth) {
    SymProcBasePtr proc = std::dynamic_pointer_cast<SymProcBase>(symbol);
    _code.addLabel(symbol->getName() + std::to_string(proc->getDepth()));
    _code.addCmd(PUSH, RBP);
    _code.addCmd(MOV, RBP, RSP);
    _code.addCmd(SUB, RSP, proc->getLocals()->getSize());
    _procedureBodies[symbol]->generate(_code);
    _code.addCmd(MOV, RSP, RBP);
    _code.addCmd(POP, RBP);
    _code.addCmd(RET);
    for (auto sym : proc->getLocals()->getSymbols()) {
        //if (sym->getName() == "write" && sym->getName() == "writeln") {
        //    throw "Error"; //todo, maybe replace
        //}
        if (sym->getType() == SymbolType::Proc || sym->getType() == SymbolType::Func)
            generateProc(sym, depth + 1);
    }
}

PNode Parser::parseWrite() {
    _scanner.next();
    _scanner.expect(TokenType::OpeningParenthesis);
    _scanner.next();
    std::vector<PNode> args = getArgsArray(TokenType::ClosingParenthesis);
    _scanner.next();
    return PNode(new WriteNode(PNode(new IdentifierNode("write", nullptr)), args));
}

PNode Parser::parseWriteln() {
    _scanner.next();
    _scanner.expect(TokenType::OpeningParenthesis);
    _scanner.next();
    std::vector<PNode> args = getArgsArray(TokenType::ClosingParenthesis);
    _scanner.next();
    return PNode(new WritelnNode(PNode(new IdentifierNode("writeln", nullptr)), args));
}

PNode Parser::parseBreak() {
    _scanner.next();
    return PNode(new BreakNode());
}

PNode Parser::parseContinue() {
    _scanner.next();
    return PNode(new ContinueNode());
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

Const Parser::ComputeConstantExpression(PNode node) {
    if (*node == SynNodeType::RecordAccess || *node == SynNodeType::ArrayIndex)
        throw InvalidExpression(getToken()->getLine(), getToken()->getCol());

    if (*node == SynNodeType::UnaryOp) {
        auto it = _computableUnOps.find(std::dynamic_pointer_cast<OpNode>(node)->getOpType());
        if (it == _computableUnOps.end())
            throw InvalidExpression(getToken()->getLine(), getToken()->getCol());
        return  (*(it->second))(ComputeConstantExpression(std::dynamic_pointer_cast<UnaryNode>(node)->getArg()));
    }
    else if (*node == SynNodeType::BinaryOp) {
        auto it = _computableBinOps.find(std::dynamic_pointer_cast<OpNode>(node)->getOpType());
        if (it == _computableBinOps.end())
            throw InvalidExpression(getToken()->getLine(), getToken()->getCol());
        return (*(it->second))(ComputeConstantExpression(std::dynamic_pointer_cast<BinOpNode>(node)->getLeft()),
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
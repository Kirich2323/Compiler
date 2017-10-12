#include "TypeChecker.h"

TypeChecker::TypeChecker(SymTableStackPtr tableStack) : _tableStack(tableStack) {}

std::unordered_map <SymbolType, std::unordered_map<TokenType, SymbolType> > opTypeResults = {
    { SymbolType::TypeInteger,
        {
            { TokenType::Add,          SymbolType::TypeInteger },
            { TokenType::Sub,          SymbolType::TypeInteger },
            { TokenType::DivReal,      SymbolType::TypeReal    },
            { TokenType::Div,          SymbolType::TypeInteger },
            { TokenType::Mul,          SymbolType::TypeInteger },
            { TokenType::Mod,          SymbolType::TypeInteger },
            { TokenType::Equal,        SymbolType::TypeBoolean },
            { TokenType::NotEqual,     SymbolType::TypeBoolean },
            { TokenType::Greater,      SymbolType::TypeBoolean },
            { TokenType::GreaterEqual, SymbolType::TypeBoolean },
            { TokenType::Less,         SymbolType::TypeBoolean },
            { TokenType::LessEqual,    SymbolType::TypeBoolean },
            { TokenType::Shl,          SymbolType::TypeInteger },
            { TokenType::Shr,          SymbolType::TypeInteger },
            { TokenType::And,          SymbolType::TypeInteger },
            { TokenType::Or,           SymbolType::TypeInteger },
            { TokenType::Xor,          SymbolType::TypeInteger },

        }
    },
    { SymbolType::TypeReal,
        {
            { TokenType::Add,          SymbolType::TypeReal    },
            { TokenType::Sub,          SymbolType::TypeReal    },
            { TokenType::DivReal,      SymbolType::TypeReal    },
            { TokenType::Mul,          SymbolType::TypeReal    },
            { TokenType::NotEqual,     SymbolType::TypeBoolean },
            { TokenType::Greater,      SymbolType::TypeBoolean },
            { TokenType::Less,         SymbolType::TypeBoolean },
            { TokenType::GreaterEqual, SymbolType::TypeBoolean },
            { TokenType::LessEqual,    SymbolType::TypeBoolean },
            { TokenType::Equal,        SymbolType::TypeBoolean },
       }
    },
    { SymbolType::TypeBoolean,
        {
            { TokenType::And,          SymbolType::TypeBoolean },
            { TokenType::Or,           SymbolType::TypeBoolean },
            { TokenType::Xor,          SymbolType::TypeBoolean },
        }
    }
};

std::unordered_map<SymbolType, std::set<SymbolType>> typeCasts = {
    { SymbolType::TypeInteger, { SymbolType::TypeInteger, SymbolType::TypeReal, SymbolType::TypeBoolean } },
    { SymbolType::TypeReal, { SymbolType::TypeReal } },
    { SymbolType::TypeBoolean,{ SymbolType::TypeBoolean } },
};

SymbolType TypeChecker::getExprType(PNode exp) {
    switch (exp->getNodeType()) {
        case SynNodeType::BinaryOp:
        {
            TokenType op = std::dynamic_pointer_cast<BinOpNode>(exp)->getOpType();
            SymbolType left = getExprType(std::dynamic_pointer_cast<BinOpNode>(exp)->getLeft());
            SymbolType right = getExprType(std::dynamic_pointer_cast<BinOpNode>(exp)->getRight());
            return calcTypeResult(tryCast(left, right), op);
        }
        case SynNodeType::ArrayIndex:
        {
            PNode expr = exp;
            SymbolPtr arr = std::dynamic_pointer_cast<SymVar>(std::dynamic_pointer_cast<ArrayIndexNode>(expr)->getSymbol())->getVarTypeSymbol();
            SymbolType type = std::dynamic_pointer_cast<SymTypeArray>(arr)->getArrType();
            return type;
        }
        case SynNodeType::Identifier:
        {
            IdentifierNodePtr tmpPtr = std::dynamic_pointer_cast<IdentifierNode>(exp);
            SymbolType type = tmpPtr->getSymbol()->getVarType();
            if (type == SymbolType::TypeAlias)
                type = std::dynamic_pointer_cast<SymTypeAlias>(tmpPtr->getSymbol())->getRefType();
            return type;
        }
        case SynNodeType::Call:
        {
            SymbolPtr sym = std::dynamic_pointer_cast<CallNode>(exp)->getSymbol();
            if (sym->getType() == SymbolType::Proc)
                throw "smth";
            if (sym->getType() == SymbolType::Func)
                return std::dynamic_pointer_cast<SymProcBase>(sym)->getArgs()->getSymbols().back()->getVarType();
        }
        case SynNodeType::RecordAccess:
            while (exp->getNodeType() == SynNodeType::RecordAccess)
                exp = std::dynamic_pointer_cast<RecordAccessNode>(exp)->getRight();
            if (exp->getNodeType() == SynNodeType::Identifier)
                return getExprType(exp);
        case SynNodeType::UnaryOp:
            return getExprType(std::dynamic_pointer_cast<UnaryNode>(exp)->getArg());
    }
    return getExprType(exp->getNodeType());
}

SymbolType TypeChecker::getExprType(SynNodeType type) {
    switch (type) {
        case SynNodeType::IntegerNumber: return SymbolType::TypeInteger;
        case SynNodeType::RealNumber: return SymbolType::TypeReal;            
    }
}

bool TypeChecker::checkExprType(SymbolType type, PNode expr) {
    SymbolType exprType = getExprType(expr);
    if (type == exprType)
        return true;
    else
        return canCast(exprType, type);
}

bool TypeChecker::equalTypes(SymbolType left, SymbolType right) {
    return left == right;
}

SymbolType TypeChecker::tryCast(SymbolType left, SymbolType right) {
    if (canCast(left, right))
        return right;
    else if (canCast(right, left))
        return left;
    else
        return SymbolType::TypeBadType;
}

void TypeChecker::setTableStack(SymTableStackPtr tableStack) {
    _tableStack = tableStack;
}

bool TypeChecker::canCast(SymbolType from, SymbolType to) {
    return typeCasts[from].find(to) != typeCasts[from].end();
}

SymbolType TypeChecker::calcTypeResult(SymbolType type, TokenType tokType) {
    if (opTypeResults[type].find(tokType) != opTypeResults[type].end())
        return opTypeResults[type][tokType];
    return SymbolType::TypeBadType;
}

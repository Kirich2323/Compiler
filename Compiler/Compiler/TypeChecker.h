#pragma once
#include <unordered_map>
#include <set>
#include <map>
#include "Symbol.h"
#include "SynNode.h"

class TypeChecker {
public:
    TypeChecker(SymTableStackPtr tableStack = nullptr);
    SymbolType getExprType(PNode exp);
    SymbolType getExprType(SynNodeType type);
    bool checkExprType(SymbolType type, PNode expr);
    bool equalTypes(SymbolType left, SymbolType right);
    SymbolType tryCast(SymbolType left, SymbolType right);
    void setTableStack(SymTableStackPtr tableStack);

    std::map<SymbolType, std::string> typeNames = {
        { SymbolType::TypeInteger, "integer" },
        { SymbolType::TypeReal,    "float"   },
        { SymbolType::TypeBoolean, "boolean" },
        { SymbolType::TypeChar,    "char"    },
        { SymbolType::None,        "None"    },
        { SymbolType::TypeRecord,  "record"  }
    };

private:
    bool canCast(SymbolType from, SymbolType to);
    SymbolType calcTypeResult(SymbolType type, TokenType tokType);
    SymTableStackPtr _tableStack;
};
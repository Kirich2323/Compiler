#pragma once

enum exprType {
    Real,
    Integer,
};

typedef long long i64;

struct Const {
    Const(exprType type, double value);
    template<class T>
    T getValue() {
        return T(value);
    }

    static Const ComputeUnaryAddition(Const arg);
    static Const ComputeUnarySubtraction(Const arg);
    static Const ComputeUnaryNeagtion(Const arg);
    static Const ComputeBinaryMultiplication(Const left, Const right);
    static Const ComputeBinaryDivision(Const left, Const);
    static Const ComputeBinaryDiv(Const left, Const right);
    static Const ComputeBinaryMod(Const left, Const right);
    static Const ComputeBinaryAnd(Const left, Const right);
    static Const ComputeBinaryShl(Const left, Const right);
    static Const ComputeBinaryShr(Const left, Const right);
    static Const ComputeBinaryAddition(Const left, Const right);
    static Const ComputeBinarySubtraction(Const left, Const right);
    static Const ComputeBinaryOr(Const left, Const right);
    static Const ComputeBinaryXor(Const left, Const right);
    static Const ComputeBinaryEq(Const left, Const right);
    static Const ComputeBinaryNe(Const left, Const right);
    static Const ComputeBinaryLt(Const left, Const right);
    static Const ComputeBinaryGt(Const left, Const right);
    static Const ComputeBinaryLe(Const left, Const right);
    static Const ComputeBinaryGe(Const left, Const right);   

    exprType type;
    double value;
    Const operator == (Const right);
    Const operator < (Const right);
    Const operator > (Const right);
    Const operator <= (Const right);
    Const operator >= (Const right);
    Const operator != (Const right);
    Const operator ! ();
    Const operator % (Const right);
    Const operator + (Const right);
    Const operator - (Const right);
    Const operator * (Const right);
    Const operator / (Const right);
    Const operator - ();
    Const operator & (Const right);
    Const operator ^ (Const right);
    Const operator | (Const right);
    Const operator >> (Const val);
    Const operator << (Const val);
};

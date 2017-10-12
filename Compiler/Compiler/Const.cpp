#include "Const.h"

exprType castTypes(exprType left, exprType right) {
    if (left == right)
        return left;
    else {
        return exprType::Real;
    }
}

Const Const::ComputeUnaryAddition(Const arg) {
    return arg;
}

Const Const::ComputeUnarySubtraction(Const arg) {
    return -arg;
}

Const Const::ComputeUnaryNeagtion(Const arg) {
    return !arg;
}

Const Const::ComputeBinaryMultiplication(Const left, Const right) {
    return left * right;
}

Const Const::ComputeBinaryDivision(Const left, Const right) {
    return left / right;
}

Const Const::ComputeBinaryDiv(Const left, Const right) {
    return left / right;
}

Const Const::ComputeBinaryMod(Const left, Const right) {
    return left % right;
}

Const Const::ComputeBinaryAnd(Const left, Const right) {
    return left & right;
}

Const Const::ComputeBinaryShl(Const left, Const right) {
    return left << right;
}

Const Const::ComputeBinaryShr(Const left, Const right) {
    return left >> right;
}

Const Const::ComputeBinaryAddition(Const left, Const right) {
    return left + right;
}

Const Const::ComputeBinarySubtraction(Const left, Const right) {
    return left - right;
}

Const Const::ComputeBinaryOr(Const left, Const right) {
    return left | right;
}

Const Const::ComputeBinaryXor(Const left, Const right) {
    return left ^ right;
}

Const Const::ComputeBinaryEq(Const left, Const right) {
    return left == right;
}

Const Const::ComputeBinaryNe(Const left, Const right) {
    return left != right;
}

Const Const::ComputeBinaryLt(Const left, Const right) {
    return left < right;
}

Const Const::ComputeBinaryGt(Const left, Const right) {
    return left > right;
}

Const Const::ComputeBinaryLe(Const left, Const right) {
    return left <= right;
}

Const Const::ComputeBinaryGe(Const left, Const right) {
    return left >= right;
}

Const::Const(exprType type, double value) : type(type), value(value) {}

Const Const::operator==(Const right) {
    return Const(exprType::Integer, value == right.value);
}

Const Const::operator<(Const right) {
    return Const(exprType::Integer, value < right.value);
}

Const Const::operator>(Const right) {
    return Const(type, (double)((i64)value > (i64)right.value));
}

Const Const::operator<=(Const right) {
    return Const(type, (double)((i64)value <= (i64)right.value));
}

Const Const::operator>=(Const right) {
    return Const(type, (double)((i64)value >= (i64)right.value));
}

Const Const::operator!=(Const right) {
    return Const(type, (double)((i64)value != (i64)right.value));
}

Const Const::operator!() {
    return Const(type, (double)(!((i64)value)));
}

Const Const::operator%(Const right) {
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
    return Const(type, (double)((i64)value & (i64)right.value));
}

Const Const::operator^(Const right) {
    return Const(type, (double)((i64)value ^ (i64)right.value));
}

Const Const::operator|(Const right) {
    return Const(type, (double)((i64)value | (i64)right.value));
}

Const Const::operator >> (Const right) {
    return Const(type, (double)((i64)value >> (i64)right.value));
}

Const Const::operator<<(Const right) {
    return Const(type, (double)((i64)value << (i64)right.value));
}
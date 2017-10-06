#include "Const.h"

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
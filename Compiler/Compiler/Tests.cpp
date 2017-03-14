#include "Tests.h"

TEST_F(ScannerTest, SingleCharacterIdentifire) {
check("test000");
}

TEST_F(ScannerTest, MultiCharacterIdentifire) {
    check("test002");
}

TEST_F(ScannerTest, EmptyFile) {
    check("test001");
}

TEST_F(ScannerTest, SkipSpaces) {
    check("test003");
}

TEST_F(ScannerTest, SkipEmptyLines) {
    check("test004");
}

TEST_F(ScannerTest, SkipSingleLineComment) {
    check("test005");
}

TEST_F(ScannerTest, SkipMultiLineComment) {
    check("test006");
}

TEST_F(ScannerTest, SkipTwoSingleLineComments) {
    check("test007");
}

TEST_F(ScannerTest, TwoIdentifiers) {
    check("test008");
}

TEST_F(ScannerTest, DecimalInteger) {
    check("test009");
}

TEST_F(ScannerTest, Real1) {
    check("test010");
}

TEST_F(ScannerTest, Real2) {
    check("test011");
}

TEST_F(ScannerTest, Real3) {
    check("test012");
}

TEST_F(ScannerTest, HexadecimalInteger1) {
    check("test013");
}

TEST_F(ScannerTest, HexadecimalInteger2) {
    check("test014");
}

TEST_F(ScannerTest, OctalInteger) {
    check("test015");
}

TEST_F(ScannerTest, BinaryInteger) {
    check("test016");
}

TEST_F(ScannerTest, String) {
    check("test017");
}

TEST_F(ScannerTest, Operations) {
    check("test018");
}

TEST_F(ScannerTest, Keywords) {
    check("test019");
}

TEST_F(ScannerTest, Delimiters) {
    check("test020");
}

TEST_F(ScannerTest, Range) {
    check("test021");
}

TEST_F(ScannerTest, OperationWithIntegers) {
    check("test022");
}

TEST_F(ScannerTest, OperationWithReals) {
    check("test023");
}

TEST_F(ScannerTest, InvalidInteger1) {
    check("test024");
}

TEST_F(ScannerTest, InvalidInteger2) {
    check("test025");
}

TEST_F(ScannerTest, InvalidInteger3) {
    check("test026");
}

TEST_F(ScannerTest, InvalidInteger4) {
    check("test027");
}

TEST_F(ScannerTest, InvalidReal1) {
    check("test028");
}

TEST_F(ScannerTest, InvalidReal2) {
    check("test029");
}

TEST_F(ScannerTest, InvalidReal3) {
    check("test030");
}

TEST_F(ScannerTest, InvalidReal4) {
    check("test031");
}

TEST_F(ScannerTest, UnterminatedString1) {
    check("test032");
}

TEST_F(ScannerTest, UnterminatedString2) {
    check("test033");
}

TEST_F(ScannerTest, UnterminatedComment) {
    check("test034");
}

TEST_F(ScannerTest, MissingFile) {
    EXPECT_THROW(check("test035"), MissingFile);
}

TEST_F(ParserTest, EmptyFile) {
    //EXPECT_THROW(check("test000"), UnexpectedEndOfFile);
    check_throw("test000");
}

TEST_F(ParserTest, IntegerNode) {
    check("test001");
}

TEST_F(ParserTest, RealNode) {
    check("test002");
}

TEST_F(ParserTest, IdentifierNode) {
    check("test003");
}

TEST_F(ParserTest, Equal) {
    check("test004");
}

TEST_F(ParserTest, NotEqual) {
    check("test005");
}

TEST_F(ParserTest, Less) {
    check("test006");
}

TEST_F(ParserTest, LessEqual) {
    check("test007");
}

TEST_F(ParserTest, Greater) {
    check("test008");
}

TEST_F(ParserTest, GreaterEqual) {
    check("test009");
}

TEST_F(ParserTest, Add2Factors) {
    check("test010");
}

TEST_F(ParserTest, Add3Factors) {
    check("test011");
}

TEST_F(ParserTest, Sub2Factors) {
    check("test012");
}

TEST_F(ParserTest, Sub3Factors) {
    check("test013");
}

TEST_F(ParserTest, OrFactors) {
    check("test014");
}

TEST_F(ParserTest, XorFactors) {
    check("test015");
}

TEST_F(ParserTest, ShiftLeft) {
    check("test016");
}

TEST_F(ParserTest, ShiftRight) {
    check("test017");
}

TEST_F(ParserTest, Mult2Factors) {
    check("test018");
}

TEST_F(ParserTest, Mult3Factors) {
    check("test019");
}

TEST_F(ParserTest, AddAndMult) {
    check("test020");
}

TEST_F(ParserTest, GroupedAddAndMult) {
    check("test021");
}

TEST_F(ParserTest, Div2Factors) {
    check("test022");
}

TEST_F(ParserTest, Div3Factors) {
    check("test023");
}

TEST_F(ParserTest, Mod2Factors) {
    check("test024");
}

TEST_F(ParserTest, Mod3Factors) {
    check("test025");
}

TEST_F(ParserTest, RealDiv2Factors) {
    check("test026");
}

TEST_F(ParserTest, RealDiv3Factors) {
    check("test027");
}

TEST_F(ParserTest, And) {
    check("test028");
}

TEST_F(ParserTest, UnarMinus) {
    check("test029");
}

TEST_F(ParserTest, UnarPlus) {
    check("test030");
}

TEST_F(ParserTest, UnarNot) {
    check("test031");
}

TEST_F(ParserTest, UnarCombination) {
    check("test032");
}

TEST_F(ParserTest, RecordAccess) {
    check("test033");
}

TEST_F(ParserTest, MultipleRecordAccess) {
    check("test034");
}

TEST_F(ParserTest, ArrayIndex) {
    check("test035");
}

TEST_F(ParserTest, MultidimensionalArrayIndex) {
    check("test036");
}

TEST_F(ParserTest, CombinationOfRecordAccesAndArrayIndeces1) {
    check("test037");
}

TEST_F(ParserTest, CombinationOfRecordAccesAndArrayIndeces2) {
    check("test038");
}

TEST_F(ParserTest, ComplexExpression1) {
    check("test039");
}

TEST_F(ParserTest, ComplexExpression2) {
    check("test040");
}

TEST_F(ParserTest, MissingClosingParenthesis) {
    check_throw("test041");
}

TEST_F(ParserTest, MissingClosingSquareBracket1) {
    check_throw("test042");
}

TEST_F(ParserTest, MissingClosingSquareBracket2) {
    check_throw("test043");
}

TEST_F(ParserTest, MissingIdentifierAfterRecordAccess) {
    check_throw("test044");
}

TEST_F(ParserTest, MissingIdentgIdentifierAfterOperation) {
    check_throw("test045");
}


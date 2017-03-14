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

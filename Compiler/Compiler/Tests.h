#pragma once
#include <fstream>
#include <string>

#include "gtest\gtest.h"
#include "gtest\gtest-param-test.h"
#include "Scanner.h"
#include "Parser.h"
//#include "Symbol.h"

template<class T>
class BaseTest : public ::testing::TestWithParam<std::string> {
protected:
    virtual std::string getPath() = 0;
    virtual std::string getData(T& obj) = 0;
    virtual void modifyObj(T& obj) {}
    void check(const std::string& fname) {
        std::ifstream test_stream(getPath() + fname + ".out");
        expected = std::string(std::istreambuf_iterator<char>(test_stream),
                               std::istreambuf_iterator<char>());

        T obj((getPath() + fname + ".in").c_str());
        modifyObj(obj);
        testing = getData(obj);
        ASSERT_STREQ(testing.c_str(), expected.c_str());
    }
    void check_throw(const std::string& fname) {
        try {
            check(fname);
        }
        catch (BaseException e) {
            std::ifstream test_stream(getPath() + fname + ".out");
            expected = std::string(std::istreambuf_iterator<char>(test_stream),
                                   std::istreambuf_iterator<char>());
            ASSERT_STREQ(e.what(), expected.c_str());
        }
    }
    std::string expected;
    std::string testing;
};

class ScannerBaseTest : public BaseTest<Scanner> {   
    std::string getPath() override { return "../Tests/scanner_tests/"; }
    std::string getData(Scanner& obj) override { return obj.getTokensString(); } 
};
class ScannerCheckTest : public ScannerBaseTest {};
class ScannerThrowTest : public ScannerBaseTest {};

class ParserExprBaseTest : public BaseTest<Parser> {
    std::string getPath() override { return "../Tests/parser_exp_tests/"; }
    std::string getData(Parser& obj) override { return obj.getNodeTreeStr(); }
    void modifyObj(Parser& obj) override { obj.setSymbolCheck(false); }
};
class ParserExpTest : public ParserExprBaseTest {};
class ParserExpThrowTest : public ParserExprBaseTest {};

class ParserDeclBaseTest : public BaseTest<Parser> {
    std::string getPath() override { return "../Tests/parser_decl_tests/"; }
    std::string getData(Parser& obj) override { return obj.getDeclStr(); }    
};
class ParserDeclTest : public ParserDeclBaseTest {};
class ParserDeclThrowTest : public ParserDeclBaseTest {};

class ParserStatementBaseTest : public BaseTest<Parser> {
    std::string getPath() override { return "../Tests/parser_statements_tests/"; }
    std::string getData(Parser& obj) override { return obj.getStmtStr(); }
};
class ParserStatementCheckTest : public ParserStatementBaseTest {};
class ParserStatementCheckThrowTest : public ParserStatementBaseTest {};

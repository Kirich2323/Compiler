#pragma once
#include <fstream>
#include <string>

#include "gtest\gtest.h"
#include "Scanner.h"
#include "Parser.h"

class ScannerTest : public ::testing::Test {
protected:
    void check(const std::string& fname) 
    {
        std::ifstream test_stream("../Tests/scanner_tests/" + fname + ".out");
        expected = std::string(std::istreambuf_iterator<char>(test_stream),
                               std::istreambuf_iterator<char>());
        testing = Scanner( ("../Tests/scanner_tests/" + fname + ".in").c_str() ).getTokensString();
        ASSERT_STREQ(testing.c_str(), expected.c_str());
    }
    std::string expected;
    std::string testing;    
};

class ParserTest : public ::testing::Test {
protected:
    void check(const std::string& fname) {
        std::ifstream test_stream("../Tests/parser_tests/" + fname + ".out");       
        expected = std::string(std::istreambuf_iterator<char>(test_stream),
                               std::istreambuf_iterator<char>());
        testing = Parser(("../Tests/parser_tests/" + fname + ".in").c_str()).getNodeTreeStr();
        ASSERT_STREQ(testing.c_str(), expected.c_str());
    }

    void check_throw(const std::string& fname) {
        try {
            check(fname);
        }
        catch (BaseException e) {
            std::ifstream test_stream("../Tests/parser_tests/" + fname + ".out");
            expected = std::string(std::istreambuf_iterator<char>(test_stream),
                                   std::istreambuf_iterator<char>());
            ASSERT_STREQ(e.what(), expected.c_str());

        }
    }
    std::string expected;
    std::string testing;
};
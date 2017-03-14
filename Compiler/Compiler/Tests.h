#pragma once
#include <fstream>
#include <string>

#include "gtest\gtest.h"
#include "Scanner.h"

class ScannerTest : public ::testing::Test {
protected:
    void check(const std::string& fname) 
    {
        test_stream = std::ifstream("../Tests/" + fname + ".out");
        expected = std::string(std::istreambuf_iterator<char>(test_stream),
                               std::istreambuf_iterator<char>());
        testing = Scanner( ("../Tests/" + fname + ".in").c_str() ).getTokensString();
        ASSERT_STREQ(testing.c_str(), expected.c_str());
    }
    std::string expected;
    std::string testing;    
};



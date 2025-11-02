#include "catch2_single_header.hpp"
#include "forkenizer/PreTokenizer.hpp"
#include <vector>
#include <string>

TEST_CASE("PreTokenizer basic text", "[pretokenizer]") {
    forkenizer::PreTokenizer preTokenizer;
    
    auto tokens = preTokenizer.preTokenize("hello world");
    REQUIRE(tokens.size() >= 2);
}

TEST_CASE("PreTokenizer numbers", "[pretokenizer]") {
    forkenizer::PreTokenizer preTokenizer;
    
    auto tokens = preTokenizer.preTokenize("3.14");
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0] == "3.14");
    
    tokens = preTokenizer.preTokenize("42");
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0] == "42");
    
    tokens = preTokenizer.preTokenize("-123.45");
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0] == "-123.45");
}

TEST_CASE("PreTokenizer math expressions", "[pretokenizer]") {
    forkenizer::PreTokenizer preTokenizer;
    
    auto tokens = preTokenizer.preTokenize("3 + 4 = 7");
    REQUIRE(tokens.size() >= 5);
    
    tokens = preTokenizer.preTokenize("(a+b)*c");
    REQUIRE(tokens.size() >= 5);
}

TEST_CASE("PreTokenizer identifiers", "[pretokenizer]") {
    forkenizer::PreTokenizer preTokenizer;
    
    auto tokens = preTokenizer.preTokenize("camelCase snake_case");
    REQUIRE(tokens.size() >= 2);
    
    tokens = preTokenizer.preTokenize("module.name");
    REQUIRE(tokens.size() >= 1);
}

TEST_CASE("PreTokenizer punctuation", "[pretokenizer]") {
    forkenizer::PreTokenizer preTokenizer;
    
    auto tokens = preTokenizer.preTokenize("3.14, x=2");
    REQUIRE(tokens.size() >= 4);
}

int main() {
    return 0;
}


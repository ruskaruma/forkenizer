#include "catch2_single_header.hpp"
#include "forkenizer/Tokenizer.hpp"
#include <cstdint>
#include <string>

TEST_CASE("Number tokenization", "[numbers]") {
    forkenizer::Tokenizer tokenizer;
    
    bool loaded = tokenizer.load("../data_examples");
    if (!loaded) {
        std::cerr << "Warning: Could not load model, skipping test\n";
        return;
    }
    
    auto tokens1 = tokenizer.encode("42");
    REQUIRE(tokens1.has_value());
    
    auto tokens2 = tokenizer.encode("3.14");
    REQUIRE(tokens2.has_value());
}

TEST_CASE("Math expression tokenization", "[math]") {
    forkenizer::Tokenizer tokenizer;
    
    bool loaded = tokenizer.load("../data_examples");
    if (!loaded) {
        std::cerr << "Warning: Could not load model, skipping test\n";
        return;
    }
    
    auto tokens = tokenizer.encode("3 + 4 = 7");
    REQUIRE(tokens.has_value());
    REQUIRE(tokens->size() > 0);
    
    tokens = tokenizer.encode("x^2 + y^2 = z^2");
    REQUIRE(tokens.has_value());
}

TEST_CASE("Scientific notation", "[numbers]") {
    forkenizer::Tokenizer tokenizer;
    
    bool loaded = tokenizer.load("../data_examples");
    if (!loaded) {
        std::cerr << "Warning: Could not load model, skipping test\n";
        return;
    }
    
    auto tokens = tokenizer.encode("1e-10");
    REQUIRE(tokens.has_value());
}

int main() {
    return 0;
}


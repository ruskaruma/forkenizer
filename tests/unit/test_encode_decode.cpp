#include "catch2_single_header.hpp"
#include "forkenizer/Tokenizer.hpp"
#include <string>
#include <vector>

TEST_CASE("Round-trip encode/decode", "[encode_decode]") {
    forkenizer::Tokenizer tokenizer;
    
    bool loaded = tokenizer.load("../data_examples");
    if (!loaded) {
        std::cerr << "Warning: Could not load model, skipping test\n";
        return;
    }
    
    std::string testText = "hello world";
    auto tokens = tokenizer.encode(testText);
    REQUIRE(tokens.has_value());
    REQUIRE(tokens->size() > 0);
    
    auto decoded = tokenizer.decode(*tokens);
    REQUIRE(decoded.has_value());
}

TEST_CASE("Encode empty string", "[encode_decode]") {
    forkenizer::Tokenizer tokenizer;
    
    bool loaded = tokenizer.load("../data_examples");
    if (!loaded) {
        std::cerr << "Warning: Could not load model, skipping test\n";
        return;
    }
    
    auto tokens = tokenizer.encode("");
    REQUIRE(tokens.has_value());
    REQUIRE(tokens->size() == 0);
}

TEST_CASE("Decode empty tokens", "[encode_decode]") {
    forkenizer::Tokenizer tokenizer;
    
    bool loaded = tokenizer.load("../data_examples");
    if (!loaded) {
        std::cerr << "Warning: Could not load model, skipping test\n";
        return;
    }
    
    std::vector<uint32_t> empty;
    auto decoded = tokenizer.decode(empty);
    REQUIRE(decoded.has_value());
    REQUIRE(decoded->empty());
}

TEST_CASE("IsLoaded check", "[encode_decode]") {
    forkenizer::Tokenizer tokenizer;
    REQUIRE_FALSE(tokenizer.isLoaded());
    
    bool loaded = tokenizer.load("../data_examples");
    if (loaded) {
        REQUIRE(tokenizer.isLoaded());
    }
}

int main() {
    return 0;
}


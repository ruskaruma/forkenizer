#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <memory>
#include <cstdint>

namespace forkenizer {

class PreTokenizer;

class Tokenizer {
public:
    Tokenizer();
    ~Tokenizer();
    bool load(const std::string& modelDir);
    bool save(const std::string& modelDir) const;
    std::optional<std::vector<uint32_t>> encode(const std::string& text) const;
    std::optional<std::string> decode(const std::vector<uint32_t>& tokens) const;
    void setNormalization(bool enabled);
    bool isLoaded() const;

private:
    std::unordered_map<std::string, uint32_t> tokenToId_;
    std::vector<std::string> idToToken_;
    struct TrieNode {
        std::unordered_map<unsigned char, std::unique_ptr<TrieNode>> children;
        std::optional<uint32_t> tokenId;
        TrieNode() : tokenId(std::nullopt) {}
    };
    std::unique_ptr<TrieNode> trieRoot_;
    std::vector<std::pair<std::string, std::string>> merges_;
    std::unique_ptr<PreTokenizer> preTokenizer_;
    bool normalizationEnabled_ = false;
    bool loaded_ = false;

    void buildTrie_();
};

}


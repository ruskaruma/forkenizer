#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <string>
#include <optional>

namespace forkenizer {

struct TrieNode {
    std::unordered_map<unsigned char, std::unique_ptr<TrieNode>> children;
    std::optional<uint32_t> tokenId;

    TrieNode() : tokenId(std::nullopt) {}
};

class Trie {
public:
    void insert(const std::string& token, uint32_t tokenId);
    std::optional<uint32_t> findLongestMatch(const std::string& text, size_t start, size_t& matchLen) const;

private:
    std::unique_ptr<TrieNode> root_ = std::make_unique<TrieNode>();
};

}


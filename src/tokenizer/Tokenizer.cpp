#include "forkenizer/Tokenizer.hpp"
#include "forkenizer/PreTokenizer.hpp"
#include "forkenizer/ModelIO.hpp"
#include <algorithm>
#include <sstream>

namespace forkenizer {

Tokenizer::Tokenizer() : preTokenizer_(std::make_unique<PreTokenizer>()) {}
Tokenizer::~Tokenizer() = default;

bool Tokenizer::load(const std::string& modelDir) {
    ModelData data;
    if (!loadModel(modelDir, data)) {
        return false;
    }

    tokenToId_ = std::move(data.tokenToId);
    idToToken_ = std::move(data.idToToken);
    merges_ = std::move(data.merges);

    buildTrie_();
    loaded_ = true;
    return true;
}

bool Tokenizer::save(const std::string& modelDir) const {
    if (!loaded_) {
        return false;
    }

    ModelData data;
    data.tokenToId = tokenToId_;
    data.idToToken = idToToken_;
    data.merges = merges_;

    return saveModel(modelDir, data);
}

void Tokenizer::buildTrie_() {
    trieRoot_ = std::make_unique<Tokenizer::TrieNode>();

    for (const auto& pair : tokenToId_) {
        TrieNode* node = trieRoot_.get();
        for (unsigned char byte : pair.first) {
            if (node->children.find(byte) == node->children.end()) {
                node->children[byte] = std::make_unique<TrieNode>();
            }
            node = node->children[byte].get();
        }
        node->tokenId = pair.second;
    }
}

std::optional<std::vector<uint32_t>> Tokenizer::encode(const std::string& text) const {
    if (!loaded_) {
        return std::nullopt;
    }

    std::vector<uint32_t> tokenIds;
    std::vector<std::string> preTokens = preTokenizer_->preTokenize(text);

    for (const auto& preToken : preTokens) {
        size_t pos = 0;
        while (pos < preToken.length()) {
            size_t bestMatchLen = 0;
            uint32_t bestTokenId = 0;
            bool found = false;

            for (size_t len = std::min(preToken.length() - pos, size_t(256)); len > 0; --len) {
                std::string candidate = preToken.substr(pos, len);
                auto it = tokenToId_.find(candidate);
                if (it != tokenToId_.end()) {
                    bestMatchLen = len;
                    bestTokenId = it->second;
                    found = true;
                    break;
                }
            }

            if (found) {
                tokenIds.push_back(bestTokenId);
                pos += bestMatchLen;
            } else {
                for (size_t i = pos; i < preToken.length(); ++i) {
                    unsigned char byte = static_cast<unsigned char>(preToken[i]);
                    std::string byteStr(1, static_cast<char>(byte));
                    auto it = tokenToId_.find(byteStr);
                    if (it != tokenToId_.end()) {
                        tokenIds.push_back(it->second);
                    } else {
                        uint32_t fallbackId = 1;
                        if (!idToToken_.empty()) {
                            fallbackId = 0;
                        }
                        tokenIds.push_back(fallbackId);
                    }
                }
                break;
            }
        }
    }

    return tokenIds;
}

std::optional<std::string> Tokenizer::decode(const std::vector<uint32_t>& tokens) const {
    if (!loaded_) {
        return std::nullopt;
    }

    std::stringstream ss;
    for (uint32_t tokenId : tokens) {
        if (tokenId < idToToken_.size()) {
            ss << idToToken_[tokenId];
        }
    }

    return ss.str();
}

void Tokenizer::setNormalization(bool enabled) {
    normalizationEnabled_ = enabled;
}

bool Tokenizer::isLoaded() const {
    return loaded_;
}

}


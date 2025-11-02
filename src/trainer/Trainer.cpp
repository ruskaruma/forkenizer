#include "Trainer.hpp"
#include "forkenizer/PreTokenizer.hpp"
#include "forkenizer/ModelIO.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

namespace forkenizer {

static void initializeByteVocab(ModelData& data) {
    data.tokenToId["<pad>"] = 0;
    data.tokenToId["<unk>"] = 1;
    data.tokenToId["<bos>"] = 2;
    data.tokenToId["<eos>"] = 3;

    uint32_t nextId = 4;
    for (int i = 0; i < 256; ++i) {
        std::string byteToken(1, static_cast<char>(i));
        if (data.tokenToId.find(byteToken) == data.tokenToId.end()) {
            data.tokenToId[byteToken] = nextId++;
        }
    }
}

static std::vector<std::string> tokenizeToBytes(const std::string& text) {
    std::vector<std::string> tokens;
    tokens.reserve(text.length());
    for (unsigned char byte : text) {
        tokens.emplace_back(1, static_cast<char>(byte));
    }
    return tokens;
}

bool Trainer::train(const std::vector<std::string>& corpusFiles, const std::string& outputDir,
                    uint32_t vocabSize, uint32_t numMerges) {
    ModelData data;
    initializeByteVocab(data);

    PreTokenizer preTokenizer;
    std::vector<std::vector<std::string>> allTokens;

    for (const auto& corpusFile : corpusFiles) {
        std::ifstream file(corpusFile);
        if (!file.is_open()) continue;

        std::string line;
        while (std::getline(file, line)) {
            std::vector<std::string> preTokens = preTokenizer.preTokenize(line);
            std::vector<std::string> byteTokens;
            for (const auto& preToken : preTokens) {
                std::vector<std::string> bytes = tokenizeToBytes(preToken);
                byteTokens.insert(byteTokens.end(), bytes.begin(), bytes.end());
            }
            if (!byteTokens.empty()) {
                allTokens.push_back(byteTokens);
            }
        }
        file.close();
    }

    if (allTokens.empty()) {
        return false;
    }

    std::map<std::pair<std::string, std::string>, uint64_t> pairCounts;
    for (const auto& tokens : allTokens) {
        for (size_t i = 0; i + 1 < tokens.size(); ++i) {
            pairCounts[{tokens[i], tokens[i + 1]}]++;
        }
    }

    uint32_t currentVocabSize = static_cast<uint32_t>(data.tokenToId.size());
    uint32_t mergesPerformed = 0;

    while (currentVocabSize < vocabSize && mergesPerformed < numMerges && !pairCounts.empty()) {
        auto bestPair = std::max_element(pairCounts.begin(), pairCounts.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        if (bestPair == pairCounts.end() || bestPair->second < 2) break;

        std::string merged = bestPair->first.first + bestPair->first.second;
        data.tokenToId[merged] = currentVocabSize++;
        data.merges.emplace_back(bestPair->first.first, bestPair->first.second);

        for (auto& tokens : allTokens) {
            for (size_t i = 0; i + 1 < tokens.size(); ++i) {
                if (tokens[i] == bestPair->first.first && tokens[i + 1] == bestPair->first.second) {
                    tokens[i] = merged;
                    tokens.erase(tokens.begin() + i + 1);
                    if (i > 0) {
                        pairCounts[{tokens[i - 1], tokens[i]}]++;
                    }
                    if (i + 1 < tokens.size()) {
                        pairCounts[{tokens[i], tokens[i + 1]}]++;
                    }
                    break;
                }
            }
        }

        pairCounts.erase(bestPair);
        mergesPerformed++;
    }

    uint32_t maxId = 0;
    for (const auto& pair : data.tokenToId) {
        if (pair.second > maxId) maxId = pair.second;
    }
    data.idToToken.resize(maxId + 1);
    for (const auto& pair : data.tokenToId) {
        if (pair.second < data.idToToken.size()) {
            data.idToToken[pair.second] = pair.first;
        }
    }

    return saveModel(outputDir, data);
}

}


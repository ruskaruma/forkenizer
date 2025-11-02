#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <cstdint>

namespace forkenizer {

struct ModelData {
    std::unordered_map<std::string, uint32_t> tokenToId;
    std::vector<std::string> idToToken;
    std::vector<std::pair<std::string, std::string>> merges;
};

bool loadModel(const std::string& modelDir, ModelData& data);
bool saveModel(const std::string& modelDir, const ModelData& data);

}


#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace forkenizer {

struct ModelData;

class Trainer {
public:
    bool train(const std::vector<std::string>& corpusFiles, const std::string& outputDir,
               uint32_t vocabSize, uint32_t numMerges);
};

}


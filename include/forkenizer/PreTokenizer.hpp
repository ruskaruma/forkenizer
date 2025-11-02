#pragma once

#include <string>
#include <vector>

namespace forkenizer {

class PreTokenizer {
public:
    std::vector<std::string> preTokenize(const std::string& utf8Text) const;
};

}


#include "forkenizer/ModelIO.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <sys/stat.h>
#include <sys/types.h>

namespace forkenizer {

static std::string escapeJsonString(const std::string& str) {
    std::stringstream ss;
    ss << '"';
    for (char c : str) {
        if (c == '"') ss << "\\\"";
        else if (c == '\\') ss << "\\\\";
        else if (c == '\n') ss << "\\n";
        else if (c == '\r') ss << "\\r";
        else if (c == '\t') ss << "\\t";
        else if (static_cast<unsigned char>(c) < 32) {
            ss << "\\x" << std::hex << (static_cast<unsigned int>(static_cast<unsigned char>(c)) & 0xFF);
        } else {
            ss << c;
        }
    }
    ss << '"';
    return ss.str();
}

static std::string unescapeJsonString(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            ++i;
            if (str[i] == 'n') result += '\n';
            else if (str[i] == 'r') result += '\r';
            else if (str[i] == 't') result += '\t';
            else if (str[i] == '"') result += '"';
            else if (str[i] == '\\') result += '\\';
            else if (str[i] == 'x' && i + 2 < str.length()) {
                int byte = 0;
                for (int j = 0; j < 2 && i + 1 + j < str.length(); ++j) {
                    char ch = str[i + 1 + j];
                    if (std::isdigit(static_cast<unsigned char>(ch))) {
                        byte = byte * 16 + (ch - '0');
                    } else if (ch >= 'a' && ch <= 'f') {
                        byte = byte * 16 + (ch - 'a' + 10);
                    } else if (ch >= 'A' && ch <= 'F') {
                        byte = byte * 16 + (ch - 'A' + 10);
                    }
                }
                result += static_cast<char>(byte);
                i += 2;
            } else {
                result += str[i];
            }
        } else {
            result += str[i];
        }
    }
    
    return result;
}

static bool parseJsonVocab(const std::string& content, std::unordered_map<std::string, uint32_t>& tokenToId) {
    size_t i = 0;
    while (i < content.length() && std::isspace(static_cast<unsigned char>(content[i]))) ++i;
    if (i >= content.length() || content[i] != '{') return false;
    ++i;

    while (i < content.length()) {
        while (i < content.length() && std::isspace(static_cast<unsigned char>(content[i]))) ++i;
        if (i >= content.length()) break;
        if (content[i] == '}') break;
        if (content[i] != '"') return false;
        
        ++i;
        std::string token;
        while (i < content.length() && content[i] != '"') {
            if (content[i] == '\\' && i + 1 < content.length()) {
                token += content[i];
                token += content[i + 1];
                i += 2;
            } else {
                token += content[i];
                ++i;
            }
        }
        if (i >= content.length()) return false;
        ++i;

        token = unescapeJsonString(token);

        while (i < content.length() && std::isspace(static_cast<unsigned char>(content[i]))) ++i;
        if (i >= content.length() || content[i] != ':') return false;
        ++i;

        while (i < content.length() && std::isspace(static_cast<unsigned char>(content[i]))) ++i;
        
        uint32_t id = 0;
        while (i < content.length() && std::isdigit(static_cast<unsigned char>(content[i]))) {
            id = id * 10 + (content[i] - '0');
            ++i;
        }

        tokenToId[token] = id;

        while (i < content.length() && std::isspace(static_cast<unsigned char>(content[i]))) ++i;
        if (i < content.length() && content[i] == ',') {
            ++i;
        }
    }

    return true;
}

bool loadModel(const std::string& modelDir, ModelData& data) {
    std::string vocabPath = modelDir + "/vocab.json";
    std::string mergesPath = modelDir + "/merges.txt";

    std::ifstream vocabFile(vocabPath);
    if (!vocabFile.is_open()) {
        return false;
    }

    std::string vocabContent((std::istreambuf_iterator<char>(vocabFile)),
                             std::istreambuf_iterator<char>());
    vocabFile.close();

    if (!parseJsonVocab(vocabContent, data.tokenToId)) {
        return false;
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

    std::ifstream mergesFile(mergesPath);
    if (mergesFile.is_open()) {
        std::string line;
        while (std::getline(mergesFile, line)) {
            if (line.empty()) continue;
            
            size_t spacePos = line.find(' ');
            if (spacePos != std::string::npos) {
                std::string tokenA = line.substr(0, spacePos);
                std::string tokenB = line.substr(spacePos + 1);
                data.merges.emplace_back(tokenA, tokenB);
            }
        }
        mergesFile.close();
    }

    return true;
}

static bool createDirectory(const std::string& path) {
    if (path.empty()) return true;
    size_t pos = 0;
    std::string dir;
    while ((pos = path.find_first_of('/', pos + 1)) != std::string::npos) {
        dir = path.substr(0, pos);
        if (!dir.empty()) {
            mkdir(dir.c_str(), 0755);
        }
    }
    mkdir(path.c_str(), 0755);
    return true;
}

bool saveModel(const std::string& modelDir, const ModelData& data) {
    createDirectory(modelDir);

    std::string vocabPath = modelDir + "/vocab.json";
    std::ofstream vocabFile(vocabPath);
    if (!vocabFile.is_open()) {
        return false;
    }

    vocabFile << '{';
    bool first = true;
    for (const auto& pair : data.tokenToId) {
        if (!first) vocabFile << ',';
        first = false;
        vocabFile << '\n' << "  " << escapeJsonString(pair.first) << ": " << pair.second;
    }
    vocabFile << '\n' << '}' << '\n';
    vocabFile.close();

    std::string mergesPath = modelDir + "/merges.txt";
    std::ofstream mergesFile(mergesPath);
    if (!mergesFile.is_open()) {
        return false;
    }

    for (const auto& merge : data.merges) {
        mergesFile << merge.first << ' ' << merge.second << '\n';
    }
    mergesFile.close();

    return true;
}

}


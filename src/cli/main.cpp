#include "forkenizer/Tokenizer.hpp"
#include "forkenizer/ModelIO.hpp"
#include "../trainer/Trainer.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/stat.h>

static std::string findDefaultModel() {
    const char* candidates[] = {"model", "data_examples", "../data_examples", "./data_examples"};
    for (const char* dir : candidates) {
        struct stat info;
        std::string vocabPath = std::string(dir) + "/vocab.json";
        if (stat(vocabPath.c_str(), &info) == 0) {
            return dir;
        }
    }
    return "";
}

static bool fileExists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0;
}

static void printUsage() {
    std::cerr << "Usage: forkenizer-cli <file>                    # tokenize file\n"
              << "       forkenizer-cli -m <model> <file>        # tokenize with model\n"
              << "       forkenizer-cli decode <file>            # decode token file\n"
              << "       forkenizer-cli train <corpus>...         # train model\n"
              << "\nFull options:\n"
              << "  encode --model <dir> --text <text> [--ids-out <file>]\n"
              << "  decode --model <dir> --ids-file <file> [--text-out <file>]\n"
              << "  inspect --model <dir> --token <token-string>\n";
}

static int cmdEncode(int argc, char* argv[]) {
    std::string modelDir, text, idsOut;
    bool normEnabled = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--model" && i + 1 < argc) {
            modelDir = argv[++i];
        } else if (arg == "--text" && i + 1 < argc) {
            text = argv[++i];
        } else if (arg == "--ids-out" && i + 1 < argc) {
            idsOut = argv[++i];
        } else if (arg == "--norm") {
            if (i + 1 < argc && std::string(argv[i + 1]) == "on") {
                normEnabled = true;
                ++i;
            }
        }
    }

    if (modelDir.empty() || text.empty()) {
        printUsage();
        return 1;
    }

    forkenizer::Tokenizer tokenizer;
    if (!tokenizer.load(modelDir)) {
        std::cerr << "Failed to load model from " << modelDir << "\n";
        return 1;
    }

    tokenizer.setNormalization(normEnabled);
    auto tokens = tokenizer.encode(text);
    if (!tokens.has_value()) {
        std::cerr << "Failed to encode text\n";
        return 1;
    }

    std::ostream* out = &std::cout;
    std::ofstream fileOut;
    if (!idsOut.empty()) {
        fileOut.open(idsOut);
        if (!fileOut.is_open()) {
            std::cerr << "Failed to open output file " << idsOut << "\n";
            return 1;
        }
        out = &fileOut;
    }

    for (size_t i = 0; i < tokens->size(); ++i) {
        if (i > 0) *out << " ";
        *out << (*tokens)[i];
    }
    *out << "\n";

    return 0;
}

static int cmdDecode(int argc, char* argv[]) {
    std::string modelDir, idsFile, textOut;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--model" && i + 1 < argc) {
            modelDir = argv[++i];
        } else if (arg == "--ids-file" && i + 1 < argc) {
            idsFile = argv[++i];
        } else if (arg == "--text-out" && i + 1 < argc) {
            textOut = argv[++i];
        }
    }

    if (modelDir.empty() || idsFile.empty()) {
        printUsage();
        return 1;
    }

    std::ifstream file(idsFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open ids file " << idsFile << "\n";
        return 1;
    }

    std::vector<uint32_t> tokens;
    uint32_t id;
    while (file >> id) {
        tokens.push_back(id);
    }
    file.close();

    forkenizer::Tokenizer tokenizer;
    if (!tokenizer.load(modelDir)) {
        std::cerr << "Failed to load model from " << modelDir << "\n";
        return 1;
    }

    auto text = tokenizer.decode(tokens);
    if (!text.has_value()) {
        std::cerr << "Failed to decode tokens\n";
        return 1;
    }

    if (!textOut.empty()) {
        std::ofstream out(textOut);
        if (!out.is_open()) {
            std::cerr << "Failed to open output file " << textOut << "\n";
            return 1;
        }
        out << *text << "\n";
    } else {
        std::cout << *text << "\n";
    }

    return 0;
}

static int cmdInspect(int argc, char* argv[]) {
    std::string modelDir, token;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--model" && i + 1 < argc) {
            modelDir = argv[++i];
        } else if (arg == "--token" && i + 1 < argc) {
            token = argv[++i];
        }
    }

    if (modelDir.empty() || token.empty()) {
        printUsage();
        return 1;
    }

    forkenizer::ModelData data;
    if (!forkenizer::loadModel(modelDir, data)) {
        std::cerr << "Failed to load model\n";
        return 1;
    }

    auto it = data.tokenToId.find(token);
    if (it != data.tokenToId.end()) {
        std::cout << "Token: " << token << "\n";
        std::cout << "ID: " << it->second << "\n";
        std::cout << "Bytes: ";
        for (unsigned char byte : token) {
            std::cout << static_cast<int>(byte) << " ";
        }
        std::cout << "\n";
    } else {
        std::cout << "Token not found in vocabulary\n";
    }

    return 0;
}

static int cmdTokenizeFile(const std::string& modelDir, const std::string& inputFile, 
                          const std::string& outputFile = "") {
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << inputFile << "\n";
        return 1;
    }
    
    std::string text((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    file.close();
    
    forkenizer::Tokenizer tokenizer;
    if (!tokenizer.load(modelDir)) {
        std::cerr << "Failed to load model from " << modelDir << "\n";
        return 1;
    }
    
    auto tokens = tokenizer.encode(text);
    if (!tokens.has_value()) {
        std::cerr << "Failed to encode\n";
        return 1;
    }
    
    std::string outFile = outputFile;
    if (outFile.empty()) {
        size_t dotPos = inputFile.find_last_of('.');
        std::string baseName = (dotPos != std::string::npos && dotPos > 0) 
            ? inputFile.substr(0, dotPos) 
            : inputFile;
        outFile = baseName + "_tokenized.txt";
    }
    
    std::ofstream out(outFile);
    if (!out.is_open()) {
        std::cerr << "Failed to create " << outFile << "\n";
        return 1;
    }
    
    for (size_t i = 0; i < tokens->size(); ++i) {
        if (i > 0) out << " ";
        out << (*tokens)[i];
    }
    out << "\n";
    
    std::cout << "Created: " << outFile << "\n";
    return 0;
}

static int cmdDecodeFile(const std::string& modelDir, const std::string& inputFile) {
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << inputFile << "\n";
        return 1;
    }

    std::vector<uint32_t> tokens;
    uint32_t id;
    while (file >> id) {
        tokens.push_back(id);
    }
    file.close();

    forkenizer::Tokenizer tokenizer;
    if (!tokenizer.load(modelDir)) {
        std::cerr << "Failed to load model from " << modelDir << "\n";
        return 1;
    }

    auto text = tokenizer.decode(tokens);
    if (!text.has_value()) {
        std::cerr << "Failed to decode\n";
        return 1;
    }

    size_t dotPos = inputFile.find_last_of('.');
    std::string baseName = (dotPos != std::string::npos && dotPos > 0) 
        ? inputFile.substr(0, dotPos) 
        : inputFile;
    std::string outFile = baseName + "_decoded.txt";

    std::ofstream out(outFile);
    if (!out.is_open()) {
        std::cerr << "Failed to create " << outFile << "\n";
        return 1;
    }
    out << *text << "\n";

    std::cout << "Created: " << outFile << "\n";
    return 0;
}

static int cmdTrain(int argc, char* argv[]) {
    std::vector<std::string> corpusFiles;
    std::string outputDir;
    uint32_t vocabSize = 256;
    uint32_t numMerges = 32;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--corpus") {
            while (i + 1 < argc && argv[i + 1][0] != '-') {
                corpusFiles.push_back(argv[++i]);
            }
        } else if (arg == "--out" && i + 1 < argc) {
            outputDir = argv[++i];
        } else if (arg == "--vocab-size" && i + 1 < argc) {
            vocabSize = static_cast<uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--merges" && i + 1 < argc) {
            numMerges = static_cast<uint32_t>(std::stoul(argv[++i]));
        }
    }

    if (corpusFiles.empty() || outputDir.empty()) {
        printUsage();
        return 1;
    }

    forkenizer::Trainer trainer;
    if (!trainer.train(corpusFiles, outputDir, vocabSize, numMerges)) {
        std::cerr << "Training failed\n";
        return 1;
    }

    std::cout << "Training completed. Model saved to " << outputDir << "\n";
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    // Simple file tokenize: forkenizer-cli <file>
    if (fileExists(command) && argc == 2) {
        std::string modelDir = findDefaultModel();
        if (modelDir.empty()) {
            std::cerr << "No model found. Use: forkenizer-cli -m <model> <file>\n";
            return 1;
        }
        return cmdTokenizeFile(modelDir, command);
    }

    // Tokenize with model: forkenizer-cli -m <model> <file>
    if (command == "-m" && argc >= 4) {
        return cmdTokenizeFile(argv[2], argv[3]);
    }

    // Simple decode: forkenizer-cli decode <file>
    if (command == "decode" && argc == 3) {
        std::string modelDir = findDefaultModel();
        if (modelDir.empty()) {
            std::cerr << "No model found. Use full decode command.\n";
            return 1;
        }
        return cmdDecodeFile(modelDir, argv[2]);
    }

    // Simple train: forkenizer-cli train <corpus>...
    if (command == "train" && argc >= 3) {
        std::vector<std::string> files;
        for (int i = 2; i < argc; ++i) {
            files.push_back(argv[i]);
        }
        std::string modelDir = findDefaultModel();
        if (modelDir.empty()) {
            modelDir = "model";
        }
        forkenizer::Trainer trainer;
        if (!trainer.train(files, modelDir, 256, 32)) {
            std::cerr << "Training failed\n";
            return 1;
        }
        std::cout << "Training completed. Model saved to " << modelDir << "\n";
        return 0;
    }

    // Original commands still work
    if (command == "encode") {
        return cmdEncode(argc, argv);
    } else if (command == "decode") {
        return cmdDecode(argc, argv);
    } else if (command == "inspect") {
        return cmdInspect(argc, argv);
    } else if (command == "train") {
        return cmdTrain(argc, argv);
    }

    printUsage();
    return 1;
}


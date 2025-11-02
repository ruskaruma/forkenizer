#include "forkenizer/PreTokenizer.hpp"
#include <cctype>
#include <string>
#include <algorithm>

namespace forkenizer {

static bool isDigit(char c) { return std::isdigit(static_cast<unsigned char>(c)); }
static bool isLetter(char c) { return std::isalpha(static_cast<unsigned char>(c)); }
static bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
static bool isPunct(char c) { return std::ispunct(static_cast<unsigned char>(c)); }

static bool isMathOp(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || 
           c == '=' || c == '<' || c == '>' || c == '!' || c == '%';
}

std::vector<std::string> PreTokenizer::preTokenize(const std::string& utf8Text) const {
    std::vector<std::string> tokens;
    size_t i = 0;
    const size_t len = utf8Text.length();

    while (i < len) {
        if (isSpace(utf8Text[i])) {
            tokens.emplace_back(1, utf8Text[i]);
            ++i;
            continue;
        }

        if (isDigit(utf8Text[i]) || ((utf8Text[i] == '-' || utf8Text[i] == '+') && 
            i + 1 < len && isDigit(utf8Text[i + 1]))) {
            size_t start = i;
            bool hasDot = false;

            if (utf8Text[i] == '-' || utf8Text[i] == '+') {
                ++i;
            }

            while (i < len && isDigit(utf8Text[i])) {
                ++i;
            }

            if (i < len && utf8Text[i] == '.' && !hasDot) {
                hasDot = true;
                ++i;
                while (i < len && isDigit(utf8Text[i])) {
                    ++i;
                }
            }

            if (i < len && (utf8Text[i] == 'e' || utf8Text[i] == 'E')) {
                ++i;
                if (i < len && (utf8Text[i] == '-' || utf8Text[i] == '+')) {
                    ++i;
                }
                while (i < len && isDigit(utf8Text[i])) {
                    ++i;
                }
            }

            tokens.push_back(utf8Text.substr(start, i - start));
            continue;
        }

        if (isLetter(utf8Text[i]) || utf8Text[i] == '_') {
            size_t start = i;
            while (i < len && (isLetter(utf8Text[i]) || isDigit(utf8Text[i]) || 
                   utf8Text[i] == '_' || utf8Text[i] == '.')) {
                ++i;
            }
            tokens.push_back(utf8Text.substr(start, i - start));
            continue;
        }

        if (isMathOp(utf8Text[i])) {
            if (i + 1 < len) {
                std::string pair = utf8Text.substr(i, 2);
                if (pair == "<=" || pair == ">=" || pair == "!=" || pair == "->") {
                    tokens.push_back(pair);
                    i += 2;
                    continue;
                }
            }
            tokens.emplace_back(1, utf8Text[i]);
            ++i;
            continue;
        }

        if (utf8Text[i] == '(' || utf8Text[i] == ')' || 
            utf8Text[i] == '[' || utf8Text[i] == ']' ||
            utf8Text[i] == '{' || utf8Text[i] == '}') {
            tokens.emplace_back(1, utf8Text[i]);
            ++i;
            continue;
        }

        if (isPunct(utf8Text[i])) {
            tokens.emplace_back(1, utf8Text[i]);
            ++i;
            continue;
        }

        tokens.emplace_back(1, utf8Text[i]);
        ++i;
    }

    return tokens;
}

}


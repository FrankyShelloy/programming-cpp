#include "text_processor.h"
#include <cctype>

std::string TextProcessor::Process(const std::string& text) {
    std::string result = text;

    for (char &c : result) {
        if (std::ispunct(static_cast<unsigned char>(c))) {
            c = ' ';
        }
    }

    for (int i = 0; i < result.length(); i++) {
        result[i] = std::tolower(result[i]);
    }

    return result;
}
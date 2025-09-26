#include "word_counter.h"
#include <sstream>
#include <algorithm>

void WordCounter::CountWords(const std::string& text) {
    word_frequencies_.clear();
    std::istringstream stream(text);
    std::string word;

    while (stream >> word) {

        while (word.size() > 0 && ispunct(word[0])) {
            word = word.substr(1);
        }


        while (word.size() > 0 && ispunct(word[word.size() - 1])) {
            word.pop_back();
        }

        if (word.size() > 0) {
            word_frequencies_[word]++;
        }
    }
}

std::vector<std::pair<std::string, unsigned int>> WordCounter::GetSortedWords() {
    std::vector<std::pair<std::string, unsigned int>> result;


    for (const auto& pair : word_frequencies_) {
        result.push_back(pair);
    }


    std::sort(result.begin(), result.end(),
        [](const std::pair<std::string, unsigned int>& a,
           const std::pair<std::string, unsigned int>& b) {

            if (a.second != b.second) {
                return a.second > b.second;
            }

            return a.first < b.first;
        });

    return result;
}

int WordCounter::GetTotalWords() {
    int total = 0;
    for (const auto& pair : word_frequencies_) {
        total += pair.second;
    }
    return total;
}
#pragma once
#include <string>
#include <map>
#include <vector>

class WordCounter {
public:
    void CountWords(const std::string& text);
    std::vector<std::pair<std::string, unsigned int>> GetSortedWords();
    int GetTotalWords();

private:
    std::map<std::string, unsigned int> word_frequencies_;
};
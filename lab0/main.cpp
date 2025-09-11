#include <iostream>
#include <string>
#include <fstream>
#include <cctype>
#include <cstring>
#include <map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>


int valid_input(int argc);


int main(int argc, char *argv[]) {
    if (int i = valid_input(argc) == -1) {
        return -1;
    }

    std::ifstream input_file(argv[1]);

    if (!input_file.is_open()) {
        std::cout << "file not open" << std::endl;
        return -1;
    }

    std::string line;
    std::string full_line;

    if (input_file.is_open())
    {
        while (std::getline(input_file, line)) {
            full_line += line + "\n";
        };
    }
    input_file.close();

    for (char &c : full_line) {
        if (std::ispunct(static_cast<unsigned char>(c))) {
            c = ' ';
        }
    }

    for (int i = 0; i < full_line.length(); i++) {
        full_line[i] = std::tolower(full_line[i]);
    }




    std::map<std::string, unsigned int> set_list;

    std::istringstream input_string(full_line);
    std::string word;

    while (input_string >> word) {
        if (!word.empty()) {
            while (!word.empty() && std::ispunct(static_cast<unsigned char>(word.front()))) {
                word.erase(0, 1);
            }
            while (!word.empty() && std::ispunct(static_cast<unsigned char>(word.back()))) {
                word.pop_back();
            }

            if (!word.empty()) {
                set_list[word]++;
            }
        }
    }

    int word_count = 0;
    for (const auto &pair : set_list) {
        word_count += pair.second;
    }
    std::vector<std::pair<std::string, unsigned int>> sorted_words(set_list.begin(), set_list.end());

    std::sort(sorted_words.begin(), sorted_words.end(),
        [](const auto &a, const auto &b) {
            return a.second > b.second;
        });

    std::ofstream output("output.csv");

    output << "Word;Count;Procent" << std::endl;

    for (const auto &pair : sorted_words) {
        float procent = (word_count > 0) ?
        static_cast<float>(pair.second) / word_count * 100 : 0.0f;

        output << pair.first << ";" << pair.second << ";" << std::fixed << std::setprecision(2) << procent << "%" << std::endl;
    }
    output.close();
    return 0;


}

int valid_input(int argc) {
    if ( argc != 3) {
        std::cout << "invalid number of arguments" << std::endl;
        return -1;
    } else return 0;
}


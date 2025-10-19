#include <iostream>
#include <fstream>
#include "text_processor.h"
#include "word_counter.h"
#include "file_handler.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input.txt> <output.csv>" << std::endl;
        return 1;
    }

    try {
        std::ifstream input_file(argv[1]);
        if (!input_file.is_open()) {
            throw std::runtime_error("Cannot open file: " + std::string(argv[1]));
        }

        TextProcessor processor;
        WordCounter counter;
        std::string line;

        while (std::getline(input_file, line)) {
            std::string clean_line = processor.Process(line);
            counter.CountWords(clean_line);
        }
        input_file.close();


        auto sorted_words = counter.GetSortedWords();
        int total_words = counter.GetTotalWords();


        FileHandler::WriteCSV(argv[2], sorted_words, total_words);

        std::cout << "Done! Results saved to " << argv[2] << std::endl;

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
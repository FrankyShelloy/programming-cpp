#include <iostream>
#include "text_processor.h"
#include "word_counter.h"
#include "file_handler.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <input.txt> <output.csv>" << std::endl;
        return 1;
    }

    try {

        std::string content = FileHandler::ReadFile(argv[1]);

        TextProcessor processor;
        std::string clean_text = processor.Process(content);


        WordCounter counter;
        counter.CountWords(clean_text);


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
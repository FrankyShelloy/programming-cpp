#include "file_handler.h"
#include <fstream>
#include <iomanip>

std::string FileHandler::ReadFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    std::string content;

    while (std::getline(file, line)) {
        content += line + "\n";
    }

    file.close();
    return content;
}

void FileHandler::WriteCSV(const std::string& filename,
                          const std::vector<std::pair<std::string, unsigned int>>& words,
                          int total_count) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create file: " + filename);
    }

    file << "Word;Count;Procent" << std::endl;

    for (const auto& pair : words) {
        float percentage = (total_count > 0) ?
            static_cast<float>(pair.second) / total_count * 100 : 0.0f;

        file << pair.first << ";" << pair.second << ";"
             << std::fixed << std::setprecision(2) << percentage << "%" << std::endl;
    }

    file.close();
}
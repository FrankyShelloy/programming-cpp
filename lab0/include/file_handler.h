#pragma once
#include <string>
#include <vector>

class FileHandler {
public:
  static std::string ReadFile(const std::string& filename);
  static void WriteCSV(const std::string& filename,
                      const std::vector<std::pair<std::string, unsigned int>>& words,
                      int total_count);
};
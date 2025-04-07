#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <set>
#include <unordered_set>
namespace fs = std::filesystem;

class WordInDocument {
private:
    std::string documentName;
    std::vector<std::pair<std::string, int>> wordPositions;
    std::vector<std::string> lines;

public:
    WordInDocument() : documentName("") {}

    WordInDocument(const std::string& docName) : documentName(docName) {}

    void addWordPosition(const std::string& word, int position) {
        wordPositions.push_back({ word, position });
    }

    const std::string& getDocumentName() const {
        return documentName;
    }

    int getOccurrenceCount() const {
        return wordPositions.size();
    }
    void addLine(const std::string& line) {
        lines.push_back(line);
    }

    const std::vector<std::string>& getLines() const {
        return lines;
    }


    const std::vector<std::pair<std::string, int>>& getWordPositions() const {
        return wordPositions;
    }

};
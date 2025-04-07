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
#include <memory>
#include<functional>
#include <unordered_set>
namespace fs = std::filesystem;

struct TrieNode {
    std::unordered_map<char, std::shared_ptr<TrieNode>> children;
    std::vector<std::pair<std::string, int>> wordPositions; 
    bool isEndOfWord = false;
};

class TriesSearchEngine {
private:
    std::shared_ptr<TrieNode> root;
    std::unordered_map<std::string, std::vector<std::string>> index; 
    void insertWord(const std::string& word, const std::string& documentName, int position) {
        std::shared_ptr<TrieNode> node = root;
        for (char c : word) {
            if (!node->children[c]) {
                node->children[c] = std::make_shared<TrieNode>();
            }
            node = node->children[c];
        }
        node->isEndOfWord = true;
        node->wordPositions.emplace_back(documentName, position);
    }
    std::shared_ptr<TrieNode> findNode(const std::string& prefix) const {
        std::shared_ptr<TrieNode> node = root;
        for (char c : prefix) {
            if (!node->children.count(c)) {
                return nullptr;
            }
            node = node->children[c];
        }
        return node;
    }

public:
    TriesSearchEngine() {
        root = std::make_shared<TrieNode>();
    }
    std::string getLine(const std::string& documentName, int position) {
        if (index.find(documentName) != index.end() && position < index[documentName].size()) {
            return index[documentName][position]; // Return the line at the specified position
        }
        return ""; // Return an empty string if not found
    }
    void crawlFile(const std::string& fileName) {
        std::ifstream file(fileName);
        std::string line;
        int position = 0;

        if (file.is_open()) {
            while (std::getline(file, line)) {
                std::transform(line.begin(), line.end(), line.begin(), ::tolower);
                index[fileName].push_back(line); 

                std::istringstream stream(line);
                std::string word;

                while (stream >> word) {
                    word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
                    if (!word.empty()) {
                        insertWord(word, fileName, position++);
                    }
                }
            }
        }
        file.close();
    }
    void crawlFile1(const std::string& fileName) {
        std::ifstream file(fileName);
        std::string line;
        int position = 0;

        if (file.is_open()) {
            while (std::getline(file, line)) {
                std::transform(line.begin(), line.end(), line.begin(), ::tolower);
                std::istringstream stream(line);
                std::string word;

                while (stream >> word) {
                    word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
                    if (!word.empty()) {
                        insertWord(word, fileName, position++);
                    }
                }
            }
        }
        file.close();
    }
    void crawlNewFile(const std::string& content) {
        std::string newFileName = "new_file.txt"; 
        std::ofstream newFile(newFileName);
        if (newFile.is_open()) {
            newFile << content; 
            newFile.close();
            crawlFile(newFileName); 
        }
        else {
            std::cerr << "Error: Unable to create new file '" << newFileName << "'\n";
        }
    }
    void dumpSearchEngine(const std::string& dumpFileName) const {
        std::ofstream outFile(dumpFileName, std::ios::trunc);
        if (outFile.is_open()) {
            std::function<void(std::shared_ptr<TrieNode>, const std::string&)> dumpNode;
            dumpNode = [&](std::shared_ptr<TrieNode> node, const std::string& prefix) {
                if (node->isEndOfWord) {
                    outFile << prefix << "\n";
                    for (const auto& [doc, pos] : node->wordPositions) {
                        outFile << doc << " " << pos << "\n";
                    }
                }
                for (const auto& [ch, child] : node->children) {
                    dumpNode(child, prefix + ch);
                }
                };
            dumpNode(root, "");
        }
        outFile.close();
    }
    void loadSearchEngine(const std::string& dumpFileName) {
        std::ifstream inFile(dumpFileName);
        std::string line, word, documentName;
        int position;

        if (inFile.is_open()) {
            while (std::getline(inFile, line)) {
                if (line.empty()) continue;

                if (std::isdigit(line[0])) {
                    std::istringstream stream(line);
                    stream >> documentName >> position;
                    insertWord(word, documentName, position);
                }
                else {
                    word = line;
                }
            }
        }
        inFile.close();
    }
    void searchMultipleWords(const std::vector<std::string>& words) {
        std::unordered_map<std::string, int> docWordCount;
        std::unordered_map<std::string, int> totalOccurrences;
        std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> contextData;
        std::set<std::pair<std::string, int>> printedPositions; 

        for (const auto& word : words) {
            std::shared_ptr<TrieNode> node = findNode(word);
            if (node && node->isEndOfWord) {
                for (const auto& [docName, position] : node->wordPositions) {
                    docWordCount[docName]++;
                    totalOccurrences[docName]++;
                    contextData[docName].emplace_back(word, position);
                }
            }
        }

        std::vector<std::pair<std::string, int>> result;
        for (const auto& pair : totalOccurrences) {
            result.emplace_back(pair.first, pair.second);
        }

        std::sort(result.begin(), result.end(), [&](const auto& a, const auto& b) {
            if (docWordCount[a.first] == docWordCount[b.first]) {
                return a.second > b.second;
            }
            return docWordCount[a.first] > docWordCount[b.first];
            });

        for (const auto& doc : result) {
            std::cout << "Document: " << doc.first << " | Total Words Matched: " << docWordCount[doc.first]
                << " | Total Occurrences: " << doc.second << "\n";

            for (const auto& [word, position] : contextData[doc.first]) {
                displayContextFromTrie(word, doc.first, printedPositions);
            }
        }
    }
    void displayContextFromTrie(const std::string& targetWord, const std::string& documentName, std::set<std::pair<std::string, int>>& printedPositions) {
        std::shared_ptr<TrieNode> node = findNode(targetWord);
        if (!node || !node->isEndOfWord) {
            std::cout << "Word '" << targetWord << "' not found in document '" << documentName << "'.\n";
            return;
        }

        for (const auto& [docName, position] : node->wordPositions) {
            if (docName == documentName && printedPositions.find({ docName, position }) == printedPositions.end()) {
                
                std::ifstream file(documentName);
                if (!file.is_open()) {
                    std::cerr << "Error: Unable to open file '" << documentName << "'\n";
                    continue;
                }

                std::vector<std::string> words;
                std::string line;
                while (std::getline(file, line)) {
                    std::transform(line.begin(), line.end(), line.begin(), ::tolower);
                    std::istringstream stream(line);
                    std::string word;
                    while (stream >> word) {
                        word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
                        words.push_back(word);
                    }
                }
                file.close();

                
                int start = std::max(0, position - 5);
                int end = std::min(static_cast<int>(words.size()) - 1, position + 5);
                std::cout << "Context for word '" << targetWord << "' at position " << position
                    << " in document '" << documentName << "':\n";
                for (int i = start; i <= end; ++i) {
                    std::cout << words[i] << " ";
                }
                std::cout << "\n";

                printedPositions.insert({ docName, position });
            }
        }
    }
    void displayContext1(const std::string& line, const std::string& phrase, std::unordered_set<std::string>& printedLines) {
        
        if (printedLines.find(line) != printedLines.end()) {
            return;
        }
        std::string lowerLine = line;
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);
        size_t pos = lowerLine.find(phrase);
        if (pos != std::string::npos) {
           
            size_t start = (pos > 30) ? pos - 30 : 0;
            size_t end = std::min(pos + phrase.length() + 30, line.length());
            std::string context = "... " + line.substr(start, end - start) + " ...";

            std::cout << "Context: " << context << "\n";
            printedLines.insert(line);
        }
    }
    void searchSubtraction(const std::vector<std::string>& includeWords, const std::vector<std::string>& excludeWords) {
        std::unordered_map<std::string, int> includeDocCounts;
        std::unordered_map<std::string, int> excludeDocs;
        std::unordered_map<std::string, std::vector<std::string>> contexts;
        std::set<std::pair<std::string, int>> printedPositions;

        // Process included words
        for (const auto& word : includeWords) {
            std::shared_ptr<TrieNode> includeNode = findNode(word);
            if (includeNode && includeNode->isEndOfWord) {
                for (const auto& [docName, position] : includeNode->wordPositions) {
                    includeDocCounts[docName]++;
                    contexts[docName].push_back(word); // Store context
                }
            }
        }

        // Process excluded words
        for (const auto& word : excludeWords) {
            std::shared_ptr<TrieNode> excludeNode = findNode(word);
            if (excludeNode && excludeNode->isEndOfWord) {
                for (const auto& [docName, _] : excludeNode->wordPositions) {
                    excludeDocs[docName]++;
                }
            }
        }

        // Filter documents based on inclusion and exclusion
        std::unordered_map<std::string, int> finalDocCounts;
        for (const auto& [docName, count] : includeDocCounts) {
            if (excludeDocs.find(docName) == excludeDocs.end()) {
                finalDocCounts[docName] = count;
            }
        }

        // Sort the results
        std::vector<std::pair<std::string, int>> sortedResults(finalDocCounts.begin(), finalDocCounts.end());
        std::sort(sortedResults.begin(), sortedResults.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second; // Sort by occurrences
            });

        // Display the results
        std::cout << "Documents matching the inclusion group but excluding the exclusion group:\n";
        for (const auto& result : sortedResults) {
            std::cout << "Document: " << result.first
                << " | Matching Words Count: " << result.second << "\n";
            for (const auto& word : contexts[result.first]) {
                displayContextFromTrie(word, result.first, printedPositions);
            }
        }
    }
    void searchExactPhrase(const std::string& phrase) {
        
        std::string lowerPhrase = phrase;
        std::transform(lowerPhrase.begin(), lowerPhrase.end(), lowerPhrase.begin(), ::tolower);

        std::unordered_map<std::string, std::vector<int>> phraseOccurrences; 
        std::unordered_map<std::string, int> docOccurrences; 

        for (const auto& [docName, lines] : index) {
            for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
                std::string lowerLine = lines[lineIndex];
                std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);

                size_t pos = lowerLine.find(lowerPhrase);
                while (pos != std::string::npos) {   
                    if ((pos == 0 || !isalnum(lowerLine[pos - 1])) &&
                        (pos + lowerPhrase.size() == lowerLine.size() || !isalnum(lowerLine[pos + lowerPhrase.size()])))
                    {
                        docOccurrences[docName]++;
                        phraseOccurrences[docName].push_back(lineIndex); 
                    }
                    pos = lowerLine.find(lowerPhrase, pos + 1);
                }
            }
        }

        std::vector<std::pair<std::string, int>> sortedResults(docOccurrences.begin(), docOccurrences.end());
        std::sort(sortedResults.begin(), sortedResults.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
            });

       
        for (const auto& [docName, totalOccurrences] : sortedResults) {
            std::cout << "Document: " << docName << " | Total Occurrences: " << totalOccurrences << "\n";

           
            std::unordered_set<std::string> printedLines;
            for (int lineIndex : phraseOccurrences[docName]) {
                std::string line = getLine(docName, lineIndex); 
                displayContext1(line, lowerPhrase, printedLines); 
            }
        }
    }
    void searchSentenceSubtraction(const std::string& includeSentence, const std::string& excludeSentence) {
        std::cout << "Performing sentence subtraction search: Include = \"" << includeSentence
            << "\", Exclude = \"" << excludeSentence << "\"\n";

        std::string lowerInclude = includeSentence, lowerExclude = excludeSentence;
        std::transform(lowerInclude.begin(), lowerInclude.end(), lowerInclude.begin(), ::tolower);
        std::transform(lowerExclude.begin(), lowerExclude.end(), lowerExclude.begin(), ::tolower);

        std::unordered_map<std::string, std::vector<int>> includeOccurrences;
        std::unordered_map<std::string, int> docIncludeCounts;

        for (const auto& [docName, lines] : index) {
            for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
                std::string lowerLine = lines[lineIndex];
                std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);

                size_t pos = lowerLine.find(lowerInclude);
                while (pos != std::string::npos) {
                    if ((pos == 0 || !isalnum(lowerLine[pos - 1])) &&
                        (pos + lowerInclude.size() == lowerLine.size() || !isalnum(lowerLine[pos + lowerInclude.size()]))) {
                        docIncludeCounts[docName]++;
                        includeOccurrences[docName].push_back(lineIndex);
                    }
                    pos = lowerLine.find(lowerInclude, pos + 1);
                }
            }
        }

        std::unordered_set<std::string> excludeDocs;
        for (const auto& [docName, lines] : index) {
            for (const auto& line : lines) {
                std::string lowerLine = line;
                std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);

                size_t pos = lowerLine.find(lowerExclude);
                if (pos != std::string::npos &&
                    (pos == 0 || !isalnum(lowerLine[pos - 1])) &&
                    (pos + lowerExclude.size() == lowerLine.size() || !isalnum(lowerLine[pos + lowerExclude.size()]))) {
                    excludeDocs.insert(docName);
                    break;
                }
            }
        }

        std::unordered_map<std::string, int> finalDocCounts;
        for (const auto& [docName, count] : docIncludeCounts) {
            if (excludeDocs.find(docName) == excludeDocs.end()) {
                finalDocCounts[docName] = count;
            }
        }

        std::vector<std::pair<std::string, int>> sortedResults(finalDocCounts.begin(), finalDocCounts.end());
        std::sort(sortedResults.begin(), sortedResults.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
            });

        for (const auto& [docName, totalOccurrences] : sortedResults) {
            std::cout << "Document: " << docName << " | Total Occurrences: " << totalOccurrences << "\n";
            std::unordered_set<std::string> printedLines;
            for (int lineIndex : includeOccurrences[docName]) {
                std::string line = getLine(docName, lineIndex);
                displayContext1(line, lowerInclude, printedLines);
            }
        }
    }
    void searchQuery(const std::string& query) {
        std::cout << "Received query: " << query << "\n";

        size_t minusPos = query.find(" - ");
        if (query.front() == '"' && minusPos != std::string::npos && query.back() == '"') {
            // Sentence subtraction search
            std::string includeSentence = query.substr(1, minusPos - 2);
            std::string excludeSentence = query.substr(minusPos + 3, query.size() - minusPos - 4);
            searchSentenceSubtraction(includeSentence, excludeSentence);
        }
        else if (minusPos != std::string::npos) {
            // Word subtraction search
            std::string includeWordsPart = query.substr(0, minusPos);
            std::string excludeWordsPart = query.substr(minusPos + 3);

            std::vector<std::string> includeWords, excludeWords;
            std::istringstream includeStream(includeWordsPart);
            std::string word;
            while (includeStream >> word) {
                includeWords.push_back(word);
            }
            std::istringstream excludeStream(excludeWordsPart);
            while (excludeStream >> word) {
                excludeWords.push_back(word);
            }

            searchSubtraction(includeWords, excludeWords);
        }
        else if (query.front() == '"' && query.back() == '"') {
            // Exact phrase search
            std::string phrase = query.substr(1, query.size() - 2);
            searchExactPhrase(phrase);
        }
        else {
            // Word search
            std::vector<std::string> words;
            std::istringstream stream(query);
            std::string word;
            while (stream >> word) {
                words.push_back(word);
            }
            searchMultipleWords(words);
        }
    }

};

#include "bscs23058wordindoc.h"


class SearchEngine {
private:
    std::unordered_map<std::string, std::vector<WordInDocument>> index;
    std::set<std::string> displayedContexts;

public:
    void crawlFile(const std::string& fileName) {
        std::ifstream file(fileName);
        std::string line;
        int position = 0;

        if (file.is_open()) {
            while (std::getline(file, line)) {
                std::string originalLine = line;
                std::transform(line.begin(), line.end(), line.begin(), ::tolower);

                std::istringstream stream(line);
                std::string word;

                while (stream >> word) {
                    word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
                    if (!word.empty()) {
                        if (index.find(word) == index.end()) {
                            index[word] = std::vector<WordInDocument>();
                        }

                        auto& docs = index[word];
                        bool found = false;

                        for (auto& doc : docs) {
                            if (doc.getDocumentName() == fileName) {
                                doc.addWordPosition(word, position);
                                found = true;
                                break;
                            }
                        }

                        if (!found) {
                            WordInDocument newDoc(fileName);
                            newDoc.addWordPosition(word, position);
                            newDoc.addLine(originalLine);
                            docs.push_back(newDoc);
                        }
                        position++;
                    }
                }
            }
        }
        file.close();
    }
    void dumpSearchEngine(const std::string& dumpFileName) {
        std::ofstream outFile(dumpFileName, std::ios::trunc);
        if (outFile.is_open()) {
            for (const auto& pair : index) {
                const std::string& word = pair.first;
                const std::vector<WordInDocument>& docs = pair.second;

                outFile << word << "\n";
                for (const auto& doc : docs) {
                    outFile << "Occurrences in " << doc.getDocumentName() << ": ";
                    const auto& wordPositions = doc.getWordPositions();
                    for (size_t i = 0; i < wordPositions.size(); ++i) {
                        outFile << "(" << wordPositions[i].first << ", " << wordPositions[i].second << ")";
                        if (i != wordPositions.size() - 1) outFile << ", ";
                    }
                    outFile << "\n";
                }
                outFile << "\n";
            }
        }
        outFile.close();
    }
    void loadSearchEngine(const std::string& dumpFileName) {
        std::ifstream inFile(dumpFileName);
        std::string line;
        std::string currentWord;
        std::string documentName;
        std::string word;
        int position;

        if (inFile.is_open()) {
            while (getline(inFile, line)) {
                if (line.empty()) continue;
                if (line.find("Occurrences in") == std::string::npos) {
                    currentWord = line;
                    continue;
                }
                std::istringstream stream(line);
                stream >> documentName;
                stream >> documentName;
                std::getline(stream, documentName);

                if (index.find(currentWord) == index.end()) {
                    index[currentWord] = std::vector<WordInDocument>();
                }
                WordInDocument doc(documentName);
                while (stream >> word >> position) {
                    doc.addWordPosition(word, position);
                }
                index[currentWord].push_back(doc);
            }
        }
        inFile.close();
    }
    
    void crawlNewFile(const std::string& content) {
        std::string newFileName = "new_file.txt"; 
        std::ofstream newFile(newFileName);
        if (newFile.is_open()) {
            newFile << content; 
            newFile.close();
            crawlFile(newFileName); 
        }
    }
    void displayContexta(const std::string& targetWord, int position, const std::string& documentName, std::set<std::pair<std::string, int>>& printedPositions) {
        std::ifstream file(documentName);
        std::string line;
        std::vector<std::string> words;

        if (file.is_open()) {
            while (std::getline(file, line)) {
                std::transform(line.begin(), line.end(), line.begin(), ::tolower);
                std::istringstream stream(line);
                std::string word;
                while (stream >> word) {
                    word.erase(std::remove_if(word.begin(), word.end(), ::ispunct), word.end());
                    words.push_back(word);
                }
            }
        }
        file.close();

        std::string lowerTargetWord = targetWord;
        std::transform(lowerTargetWord.begin(), lowerTargetWord.end(), lowerTargetWord.begin(), ::tolower);

        for (size_t i = 0; i < words.size(); ++i) {
            if (words[i] == lowerTargetWord) {
                if (printedPositions.find({ documentName, i }) != printedPositions.end()) {
                    continue; 
                }

                std::cout << "Context for word '" << targetWord << "' at position " << i << " in document '" << documentName << "':\n";

                int start = std::max(0, int(i) - 5);
                int end = std::min(int(words.size()) - 1, int(i) + 5);
                for (int j = start; j <= end; ++j) {
                    std::cout << words[j] << " ";
                }
                std::cout << "\n";

                printedPositions.insert({ documentName, i });
            }
        }
    }
    void searchSubtraction1(const std::string& word1, const std::string& word2) {
        auto it1 = index.find(word1);
        auto it2 = index.find(word2);

        if (it1 != index.end()) {
            std::set<std::string> resultDocs;
            std::unordered_map<std::string, int> docOccurrences;

            for (const auto& doc : it1->second) {
                resultDocs.insert(doc.getDocumentName());
                docOccurrences[doc.getDocumentName()] = doc.getOccurrenceCount();

                if (it2 != index.end()) {
                    for (const auto& wordDoc : it2->second) {
                        if (wordDoc.getDocumentName() == doc.getDocumentName()) {
                            resultDocs.erase(doc.getDocumentName());
                            break;
                        }
                    }
                }
            }

            std::vector<std::pair<std::string, int>> sortedResults;
            for (const auto& docName : resultDocs) {
                sortedResults.push_back({ docName, docOccurrences[docName] });
            }

            std::sort(sortedResults.begin(), sortedResults.end(),
                [](const auto& a, const auto& b) {
                    return a.second > b.second;
                });

            std::cout << "Documents containing '" << word1 << "' but not '" << word2 << "':\n";
            for (const auto& result : sortedResults) {
                std::cout << "Document: " << result.first << " | Occurrences of " << word1 << ": " << result.second << "\n";

                std::set<std::pair<std::string, int>> printedPositions; 
                for (const auto& doc : it1->second) {
                    if (doc.getDocumentName() == result.first) {
                        for (const auto& [word, position] : doc.getWordPositions()) {
                            if (word == word1) {
                                displayContexta(word1, position, doc.getDocumentName(), printedPositions);
                            }
                        }
                    }
                }
            }
        }
        else {
            std::cout << "No documents found for word: " << word1 << "\n";
        }
    }
    void displayContext(const std::string& targetWord, int position, const std::string& documentName, std::set<std::pair<std::string, int>>& printedPositions) {
        std::ifstream file(documentName);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file '" << documentName << "'\n";
            return;
        }

        std::string line;
        std::vector<std::string> words;

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

        std::string lowerTargetWord = targetWord;
        std::transform(lowerTargetWord.begin(), lowerTargetWord.end(), lowerTargetWord.begin(), ::tolower);

        for (size_t i = 0; i < words.size(); ++i) {
            if (words[i] == lowerTargetWord) {
                if (printedPositions.find({ documentName, i }) != printedPositions.end()) {
                    continue;
                }

                int start = std::max(0, int(i) - 5);
                int end = std::min(int(words.size()) - 1, int(i) + 5);

                std::cout << "Context for word '" << targetWord << "' at position " << i
                    << " in document '" << documentName << "':\n";
                for (int j = start; j <= end; ++j) {
                    std::cout << words[j] << " ";
                }
                std::cout << "\n";

                printedPositions.insert({ documentName, i }); 
            }
        }
    }
    void searchSubtraction(const std::vector<std::string>& includeWords, const std::vector<std::string>& excludeWords) {
        std::unordered_map<std::string, int> docOccurrences;
        std::set<std::string> includedDocs;

        // Step 1: Process inclusion group
        for (const auto& word : includeWords) {
            auto it = index.find(word);
            if (it != index.end()) {
                for (const auto& doc : it->second) {
                    includedDocs.insert(doc.getDocumentName());
                    docOccurrences[doc.getDocumentName()] += doc.getOccurrenceCount();
                }
            }
        }

        // Step 2: Process exclusion group
        for (const auto& word : excludeWords) {
            auto it = index.find(word);
            if (it != index.end()) {
                for (const auto& doc : it->second) {
                    includedDocs.erase(doc.getDocumentName());
                }
            }
        }

        // Step 3: Sort results
        std::vector<std::pair<std::string, int>> sortedResults;
        for (const auto& docName : includedDocs) {
            sortedResults.push_back({ docName, docOccurrences[docName] });
        }
        std::sort(sortedResults.begin(), sortedResults.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
            });

        // Step 4: Display results with context
        std::cout << "Documents containing ";
        for (const auto& word : includeWords) std::cout << "'" << word << "' ";
        std::cout << "but not ";
        for (const auto& word : excludeWords) std::cout << "'" << word << "' ";
        std::cout << ":\n";

        std::set<std::pair<std::string, int>> printedPositions;
        for (const auto& result : sortedResults) {
            std::cout << "Document: " << result.first << " | Total Occurrences: " << result.second << "\n";

            for (const auto& word : includeWords) {
                auto it = index.find(word);
                if (it != index.end()) {
                    for (const auto& doc : it->second) {
                        if (doc.getDocumentName() == result.first) {
                            for (const auto& [word, position] : doc.getWordPositions()) {
                                if (word == word) {
                                    displayContexta(word, position, doc.getDocumentName(), printedPositions);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    void searchSentenceSubtraction(const std::string& includeSentence, const std::string& excludeSentence) {
        std::string lowerIncludeSentence = includeSentence;
        std::string lowerExcludeSentence = excludeSentence;

        std::transform(lowerIncludeSentence.begin(), lowerIncludeSentence.end(), lowerIncludeSentence.begin(), ::tolower);
        std::transform(lowerExcludeSentence.begin(), lowerExcludeSentence.end(), lowerExcludeSentence.begin(), ::tolower);

        std::unordered_map<std::string, std::vector<std::string>> docContexts;
        std::unordered_map<std::string, int> docOccurrences;

        for (const auto& [word, docs] : index) {
            for (const auto& doc : docs) {
                bool includeFound = false;
                bool excludeFound = false;
                for (const auto& line : doc.getLines()) {
                    std::string lowerLine = line;
                    std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);

                    if (lowerLine.find(lowerIncludeSentence) != std::string::npos) {
                        includeFound = true;
                        docOccurrences[doc.getDocumentName()]++;
                        docContexts[doc.getDocumentName()].push_back(line);
                    }

                    if (lowerLine.find(lowerExcludeSentence) != std::string::npos) {
                        excludeFound = true;
                    }

                    // If the document contains the exclude sentence, skip it
                    if (excludeFound) {
                        docContexts[doc.getDocumentName()].clear();
                        docOccurrences.erase(doc.getDocumentName());
                        break;
                    }
                }
            }
        }

        // Sort results by occurrences
        std::vector<std::pair<std::string, int>> sortedResults;
        for (const auto& doc : docOccurrences) {
            sortedResults.push_back(doc);
        }

        std::sort(sortedResults.begin(), sortedResults.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

        // Display results
        for (const auto& result : sortedResults) {
            std::cout << "Document: " << result.first << "\n";

            std::unordered_set<std::string> printedLines;
            for (const auto& line : docContexts[result.first]) {
                displayContext1(line, lowerIncludeSentence, printedLines);
            }
        }
    }

    void searchMultipleWords(const std::vector<std::string>& words) {
        std::unordered_map<std::string, int> docWordCount;
        std::unordered_map<std::string, int> totalOccurrences;
        std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> contextData;

        std::set<std::pair<std::string, int>> printedPositions;

        for (const auto& word : words) {
            auto it = index.find(word);
            if (it != index.end()) {
                for (const auto& doc : it->second) {
                    docWordCount[doc.getDocumentName()]++;
                    totalOccurrences[doc.getDocumentName()] += doc.getOccurrenceCount();

                    for (const auto& [word, position] : doc.getWordPositions()) {
                        contextData[doc.getDocumentName()].emplace_back(word, position);
                    }
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
                displayContext(word, position, doc.first, printedPositions); 
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
    void searchExactPhrase(const std::string& phrase) {
        std::string lowerPhrase = phrase;
        std::transform(lowerPhrase.begin(), lowerPhrase.end(), lowerPhrase.begin(), ::tolower);

        std::unordered_map<std::string, std::vector<std::string>> docContexts; 
        std::unordered_map<std::string, int> docOccurrences;

        for (const auto& [word, docs] : index) {
            for (const auto& doc : docs) {
                for (const auto& line : doc.getLines()) {
                    std::string lowerLine = line;
                    std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);

                    size_t pos = lowerLine.find(lowerPhrase);
                    while (pos != std::string::npos) {
                        if ((pos == 0 || !isalnum(lowerLine[pos - 1])) &&
                            (pos + lowerPhrase.size() == lowerLine.size() || !isalnum(lowerLine[pos + lowerPhrase.size()])))
                        {
                            docOccurrences[doc.getDocumentName()]++;
                            docContexts[doc.getDocumentName()].push_back(line);
                        }
                        pos = lowerLine.find(lowerPhrase, pos + 1);
                    }
                }
            }
        }

        std::vector<std::pair<std::string, int>> sortedResults;
        for (const auto& doc : docOccurrences) {
            sortedResults.push_back(doc);
        }

        std::sort(sortedResults.begin(), sortedResults.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

        for (const auto& result : sortedResults) {
            std::cout << "Document: " << result.first << "\n";

            std::unordered_set<std::string> printedLines;
            for (const auto& line : docContexts[result.first]) {
                displayContext1(line, lowerPhrase, printedLines);
            }
        }
    }
    void searchQuery1(const std::string& query) {
        std::cout << "Received query: " << query << "\n";

        size_t minusPos = query.find(" - ");
        if (minusPos != std::string::npos) {
            std::string includeGroup = query.substr(0, minusPos);
            std::string excludeGroup = query.substr(minusPos + 3);

            std::vector<std::string> includeWords;
            std::vector<std::string> excludeWords;

            std::istringstream includeStream(includeGroup);
            std::istringstream excludeStream(excludeGroup);

            std::string word;

            while (includeStream >> word) {
                includeWords.push_back(word);
            }

            while (excludeStream >> word) {
                excludeWords.push_back(word);
            }

            std::cout << "Performing subtraction search with include group: ";
            for (const auto& w : includeWords) std::cout << "'" << w << "' ";
            std::cout << "and exclude group: ";
            for (const auto& w : excludeWords) std::cout << "'" << w << "' ";
            std::cout << "\n";

         
            searchSubtraction(includeWords, excludeWords);
        }
        else if (query.front() == '"' && query.back() == '"')
        {

            std::string phrase = query.substr(1, query.size() - 2);
            std::cout << "Performing exact phrase search for: " << phrase << "\n";
            searchExactPhrase(phrase);
        }
        else {
            std::vector<std::string> words;
            std::istringstream stream(query);
            std::string word;
            while (stream >> word) {
                words.push_back(word);
            }
            std::cout << "Performing word search for: ";
            for (const auto& w : words) std::cout << w << " ";
            std::cout << "\n";
            searchMultipleWords(words);
        }
    }
    void searchQuery(const std::string& query) {
        std::cout << "Received query: " << query << "\n";

        size_t minusPos = query.find(" - ");
        if (minusPos != std::string::npos) {
            std::string firstPart = query.substr(0, minusPos);
            std::string secondPart = query.substr(minusPos + 3);

            if (firstPart.front() == '"' && firstPart.back() == '"' &&
                secondPart.front() == '"' && secondPart.back() == '"') {
              
                std::string includeSentence = firstPart.substr(1, firstPart.size() - 2);
                std::string excludeSentence = secondPart.substr(1, secondPart.size() - 2);

                std::cout << "Performing sentence subtraction search for include: \""
                    << includeSentence << "\" and exclude: \"" << excludeSentence << "\"\n";

                searchSentenceSubtraction(includeSentence, excludeSentence);
            }
            else {
              
                std::string includeGroup = firstPart;
                std::string excludeGroup = secondPart;

                std::vector<std::string> includeWords;
                std::vector<std::string> excludeWords;

                std::istringstream includeStream(includeGroup);
                std::istringstream excludeStream(excludeGroup);

                std::string word;

                while (includeStream >> word) {
                    includeWords.push_back(word);
                }

                while (excludeStream >> word) {
                    excludeWords.push_back(word);
                }

                std::cout << "Performing subtraction search with include group: ";
                for (const auto& w : includeWords) std::cout << "'" << w << "' ";
                std::cout << "and exclude group: ";
                for (const auto& w : excludeWords) std::cout << "'" << w << "' ";
                std::cout << "\n";

                searchSubtraction(includeWords, excludeWords);
            }
        }
        else if (query.front() == '"' && query.back() == '"') {
           
            std::string phrase = query.substr(1, query.size() - 2);
            std::cout << "Performing exact phrase search for: " << phrase << "\n";
            searchExactPhrase(phrase);
        }
        else {
          
            std::vector<std::string> words;
            std::istringstream stream(query);
            std::string word;
            while (stream >> word) {
                words.push_back(word);
            }
            std::cout << "Performing word search for: ";
            for (const auto& w : words) std::cout << w << " ";
            std::cout << "\n";
            searchMultipleWords(words);
        }
    }

};

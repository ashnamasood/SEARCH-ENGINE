#include "bscs23058tries.h" 
#include "bscs23058hash.h"
#include <iostream>
#include <cstdlib>
namespace fs = std::filesystem;

int main()
{
    int fileCountLimit = 50;

    std::cout << "========================================\n";
    std::cout << "         WELCOME TO SEARCH ENGINE       \n";
    std::cout << "========================================\n";
    std::cout << "Choose an option to proceed:\n";
    std::cout << "  1. Use Hash\n";
    std::cout << "  2. Use Tries\n";
    std::cout << "----------------------------------------\n";
    int num;
    std::cin >> num;
    std::cin.ignore();
    system("cls");

    

    if (num == 1) {
        SearchEngine searchEngine;
        std::string reviewFolder = "C:\\Users\\ICT\\source\\repos\\TEST\\Project3\\Project3\\review_text";
        std::string dumpFileName = "search_engine_dump.txt";

        if (fileCountLimit > 50) {
            fileCountLimit = 50; 
        }

        int fileCount = 0;
        for (const auto& entry : fs::directory_iterator(reviewFolder)) {
            if (fileCount >= fileCountLimit) break;
            if (entry.is_regular_file()) {
                searchEngine.crawlFile(entry.path().string());
                fileCount++;
            }
        }
        searchEngine.dumpSearchEngine(dumpFileName);
        searchEngine.loadSearchEngine(dumpFileName);

        std::string query;
        while (true)
        {
            std::cout << "Enter a search query (or type 'exit' to quit or ~1 for new file): ";
            std::getline(std::cin, query);

            if (query == "exit") break;
            if (query == "~1") {
                std::string newFileContent;
                std::cout << "Enter the content for the new file: ";
                std::getline(std::cin, newFileContent);
                searchEngine.crawlNewFile(newFileContent);
                std::cout << "New file created and added successfully.\n";
                continue;
            }
            system("cls");
            searchEngine.searchQuery(query);
        }
    }
    else if (num == 2)
    {
        TriesSearchEngine triesEngine;
        std::string reviewFolder = "C:\\Users\\ICT\\source\\repos\\TEST\\Project3\\Project3\\review_text";
        std::string dumpFileName = "tries_engine_dump.txt";

        if (fileCountLimit > 50) {
            fileCountLimit = 50; 
        }

        int fileCount = 0;
        for (const auto& entry : fs::directory_iterator(reviewFolder)) {
            if (fileCount >= fileCountLimit) break;
            if (entry.is_regular_file()) {
                triesEngine.crawlFile(entry.path().string());
                fileCount++;
            }
        }
        triesEngine.dumpSearchEngine(dumpFileName);
        triesEngine.loadSearchEngine(dumpFileName);

        std::string query;
        while (true) {
            std::cout << "Enter a search query (or type 'exit' to quit or ~1 for new file): ";
            std::getline(std::cin, query);

            if (query == "exit") break;
            if (query == "~1") {
                std::string newFileContent;
                std::cout << "Enter the content for the new file: ";
                std::getline(std::cin, newFileContent);
                triesEngine.crawlNewFile(newFileContent);
                std::cout << "New file created and added successfully.\n";
                continue;
            }
            system("cls");
            triesEngine.searchQuery(query);
        }
    }
    else
    {
        std::cout << "Invalid option! Please restart and choose a valid option.\n";
    }

    return 0;
}

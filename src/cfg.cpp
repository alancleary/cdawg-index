#include <fstream>
#include <cstdint>
#include "cdawg-index/cfg.hpp"
#include <vector>
#include <array>
#include <list>
#include <iostream>
#include <fstream>

namespace cdawg_index {

// construction

    CFG::CFG() { }

// destruction

    CFG::~CFG() {
        for (int i = 0; i < rulesSize; i++) {
            delete[] rules[i];
        }
        delete[] rules;
    }

// construction from MR-Repair grammar

    CFG* CFG::fromMrRepairFile(std::string filename)
    {
        CFG* cfg = new CFG();

        std::ifstream reader(filename);
        std::string line;

        // read grammar specs
        std::getline(reader, line);
        cfg->textLength = std::stoi(line);
        std::getline(reader, line);
        cfg->numRules = std::stoi(line);
        std::getline(reader, line);
        cfg->startSize = std::stoi(line);
        cfg->rulesSize = 0;

        // prepare to read grammar
        cfg->startRule = cfg->numRules + CFG::MR_REPAIR_CHAR_SIZE;
        int rulesSize = cfg->startRule + 1;  // +1 for start rule
        cfg->rules = new int*[rulesSize];
        int* ruleSizes = new int[rulesSize - 1];
        for (int i = 0; i < CFG::MR_REPAIR_CHAR_SIZE; i++) {
            ruleSizes[i] = 1;
        }
        cfg->rules[cfg->startRule] = new int[cfg->startSize + 1];  // +1 for the dummy code
        int i, j, c, ruleLength;

        // read rules in order they were added to grammar, i.e. line-by-line
        for (i = CFG::MR_REPAIR_CHAR_SIZE; i < cfg->startRule; i++) {
            for (j = 0; ;j++) {
                std::getline(reader, line);
                c = std::stoi(line);
                // use start rule as a buffer
                cfg->rules[cfg->startRule][j] = c;
                if (c == CFG::MR_REPAIR_DUMMY_CODE) {
                    break;
                }
                ruleSizes[i] += ruleSizes[c];
            }
            ruleLength = j;
            cfg->rulesSize += ruleLength;
            cfg->rules[i] = new int[ruleLength + 1];
            for (j = 0; j < ruleLength + 1; j++) {
                cfg->rules[i][j] = cfg->rules[cfg->startRule][j];
            }
        }

        // read start rule
        uint32_t pos = 0;
        uint8_t* key = new uint8_t[CFG::KEY_LENGTH];
        int len;
        for (i = 0; i < cfg->startSize; i++) {
            // get the (non-)terminal character
            std::getline(reader, line);
            c = std::stoi(line);
            cfg->rules[cfg->startRule][i] = c;
            pos += ruleSizes[c];
        }
        cfg->rules[cfg->startRule][i] = CFG::MR_REPAIR_DUMMY_CODE;

        // clean up
        delete[] ruleSizes;

        return cfg;
    }

// construction from Navarro grammar

    CFG* CFG::fromNavarroFiles(std::string filenameC, std::string filenameR) {

        // First loads the data from the binary .R and .C files. After storing all the available data, this will write the data to
        // an output file that matches the output that MR-Repair takes (- the original text length). In more detail, the format will
        // be the number of rules, the compressed text length, and then rules separated by a -1. Rules start at 256.

        // Load R file with grammar information
        std::ifstream binaryFileR(filenameR, std::ios::binary);

        char byte;
        int alphSize;
        int numOfRules = 0;
        int positionIdx;

        // First int is alphabet size (terminals) followed by 3 0's
        for (int i = 0; i < 4 && binaryFileR.read(&byte, sizeof(char)); ++i) {
            unsigned int decimalValue = static_cast<unsigned char>(byte);

            if (i == 0) {
                alphSize = decimalValue;
                positionIdx = alphSize;
            }
        }

        int alphTerminals[alphSize];

        // Save the terminals in an array
        for (int i = 0; i < alphSize && binaryFileR.read(&byte, sizeof(char)); ++i) {
            unsigned int decimalValue = static_cast<unsigned char>(byte);
            alphTerminals[i] = decimalValue;
        }


        // Array of arrays
        std::vector<std::list<std::array<int, 4>>> vectorOfLists;
        std::vector<std::list<std::array<int, 4>>> vectorOfRules;


        while (!binaryFileR.eof()) {
            std::list<std::array<int, 4>> innerList;

            // Each rule is made up of two groups of four ints, if Rule 0 was (2, 0, 0, 0) (3, 0, 0, 0) that refers to the 2nd and 3rd position terminals
            // if it was something liked Rule 76: (2, 44, 0, 0), that would be a nested rule made up of the 3rd position terminal + whatever is in Rule 44.
            for (int rule = 0; rule < 2; ++rule) {
                std::array<int, 4> innerArray;

                // Read integers and put values in the inner array
                for (int j = 0; j < 4 && binaryFileR.read(&byte, sizeof(char)); ++j) {
                    unsigned int decimalValue = static_cast<unsigned char>(byte);
                    innerArray[j] = decimalValue;
                }

                // Add the inner array to the inner list
                innerList.push_back(innerArray);
            }

            // Add the inner list to the outer vector
            vectorOfLists.push_back(innerList);
            numOfRules++;
            positionIdx++;
        }
        binaryFileR.close();

        std::vector<std::list<std::array<int, 4>>> vectorContainingGrammar;

        std::ifstream binaryFileC(filenameC, std::ios::binary);

        while (!binaryFileC.eof()) {
            std::list<std::array<int, 4>> innerList2;

            // Same as before but we are loading the grammar from .C into this vector
            for (int rule = 0; rule < 2; ++rule) {
                std::array<int, 4> innerArray2;

                // Read integers and put values in the inner array
                for (int j = 0; j < 4 && binaryFileC.read(&byte, sizeof(char)); ++j) {
                    unsigned int decimalValue = static_cast<unsigned char>(byte);
                    innerArray2[j] = decimalValue;
                }

                // Add the inner array to the inner list
                innerList2.push_back(innerArray2);
            }

            // Add the inner list to the outer vector
            vectorContainingGrammar.push_back(innerList2);
        }
        binaryFileC.close();

        // Write to a file
        std::ofstream outputFile("navarroOutputFile.txt");
        // Redirect std::cout to the output file
        std::streambuf *original = std::cout.rdbuf(outputFile.rdbuf());

        std::cout << numOfRules << "\n";
        std::cout << vectorContainingGrammar.size() << "\n";

        // PRINTING

        for (size_t i = 0; i < vectorOfLists.size(); ++i) {
            // Print both arrays (inside the list entry / rule) of 4 integers
            for (const auto &innerArray: vectorOfLists[i]) {
                int first = innerArray[0];
                int second = innerArray[1];
                int third = innerArray[2];
                int fourth = innerArray[3];

                if (first < alphSize) {
                    first = alphTerminals[first];
                    std::cout << first << " \n";
                } else {
                    first = first - alphSize + 256;
                    std::cout << first << " \n";
                }

                if (second < alphSize && second != 0) {
                    second = alphTerminals[second];
                    std::cout << second << " \n";
                } else if (second != 0) {
                    second = second - alphSize + 256;
                    std::cout << second << " \n";
                }


                if (third < alphSize && third != 0) {
                    third = alphTerminals[third];
                    std::cout << third << " \n";
                } else if (third != 0) {
                    third = third - alphSize + 256;
                    std::cout << third << " \n";
                }


                if (fourth < alphSize && fourth != 0) {
                    fourth = alphTerminals[fourth];
                    std::cout << fourth << " \n";
                } else if (fourth != 0) {
                    fourth = fourth - alphSize + 256;
                    std::cout << fourth << " \n";
                }
            }
            std::cout << "-1\n";
        }


        for (size_t i = 0; i < vectorContainingGrammar.size(); ++i) {
            // Print both arrays (inside the list entry / rule) of 4 integers
            for (const auto &innerArray2: vectorContainingGrammar[i]) {
                int first = innerArray2[0];
                int second = innerArray2[1];
                int third = innerArray2[2];
                int fourth = innerArray2[3];

                if (first < alphSize) {
                    first = alphTerminals[first];
                    std::cout << first << " \n";
                } else {
                    first = first - alphSize + 256;
                    std::cout << first << " \n";
                }

                if (second < alphSize && second != 0) {
                    second = alphTerminals[second];
                    std::cout << second << " \n";
                } else if (second != 0) {
                    second = second - alphSize + 256;
                    std::cout << second << " \n";
                }


                if (third < alphSize && third != 0) {
                    third = alphTerminals[third];
                    std::cout << third << " \n";
                } else if (third != 0) {
                    third = third - alphSize + 256;
                    std::cout << third << " \n";
                }


                if (fourth < alphSize && fourth != 0) {
                    fourth = alphTerminals[fourth];
                    std::cout << fourth << " \n";
                } else if (fourth != 0) {
                    fourth = fourth - alphSize + 256;
                    std::cout << fourth << " \n";
                }
            }
        }
        // close the file
        outputFile.close();


        CFG* cfg = new CFG();

        std::ifstream reader("navarroOutputFile.txt");
        std::string line;

        // read grammar specs (- original text length as that is not supplied by navarro's)
        std::getline(reader, line);
        cfg->numRules = std::stoi(line);
        std::getline(reader, line);
        cfg->startSize = std::stoi(line);
        cfg->rulesSize = 0;

        // prepare to read grammar
        cfg->startRule = cfg->numRules + CFG::MR_REPAIR_CHAR_SIZE;
        int rulesSize = cfg->startRule + 1;  // +1 for start rule
        cfg->rules = new int*[rulesSize];
        int* ruleSizes = new int[rulesSize - 1];
        for (int i = 0; i < CFG::MR_REPAIR_CHAR_SIZE; i++) {
            ruleSizes[i] = 1;
        }
        cfg->rules[cfg->startRule] = new int[cfg->startSize + 1];  // +1 for the dummy code
        int i, j, c, ruleLength;

        // read rules in order they were added to grammar, i.e. line-by-line
        for (i = CFG::MR_REPAIR_CHAR_SIZE; i < cfg->startRule; i++) {
            for (j = 0; ;j++) {
                std::getline(reader, line);
                c = std::stoi(line);
                // use start rule as a buffer
                cfg->rules[cfg->startRule][j] = c;
                if (c == CFG::MR_REPAIR_DUMMY_CODE) {
                    break;
                }
                ruleSizes[i] += ruleSizes[c];
            }
            ruleLength = j;
            cfg->rulesSize += ruleLength;
            cfg->rules[i] = new int[ruleLength + 1];
            for (j = 0; j < ruleLength + 1; j++) {
                cfg->rules[i][j] = cfg->rules[cfg->startRule][j];
            }
        }

        // read start rule
        uint32_t pos = 0;
        uint8_t* key = new uint8_t[CFG::KEY_LENGTH];
        int len;
        for (i = 0; i < cfg->startSize; i++) {
            // get the (non-)terminal character
            std::getline(reader, line);
            c = std::stoi(line);
            cfg->rules[cfg->startRule][i] = c;
            pos += ruleSizes[c];
        }
        cfg->rules[cfg->startRule][i] = CFG::MR_REPAIR_DUMMY_CODE;

        // clean up
        delete[] ruleSizes;

        return cfg;


    }

}

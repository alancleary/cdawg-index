#ifndef INCLUDED_CDAWG_INDEX_CFG
#define INCLUDED_CDAWG_INDEX_CFG

#include <ostream>
#include <string>

namespace cdawg_index {

/** A naive CFG representation. */
class CFG
{

private:

    static const int KEY_LENGTH = 6;
    static const int CHAR_SIZE = 256;

    static const int MR_REPAIR_CHAR_SIZE = 256;
    static const int MR_REPAIR_DUMMY_CODE = -1;  // UINT_MAX in MR-RePair C code

    int textLength;
    int numRules;
    int startSize;
    int rulesSize;
    int** rules;
    int startRule;

public:

    CFG();
    ~CFG();

    /**
     * Loads a grammar from an MR-Repair file.
     *
     * @param filename The file to load the grammar from.
     * @return The grammar that was loaded.
     * @throws Exception if the file cannot be read.
     */
    static CFG* fromMrRepairFile(std::string filename);

    /**
     * Loads a grammar from Navarro files.
     *
     * @param filenameC The grammar's C file.
     * @param filenameR The grammar's R file.
     * @return The grammar that was loaded.
     * @throws Exception if the files cannot be read.
     */
    static CFG* fromNavarroFiles(std::string filenameC, std::string filenameR);

    int getTextLength() { return textLength; }
    int getNumRules() { return numRules; }
    int getStartSize() { return startSize; }
    int getRulesSize() { return rulesSize; }
    int getTotalSize() { return startSize + rulesSize; }

};

}

#endif

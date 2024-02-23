#ifndef INCLUDED_CDAWG_INDEX_CFG
#define INCLUDED_CDAWG_INDEX_CFG

#include <functional>  // std::greater
#include <iterator>  // std::forward_iterator_tag
#include <map>
#include <stack>
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

    // NOTE: key order is reversed for finding nearest key that is <=
    std::map<int, int, std::greater<int>> startIndex;

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

    int getTextLength() const { return textLength; }
    int getNumRules() const { return numRules; }
    int getStartSize() const { return startSize; }
    int getRulesSize() const { return rulesSize; }
    int getTotalSize() const { return startSize + rulesSize; }

    /**
     * Gets the character in the given position in the text.
     *
     * @param q The position in the text.
     * @return The charcter.
     * @throws Exception if q is out of range.
     */
    char get(int q) const;

    class ConstIterator;

    ConstIterator cbegin() const;
    ConstIterator cbegin(int pos) const;
    ConstIterator cend() const;

};

/** An iterator for iterating the text in the CFG. */
class CFG::ConstIterator
{

using iterator_category = std::forward_iterator_tag;
//using difference_type = std::ptrdiff_t;
using value_type = char;
using pointer = char*;
using reference = char&;

private:

    const CFG* parent;

    // TODO: stacks should be preallocated to size of max depth
    std::stack<int> ruleStack;
    std::stack<int> indexStack;
    int skip;  // how many characters to skip before decoding
    int r;  // current rule being decoded
    int i;  // index in r of current (non-)terminal being decoded
    int j;  // currently decoded character in text

    value_type m_char;

    void next();

public:

    ConstIterator(const CFG* cfg, int pos);

    // dereference
    const reference operator*();
    const pointer operator->();

    // prefix increment
    ConstIterator& operator++();

    // postfix increment
    ConstIterator operator++(int);

    // comparators
    bool operator==(const ConstIterator& itr);
    bool operator!=(const ConstIterator& itr);

};

}

#endif

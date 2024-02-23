#include <algorithm>
#include <fstream>
#include "cdawg-index/cfg.hpp"

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
    int pos = 0;
    int len;
    for (i = 0; i < cfg->startSize; i++) {
        // get the (non-)terminal character
        std::getline(reader, line);
        c = std::stoi(line);
        cfg->rules[cfg->startRule][i] = c;
        cfg->startIndex[pos] = i;
        pos += ruleSizes[c];
    }
    cfg->rules[cfg->startRule][i] = CFG::MR_REPAIR_DUMMY_CODE;

    // clean up
    delete[] ruleSizes;

    return cfg;
}

// construction from Navarro grammar

CFG* CFG::fromNavarroFiles(std::string filenameC, std::string filenameR) {
    return NULL;
}

// access single character

/**
* A naive implementation of random access.
*
* The text positions of the (non-)terminal characters in the start rule are
* indexed when the grammar is loaded. Then any position is accessed by getting
* the closest indexed position and decoding the grammar up to the query position,
* ignoring the decoded characters prior to the query position.
*/
char CFG::get(int q) const
{

    // NOTE: map keys are reversed so this finds the largest key that's <= i
    auto itr = startIndex.lower_bound(q);
    if (q < 0 || q >= textLength || itr == startIndex.end()) {
        throw std::runtime_error("q out of bounds");
    }
    int pos = itr->first;
    int length = q - pos + 1;

    // TODO: stacks should be preallocated to size of max depth
    int r = startRule;
    int i = itr->second;
    char c;
    std::stack<int> ruleStack;
    std::stack<int> indexStack;
    for (int j = 0; j < length;) {
        // end of rule
        if (rules[r][i] == MR_REPAIR_DUMMY_CODE) {
            r = ruleStack.top();
            ruleStack.pop();
            i = indexStack.top();
            indexStack.pop();
        // terminal character
        } else if (rules[r][i] < MR_REPAIR_CHAR_SIZE) {
            c = (char) rules[r][i];
            i++;
            j++;
        // non-terminal character
        } else {
            ruleStack.push(r);
            r = rules[r][i];
            indexStack.push(i + 1);
            i = 0;
        }
    }
    return c;
}

// iterator

CFG::ConstIterator::ConstIterator(const CFG* cfg, int idx) : parent(cfg), j(idx)
{
    r = parent->startRule;
    i = 0;
    next();
}

const CFG::ConstIterator::reference CFG::ConstIterator::operator*()
{
    return m_char;
}

const CFG::ConstIterator::pointer CFG::ConstIterator::operator->()
{
    return &m_char;
}

void CFG::ConstIterator::next()
{
    // iterate until the next character is decoded or the end of the text
    while(j < parent->textLength) {
        // end of rule
        if (parent->rules[r][i] == MR_REPAIR_DUMMY_CODE) {
            r = ruleStack.top();
            ruleStack.pop();
            i = indexStack.top();
            indexStack.pop();
        // terminal character
        } else if (parent->rules[r][i] < MR_REPAIR_CHAR_SIZE) {
            char c = (char) parent->rules[r][i];
            m_char = c;
            i++;
            break;
        // non-terminal character
        } else {
            ruleStack.push(r);
            r = parent->rules[r][i];
            indexStack.push(i + 1);
            i = 0;
        }
    }
}

CFG::ConstIterator& CFG::ConstIterator::operator++()
{
    j++;
    next();
    return *this;
}

CFG::ConstIterator CFG::ConstIterator::operator++(int)
{
    ConstIterator tmp = *this;
    ++(*this);
    return tmp;
}

bool CFG::ConstIterator::operator== (const ConstIterator& itr)
{
    //return a.j == b.j;
    return this->j == itr.j;
}

bool CFG::ConstIterator::operator!= (const ConstIterator& itr)
{
    //return a.j != b.j;
    return this->j != itr.j;
}

CFG::ConstIterator CFG::cbegin() const
{
    return ConstIterator(this, 0);
}

CFG::ConstIterator CFG::cend() const
{
    return ConstIterator(this, textLength);
}

}

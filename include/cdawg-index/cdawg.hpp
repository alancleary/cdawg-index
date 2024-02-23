#ifndef INCLUDED_CDAWG_INDEX_CDAWG
#define INCLUDED_CDAWG_INDEX_CDAWG

#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>  // std::pair, std::make_pair
#include <vector>
#include "cdawg-index/cfg.hpp"

#include <set>

namespace cdawg_index {

/** A CDAWG that indexes a CFG. */
class CDAWG
{

private:

    const CFG* cfg;

    class Node;
    typedef std::pair<Node*, int> NodeAndPos;

    int sid_count = 0;
    Node* source;
    Node* sink;
    Node* bt;  // bottom node

    // indexing
    void buildIndex();
    NodeAndPos update(Node* s, int k, int p, char c);
    bool check_end_point(Node* s, int k, int p, char c);
    Node* extension(Node* s, int k, int p);
    void redirect_edge(Node* s, int k, int p, Node* r);
    Node* split_edge(Node* s, int k, int p);
    NodeAndPos separate_node(Node* s, int k, int p);
    NodeAndPos canonize(Node* s, int k, int p);

    void printNodes(Node* n, std::set<std::string>& visited);

    void deleteNodes(Node* n, std::set<Node*>& visited);

public:

    CDAWG(const CFG* cfg);
    ~CDAWG();

    bool search(const std::string& pattern);

    void printGraph();

};

/** A Node in the CDAWG. */
class CDAWG::Node
{

public:

    std::string id;
    Node* suf;
    int len;
    std::unordered_map<char, std::tuple<int, int, Node*>> to;

    Node(std::string id);
    Node(std::string id, Node& n);

    void edge(char c, int k, int p, Node* n);

};

}

#endif

#include <algorithm>
#include <iostream>
#include <fstream>
#include "cdawg-index/cdawg.hpp"
#include "cdawg-index/cfg.hpp"

namespace cdawg_index {

// construction

CDAWG::CDAWG(const CFG* cfg) : cfg(cfg)
{
    bt = new Node("root");
    bt->len = -1;

    source = new Node("source");
    source->suf = bt;
    source->len = 0;

    sink = new Node("sink");
    
    buildIndex();
}

// destruction

void CDAWG::deleteNodes(Node* n, std::set<Node*>& visited)
{
    if (visited.contains(n)) {
        return;
    }
    visited.insert(n);
    for (const auto &[key, value]: n->to) {
        deleteNodes(std::get<2>(value), visited);
    }
    delete n;
}

CDAWG::~CDAWG()
{
    Node* root = bt;
    std::set<Node*> visited;
    deleteNodes(root, visited);
}

void CDAWG::buildIndex()
{
    // active point
    NodeAndPos sk = std::make_pair(source, 0);
    // build the index while decoding the CFG
    char c;
    int i = 0;
    Node* s;
    int k;
    for (auto it = cfg->cbegin(), end = cfg->cend(); it != end; ++it) { 
        c = *it; 
        // create a new edge (_|_, (-j, -j), source).
        if (!this->bt->to.contains(c)) {
            this->bt->edge(c, i, i, this->source);
        }
        std::tie(s, k) = sk;
        sk = this->update(s, k, i, c);
        i++;
    }
    // manually add end character $
    //c = '$';
    //if (!this->bt->to.contains(c)) {
    //    this->bt->edge(c, i, i, this->source);
    //}
    //std::tie(s, k) = sk;
    //sk = this->update(s, k, i, c);
}

CDAWG::NodeAndPos CDAWG::update(Node* s, int k, int p, char c)
{
    // (s, (k, p - 1)) is the canonical reference pair for the active point.
    int textLength = cfg->getTextLength();
    Node* oldr = NULL;
    Node* s1 = NULL;
    Node* r = NULL;

    while (!check_end_point(s, k, p - 1, c)) {
        // implicit case
        if (k <= p - 1) {
            if (s1 == extension(s, k, p - 1)) {
                redirect_edge(s, k, p - 1, r);
                std::tie(s, k) = canonize(s->suf, k, p - 1);
                continue;
            } else {
                s1 = extension(s, k, p - 1);
                r = split_edge(s, k, p - 1);
            }
        // explicit case
        } else {
            r = s;
        }
        r->edge(cfg->get(p), p, textLength, sink);
        if (oldr != NULL) {
            oldr->suf = r;
        }
        oldr = r;
        std::tie(s, k) = canonize(s->suf, k, p - 1);
    }
    if (oldr != NULL) {
        oldr->suf = s;
    }
    return separate_node(s, k, p);
}

bool CDAWG::check_end_point(Node* s, int k, int p, char c)
{
    // implicit case
    if (k <= p) {
        int k1, p1;
        Node* s1;
        std::tie(k1, p1, s1) = s->to[cfg->get(k)];
        return c == cfg->get(k1 + p - k + 1);
    }
    return s->to.contains(c);
}

CDAWG::Node* CDAWG::extension(Node* s, int k, int p)
{
    // (s, (k, p)) is a canonical reference pair.
    if (k > p) {
        return s;
    }
    return std::get<2>(s->to[cfg->get(k)]);
}

void CDAWG::redirect_edge(Node* s, int k, int p, Node* r)
{
    int k1, p1;
    Node* s1;
    std::tie(k1, p1, s1) = s->to[cfg->get(k)];
    s->edge(cfg->get(k1), k1, k1 + p - k, r);
}

CDAWG::Node* CDAWG::split_edge(Node* s, int k, int p)
{
    // Let (s, (k1, p1), s1) be the w[k]-edge from s.
    int k1, p1;
    Node* s1;
    std::tie(k1, p1, s1) = s->to[cfg->get(k)];
    std::string id = "s" + std::to_string(this->sid_count++);
    Node* r = new Node(id);
    // Replace the edge by edges (s, (k1, k1 + p - k), r) and
    // (r, (k1 + p - k + 1, p1), s1).
    s->edge(cfg->get(k1), k1, k1 + p - k, r);
    r->edge(cfg->get(k1 + p - k + 1), k1 + p - k + 1, p1, s1);
    r->len = s->len + p - k + 1;
    return r;
}

CDAWG::NodeAndPos CDAWG::separate_node(Node* s, int k, int p)
{
    Node* s1;
    int k1;
    std::tie(s1, k1) = canonize(s, k, p);
    // implicit case
    if (k1 <= p) {
        return std::make_pair(s1, k1);
    }

    // explicit case

    // solid case
    if (s1->len == s->len + p - k + 1) {
        return std::make_pair(s1, k1);
    }

    // non-solid case: create node r1 as a duplication of s1, together with the
    // out-going edges of s1
    std::string id = "s" + std::to_string(this->sid_count++);
    Node* r1 = new Node(id, *s1);
    r1->suf = s1->suf;
    s1->suf = r1;
    r1->len = s->len + p - k + 1;
    NodeAndPos r = std::make_pair(s1, k1);
    do {
        // replace the w[k]-edge from s to s1 by edge (s, (k, p), r1)
        s->edge(cfg->get(k), k, p, r1);
        std::tie(s, k) = canonize(s->suf, k, p - 1);
    } while (r == canonize(s, k, p));
    return std::make_pair(r1, p + 1);
}

CDAWG::NodeAndPos CDAWG::canonize(Node* s, int k, int p)
{
    if (k > p) {
        return std::make_pair(s, k);
    }
    int k1, p1;
    Node* s1;
    std::tie(k1, p1, s1) = s->to[cfg->get(k)];
    while (p1 - k1 <= p - k) {
        k = k + p1 - k1 + 1;
        s = s1;
        if (k <= p) {
            std::tie(k1, p1, s1) = s->to[cfg->get(k)];
        }
    }
    return std::make_pair(s, k);
}

void CDAWG::printNodes(Node* n, std::set<std::string>& visited)
{
    if (visited.contains(n->id)) {
        return;
    }
    visited.insert(n->id);
    std::cerr << "id: " << n->id << std::endl;
    std::cerr << "\tlen: " << n->len << std::endl;
    if (n->suf == NULL) {
        std::cerr << "\tsuf: None" << std::endl;
    } else {
        std::cerr << "\tsuf: " << n->suf->id << std::endl;
    }
    std::cerr << "\tto:" << std::endl;
    int k, p;
    Node* s;
    for (const auto &[c, value]: n->to) {
        std::tie(k, p, s) = value;
        std::cerr << "\t\t" << c << ": ((k: " << k << ", p: " << p << "), target: " << s->id << ")" << std::endl;
    }
    std::cerr << std::endl;
    for (const auto &[key, value]: n->to) {
        printNodes(std::get<2>(value), visited);
    }
}

void CDAWG::printGraph()
{
    Node* root = bt;
    std::set<std::string> visited;
    printNodes(root, visited);
}

// Node

CDAWG::Node::Node(std::string id) : id(id)
{
    len = 0;
    suf = NULL;
}

CDAWG::Node::Node(std::string id, Node& n) : id(id), to(n.to)
{
    len = n.len;
    suf = n.suf;
}

void CDAWG::Node::edge(char c, int k, int p, Node* n)
{
    to[c] = {k, p, n};
}

}

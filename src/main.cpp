#include <iostream>
#include "cdawg-index/cfg.hpp"

using namespace std;
using namespace cdawg_index;

void usage(int argc, char* argv[]) {
    cerr << "usage: " << argv[0] << " <command> [<args>]" << endl;
    cerr << endl;
    cerr << "commands: " << endl;
    cerr << "\tindex: creates a CDAWG index for the given grammar" << endl;
    cerr << "\tsearch: uses a CDAWG index to search the given grammar" << endl;
}

void usageIndex(int argc, char* argv[]) {
    cerr << "usage: " << argv[0] << " index <type> <filename>" << endl;
    cerr << endl;
    cerr << "args: " << endl;
    cerr << "\ttype={mrrepair|navarro}: the type of grammar to load" << endl;
    cerr << "\t\tmrrepair: for grammars created with the MR-RePair algorithm" << endl;
    cerr << "\t\tnavarro: for grammars created with Navarro's implementation of RePair" << endl;
    cerr << "\tfilename: the name of the grammar file(s) without the extension" << endl;
    cerr << endl;
    cerr << "output: " << endl;
    cerr << "\t<filename>.cdawg: a file containing the computed CDAWG index" << endl;
}

void usageSearch(int argc, char* argv[]) {
    cerr << "usage: " << argv[0] << " search <type> <grammar> <pattern>" << endl;
    cerr << endl;
    cerr << "args: " << endl;
    cerr << "\ttype={mrrepair|navarro}: the type of grammar-compressed string to load" << endl;
    cerr << "\t\tmrrepair: for grammars created with the MR-RePair algorithm" << endl;
    cerr << "\t\tnavarro: for grammars created with Navarro's implementation of RePair" << endl;
    cerr << "\tfilename: the name of the grammar and CDAWG files without the extensions" << endl;
    cerr << "\tpattern: the pattern to search for" << endl;
}

CFG* loadGrammar(string type, string filename) {
    if (type == "mrrepair") {
        return CFG::fromMrRepairFile(filename + ".out");
    } else if (type == "navarro") {
        return CFG::fromNavarroFiles(filename + ".C", filename + ".R");
    }
    cerr << "invalid grammar type: \"" << type << "\"" << endl;
    cerr << endl;
    return NULL;
}

int index(int argc, char* argv[]) {
    // check the command-line arguments
    if (argc < 4) {
      usageIndex(argc, argv);
      return 1;
    }
    string type = argv[2];
    string filename = argv[3];
    CFG* cfg = loadGrammar(type, filename);
    if (cfg == NULL) {
      usageIndex(argc, argv);
      return 1;
    }
    return 0;
}

int search(int argc, char* argv[]) {
    // check the command-line arguments
    if (argc < 5) {
      usageSearch(argc, argv);
      return 1;
    }
    string type = argv[2];
    string filename = argv[3];
    CFG* cfg = loadGrammar(type, filename);
    if (cfg == NULL) {
      usageSearch(argc, argv);
      return 1;
    }
    string pattern = argv[3];
    // TODO: implement pattern matching
    return 0;
}

int main(int argc, char* argv[])
{

    // check the command-line arguments
    if (argc < 2) {
      usage(argc, argv);
      return 1;
    }

    // parse the command
    string command = argv[1];
    if (command == "index") {
        return index(argc, argv);
    } else if (command == "search") {
        return search(argc, argv);
    } else {
        cerr << "invalid command: \"" << command << "\"" << endl;
        cerr << endl;
        usage(argc, argv);
        return 1;
    }

    return 1;
}

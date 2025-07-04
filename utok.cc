// compile: g++ -std=c++11 utok.cc -o utok -lre2

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
 
#include "re2/re2.h"

using namespace std;
using namespace re2;


string build_regex(const char* infilename, vector<RE2*> &splits) {
  ifstream infile(infilename);
  string line;
  string relist = "(";
  
  
  while (getline(infile, line)) {
    size_t startpos = line.find_first_not_of(" \t");
    if (string::npos == startpos || string::npos != startpos && line[startpos] == '#') {
      continue;
    }
    size_t endpos = line.find_last_not_of(" \t") +1;
    if (line.rfind("*SPLIT", 0) == 0) {
      startpos = line.find_first_not_of(" \t", startpos + 6);
      splits.push_back(new RE2(line.substr(startpos, endpos)));
      //cerr << ">>> split:[" << line.substr(startpos, endpos) << "]\n";
    } else {
      relist += line.substr(startpos, endpos);
      relist += '|';
    }
  }
  relist.erase(relist.end() -1);
  relist += ')';
  return relist;
}



int main(int argc, char* argv[]) {

  if (argc < 2) {
    cerr << "usage: utok tokenize.ggrep <TEXT >VERT" << endl;
    return 1;
  }
  vector<RE2*> splits;
  string relist = build_regex(argv[1], splits);
  RE2::Options opts;
  opts.set_longest_match(true);
  RE2 re(relist, opts);

  string line;
  while (getline(cin, line)) {
    if (line.length() == 0)
      continue;
    StringPiece tok;
    StringPiece sline(line);
    
    for (int start = 0; start < line.length(); ) {
      if (!re.Match(sline, start, sline.length(), RE2::UNANCHORED, &tok, 1))
        break;
      if (start > 0 && tok.begin() - sline.begin() == start)
        cout << "<g/>\n";
      bool whole_tok = true;
      for (auto s: splits) {
        StringPiece parts[3];
        if (s->Match(tok, 0, tok.length(), RE2::ANCHOR_BOTH, parts, 3)) {
          cout << parts[1] << "\n<g/>\n" << parts[2] << '\n';
          whole_tok = false;
          break;
        }
      }
      if (whole_tok)
        cout << tok << '\n';
      start = tok.end() - sline.begin();
    }
  }


  return 0;
}

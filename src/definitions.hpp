#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

#include <bits/stdc++.h>

#ifdef DEBUG
  #define SHW(X) (cout << ">>> " << #X << ": " << (X) << endl), fflush(stdout)
  #define DBG(X) X
#else
  #define SHW(X)
  #define DBG(X)
#endif

// =============================================================================
// Formula
// =============================================================================
enum {CONJ=1,DISJ,IMPL,EQUI,NEGA,ATOM};
struct Vertex {
  char type;
  int variable;
  int up;
  std::vector<int> down;
  int p;
  Vertex();
};

// =============================================================================
// Globals
// =============================================================================
extern std::string raw; // input
extern std::vector<Vertex> T,G; // tree and DAG
extern int nextvar; // next variable id
extern std::unordered_map<int,std::string> varname; // variables' names
extern std::vector<int> R; // renamed formulas
extern std::set<std::set<int>> finalcnf;

#endif

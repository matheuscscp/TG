#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

#include <bits/stdc++.h>
using namespace std;

#ifdef DEBUG
  #define SHW(X) (cout << ">>> " << #X << ": " << (X) << endl), fflush(stdout)
  #define DBG(X) X
#else
  #define SHW(X)
  #define DBG(X)
#endif

#define MAXN    100005
#define clip(X) min(X,10000)

// formula
enum {CONJ=0,DISJ,IMPL,EQUI,NEGA,ATOM};
struct Vertex {
  char type;
  int variable;
  int up;
  vector<int> down;
  int p;
  Vertex() : p(1) {}
};
extern string raw; // input
extern vector<Vertex> T,G; // tree and DAG
extern int nextvar; // next variable id
extern unordered_map<int,string> varname; // variables' names
extern vector<int> R; // renamed formulas
extern set<set<int>> finalcnf;

#endif

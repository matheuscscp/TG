#ifndef LIBTG_HPP
#define LIBTG_HPP

#include <bits/stdc++.h>

#ifdef DEBUG
  #define SHW(X) (cout << ">>> " << #X << ": " << (X) << endl), fflush(stdout)
  #define DBG(X) X
#else
  #define SHW(X)
  #define DBG(X)
#endif

// =============================================================================
// Types
// =============================================================================

enum {CONJ=1,DISJ,IMPL,EQUI,NEGA,ATOM};
struct Vertex {
  char type;
  int variable;
  int up;
  std::vector<int> down;
  int p;
  Vertex();
  void remove();
};

// =============================================================================
// Globals
// =============================================================================

extern std::string raw;                             // input
extern std::vector<Vertex> T,G;                     // tree and DAG
extern int nextvar;                                 // next variable id
extern std::unordered_map<int,std::string> varname; // variables' names
extern std::vector<int> R;                          // renaming
extern std::set<std::set<int>> finalcnf;            // final CNF

// =============================================================================
// Functions
// =============================================================================

void parse();       // parse string raw to build tree T
void nnf();         // put tree T in NNF
void flat();        // flat tree T. (p|(q|r)|s) becomes (p|q|r|s)
void dag();         // convert tree T to DAG G
void mindag();      // minimize DAG G. (p|q)&(p|r|s|q) becomes (p|q)&((p|q)|r|s)
void boydelatour(); // Boy de la Tour's algorithm to choose a renaming
void knapsack();    // Knapsack 0-1 based algorithm to choose a renaming
void rename();      // apply renaming R to DAG G
void cnf();         // put DAG G in CNF (G must be in NNF)
void simplifycnf(); // simplify CNF in DAG G to set of clauses finalcnf
                    // removing tautologies and repeated clauses

std::ostream& operator<<(std::ostream&, const std::vector<Vertex>&);
std::ostream& operator<<(std::ostream&, const std::set<std::set<int>>&);

#endif

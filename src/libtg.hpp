#ifndef LIBTG_HPP
#define LIBTG_HPP

#include <bits/stdc++.h>

#include "uint_t.hpp"

// =============================================================================
// Types
// =============================================================================

enum {CONJ=1,DISJ,IMPL,EQUI,NEGA,ATOM};
struct Vertex {
  char type;
  int variable;
  int up;
  std::vector<int> down;
  Vertex();
  void remove();
};
struct FORMULA_t { // just a wrapper for output methods
  bool simple;
  FORMULA_t();
  // statistics
     int    size() const; // compute size
  uint_t clauses() const; // compute number of clauses
     int symbols() const; // compute number of symbols
};
std::ostream& operator<<(std::ostream&, const FORMULA_t&);

// =============================================================================
// Globals
// =============================================================================

extern std::string raw;                             // input
extern std::vector<Vertex> T,G;                     // tree and DAG
extern int nextvar;                                 // next variable id
extern std::unordered_map<int,std::string> varname; // variables' names
extern std::vector<int> R;                          // renaming
extern std::set<std::set<int>> simplified;          // simplified CNF
extern FORMULA_t FORMULA;                           // final formula wrapper

// =============================================================================
// Functions
// =============================================================================

// transformation pipeline
void parse();       // parse string raw to build tree T
void nnf();         // put tree T in NNF
void flat();        // flat tree T. (p|(q|r)|s) becomes (p|q|r|s)
void dag();         // convert tree T to DAG G
void mindag();      // minimize DAG G. (p|q)&(p|r|s|q) becomes (p|q)&((p|q)|r|s)
void boydelatour(); // Boy de la Tour's algorithm to choose a renaming
void knapsack();    // Knapsack 0-1 based algorithm to choose a renaming
void rename();      // apply renaming R to DAG G
void cnf();         // put DAG G in CNF (G must be in NNF)
void simplecnf();   // put DAG G in CNF (G must be in NNF), removing
                    // tautologies, repeated literals and repeated clauses

// statistics
   int    size(const std::vector<Vertex>&); // compute formula size
uint_t clauses(const std::vector<Vertex>&); // compute number of clauses
   int symbols(const std::vector<Vertex>&); // compute number of symbols

// formatted output
std::string op2str(char); // convert integer logic operator to string symbol
std::ostream& operator<<(std::ostream&, const std::vector<Vertex>&);

#endif

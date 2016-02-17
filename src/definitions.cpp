#include "definitions.hpp"

string raw; // input
vector<Vertex> T,G; // tree and DAG
int nextvar = 1; // next variable id
unordered_map<int,string> varname; // variables' names
vector<int> R; // renamed formulas
set<set<int>> finalcnf;

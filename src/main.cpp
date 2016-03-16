#include "libtg.hpp"

using namespace std;

int main() {
  getline(cin,raw);
  parse();
  nnf();
  flat();
  dag();
  mindag();
  knapsack();
  rename();
  simplecnf();
  cout << "(" << raw << ") <=> (" << CNF << ")\n";
  cout << CNF.size() << " " << CNF.clauses() << " " << CNF.symbols() << endl;
  return 0;
}

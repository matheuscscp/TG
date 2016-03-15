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
  cnf();
  cout << "(" << raw << ") <=> (" << CNF << ")\n";
  return 0;
}

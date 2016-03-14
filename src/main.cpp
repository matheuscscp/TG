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
  simplifycnf();
  cout << "(" << raw << ") <=> (" << finalcnf << ")\n";
  return 0;
}

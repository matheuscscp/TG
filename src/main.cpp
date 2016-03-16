#include "libtg.hpp"

using namespace std;

int main() {
  getline(cin,raw);
  parse();
  nnf();
  //flat();
  dag();
  mindag();
  knapsack();
  rename();
  simplecnf();
  cout << "(" << raw << ") <=> (" << CNF << ")\n";
  return 0;
}

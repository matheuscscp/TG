#include "libtg.hpp"
#include "uint_t.hpp"

using namespace std;

// p(Rnm) = number of clauses if we rename the subformulas in Rnm
typedef uint_t ptype;
static ptype p(const vector<int>& Rnm) {
  vector<ptype> dp(G.size(),0);
  function<ptype(int)> f = [&](int u) {
    ptype& ans = dp[u];
    if (ans) return ans;
    switch (char(G[u].type&~(1<<7))) {
      case CONJ:
        ans = 0;
        for (int v : G[u].down) ans += (G[v].type&(1<<7) ? ptype(1) : f(v));
        break;
      case DISJ:
        ans = 1;
        for (int v : G[u].down) ans *= (G[v].type&(1<<7) ? ptype(1) : f(v));
        break;
      default:
        ans = 1;
        break;
    }
    return ans;
  };
  
  for (int u : Rnm) G[u].type |=  (1<<7);
  ptype ret = f(0);
  for (int u : Rnm) ret += f(u);
  for (int u : Rnm) G[u].type &= ~(1<<7);
  return ret;
}

// f(i,j) = "optimal renaming, with at most j subformulas, considering
//           subformulas labeled from 1 to i"
// then...
// f(0,j) = f(i,0) = {}
// f(i,j) = { f(i-1,j-1) U {i}          if p(f(i-1,j-1) U {i}) < p(f(i-1,j))
//          { f(i-1,j)                  otherwise
void knapsack() {
  // find subformulas with BFS
  vector<int> subformulas;
  {
    vector<bool> found(G.size(),false);
    found[0] = true;
    queue<int> Q;
    Q.push(0);
    while (!Q.empty()) {
      int u = Q.front(); Q.pop();
      for (int v : G[u].down) if (!found[v]) {
        found[v] = true;
        Q.push(v);
        subformulas.push_back(v);
      }
    }
  }
  
  // knapsack 0-1
  int n = subformulas.size();
  vector<vector<int>> dp(n+1);
  for (int i = 0; i < n; i++) for (int j = n; 1 <= j; j--) {
    vector<int> alt = dp[j-1]; alt.push_back(subformulas[i]);
    if (p(alt) < p(dp[j])) dp[j] = alt;
  }
  
  // as dp[j] is now f(n,j), we have that R = dp[n] = f(n,n).
  // alternatively, R may be f(n,K), for some K <= n.
  R = dp[n];
}

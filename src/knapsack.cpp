#include "definitions.hpp"

// p(Rnm) = number of clauses if we rename the subformulas in Rnm
static int64_t p(const vector<int>& Rnm) {
  vector<int64_t> dp(G.size(),0);
  function<int64_t(int)> f = [&](int u) {
    int64_t& ans = dp[u];
    if (ans) return ans;
    switch (char(G[u].type&~(1<<7))) {
      case CONJ:
        ans = 0; for (int v : G[u].down) ans += (G[v].type&(1<<7) ? 1 : f(v));
        break;
      case DISJ:
        ans = 1; for (int v : G[u].down) ans *= (G[v].type&(1<<7) ? 1 : f(v));
        break;
      default:
        ans = 1;
        break;
    }
    return ans;
  };
  
  for (int u : Rnm) G[u].type |=  (1<<7);
  int64_t ret = f(0);
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
  vector<int> subformulas;
  { // find subformulas with BFS
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

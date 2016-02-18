#include "definitions.hpp"

// position of u in a reverse toposort
static int pos(int u) {
  static int next = 1, dp[MAXN] = {};
  int& ans = dp[u];
  if (ans) return ans;
  for (int v : G[u].down) pos(v);
  return ans = next++;
}

// p(phi(u))
static int p(int u) {
  static int dp[MAXN] = {};
  int& ans = dp[u];
  if (ans) return ans;
  switch (G[u].type) {
    case CONJ: ans = 0; for (int v : G[u].down) ans = clip(ans+p(v)); break;
    case DISJ: ans = 1; for (int v : G[u].down) ans = clip(ans*p(v)); break;
    default:   ans = 1; break;
  }
  return ans;
}

// Boy de la Tour's top-down renaming
static void R_rec(int u, int a) {
  auto& phi = G[u];
  if (phi.p == 1) return;
  
  // check renaming condition
  bool renamed = false;
  if (a >= 2 && (a != 2 || phi.p != 2)) { // ap > a+p
    a = 1;
    renamed = true;
  }
  
  // search children
  if (phi.type == CONJ) {
    for (int v : phi.down) R_rec(v,a);
    phi.p = 0; for (int v : phi.down) phi.p = clip(phi.p+G[v].p);
  }
  else { // phi.type == DISJ
    int n = phi.down.size();
    
    vector<int> dp(n,1); // dp[i] = prod(phi_j.p), i < j < n
    for (int i = n-2; 0 <= i; i--) dp[i] = clip(G[phi.down[i+1]].p*dp[i+1]);
    
    int ai = a; // ai = a*prod(phi_j.p), 0 <= j < i
    for (int i = 0; i < n; i++) {
      R_rec(phi.down[i],clip(ai*dp[i]));
      ai = clip(ai*G[phi.down[i]].p);
    }
    phi.p = 1; for (int v : phi.down) phi.p = clip(phi.p*G[v].p);
  }
  
  if (renamed) {
    R.push_back(u);
    phi.p = 1;
  }
}

void boydelatour() {
  // necessary preprocessing for Boy de la Tour's algorithm
  auto toposortless = [](int u, int v) { return pos(u) < pos(v); };
  for (int u = 0; u < G.size(); u++) {
    auto& phi = G[u];
    sort(phi.down.begin(),phi.down.end(),toposortless);
    phi.p = p(u);
  }
  
  R_rec(0,1); // recursive algorithm
}

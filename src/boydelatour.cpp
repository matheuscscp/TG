#include "libtg.hpp"

#define clip(X) min(X,10000)

using namespace std;

// position of u in a reverse toposort
static vector<int> posdp;
static int pos(int u) {
  static int next = 1;
  int& ans = posdp[u];
  if (ans) return ans;
  for (int v : G[u].down) pos(v);
  return ans = next++;
}

// p(phi(u))
static vector<int> p;
static int p_(int u) {
  int& ans = p[u];
  if (ans) return ans;
  switch (G[u].type) {
    case CONJ: ans = 0; for (int v : G[u].down) ans = clip(ans+p_(v)); break;
    case DISJ: ans = 1; for (int v : G[u].down) ans = clip(ans*p_(v)); break;
    default:   ans = 1; break;
  }
  return ans;
}

// Boy de la Tour's top-down renaming
static void R_rec(int u, int a) {
  auto& phi = G[u];
  if (p[u] == 1) return;
  
  // check renaming condition
  bool renamed = false;
  if (a >= 2 && (a != 2 || p[u] != 2)) { // ap > a+p
    a = 1;
    renamed = true;
  }
  
  // search children
  if (phi.type == CONJ) {
    for (int v : phi.down) R_rec(v,a);
    p[u] = 0; for (int v : phi.down) p[u] = clip(p[u]+p[v]);
  }
  else { // phi.type == DISJ
    int n = phi.down.size();
    
    vector<int> dp(n,1); // dp[i] = prod(phi_j.p), i < j < n
    for (int i = n-2; 0 <= i; i--) dp[i] = clip(p[phi.down[i+1]]*dp[i+1]);
    
    int ai = a; // ai = a*prod(phi_j.p), 0 <= j < i
    for (int i = 0; i < n; i++) {
      R_rec(phi.down[i],clip(ai*dp[i]));
      ai = clip(ai*p[phi.down[i]]);
    }
    p[u] = 1; for (int v : phi.down) p[u] = clip(p[u]*p[v]);
  }
  
  if (renamed) {
    R.push_back(u);
    p[u] = 1;
  }
}

void boydelatour() {
  // dp tables
  posdp = vector<int>(G.size(),0);
  p = vector<int>(G.size(),0);
  
  // necessary preprocessing for Boy de la Tour's algorithm
  // compute p field and reverse toposort edges
  auto toposortless = [](int u, int v) { return pos(u) < pos(v); };
  for (int u = 0; u < G.size(); u++) {
    auto& phi = G[u];
    sort(phi.down.begin(),phi.down.end(),toposortless);
    p[u] = p_(u);
  }
  
  R_rec(0,1); // recursive algorithm
}

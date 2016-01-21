#include <bits/stdc++.h>
using namespace std;

#define MAXN    100005
#define clip(X) min(X,10000)

// IMPLEMENTACAO/TESTES
// antes, procurar formulas menores nas maiores, mais ou menos R² * d(phi)
// dp para otimizar o renaming restringindo o nro maximo de literais
// procurar um benchmark
// adaptar entrada para o benchmark
// adaptar saida para algum provador, colocando em CNF

// MONOGRAFIA
// colocar exemplo 3.2

// formula
enum {CONJ=0,DISJ,IMPL,EQUI,NEGA,ATOM};
struct Vertex {
  char type;
  int literal;
  int up;
  vector<int> N;
  int p;
  Vertex() : p(1) {}
} G[MAXN];
int V = 0;

// create tree parsing formula from string
// first: tree
// second: number of literals
pair<vector<Vertex>,int> parse(string phi) {
  // check
  auto isvarsym = [](char c) {
    return
      ('0' <= c && c <= '9') ||
      ('a' <= c && c <= 'z') ||
      ('A' <= c && c <= 'Z')
    ;
  };
  
  vector<Vertex> ret(1);
  ret.back().type = CONJ;
  ret.back().up = 0;
  stack<int> S;
  S.push(0);
  map<string,int> id;
  int nextlit = 1;
  for (int i = 0; i < phi.size(); i++) {
    char c = phi[i];
    if (c == ' ' || c == '\t') continue;
    else if (c == '&') ret[S.top()].type = CONJ;
    else if (c == '|') ret[S.top()].type = DISJ;
    else if (c == '(' || c == '~') {
      S.push(ret.size());
      ret.emplace_back();
      ret.back().type = (c == '(' ? CONJ : NEGA);
    }
    else if (c == ')') {
      int u = S.top();
      S.pop();
      ret[u].up = S.top();
      ret[S.top()].N.push_back(u);
    }
    else { // literal
      string tmp = {phi[i]};
      while (i+1 < phi.size() && isvarsym(phi[i+1])) tmp += phi[++i];
      int& lit = id[tmp];
      if (!lit) lit = nextlit++;
      ret[S.top()].N.push_back(ret.size());
      ret.emplace_back();
      ret.back().type = ATOM;
      ret.back().literal = lit;
      ret.back().up = S.top();
      if (ret[S.top()].type == NEGA) { phi[i] = ')'; i--; }
    }
  }
  return make_pair(ret,id.size());
}

void nnf(vector<Vertex>& T) {
  // copy tree rooted at u
  function<int(int)> copy = [&](int u) {
    auto& phi = T[u];
    int nu = T.size(); T.emplace_back();
    auto& nphi = T[nu];
    nphi.type = phi.type;
    nphi.literal = phi.literal;
    for (int v : phi.N) {
      int tmp = copy(v);
      T[tmp].up = nu;
      nphi.N.push_back(tmp);
    }
    return nu;
  };
  
  // remove implications and equivalences
  function<void(int)> dfs1 = [&](int u) {
    auto& phi = T[u];
    if (phi.type == IMPL) {
      phi.type = DISJ;
      int v = phi.N.front();
      auto& phi1 = T[v];
      int negi = T.size(); T.emplace_back();
      auto& neg = T[negi]; neg.type = NEGA;
      phi.N.front() = negi;
      neg.up = u;
      neg.N.push_back(v);
      phi1.up = negi;
    }
    else if (phi.type == EQUI) {
      auto f = [&](int a, int b) {
        int impi = T.size(); T.emplace_back();
        auto& imp = T[impi]; imp.type = IMPL;
        phi.N.push_back(impi);
        imp.up = u;
        imp.N.push_back(a), imp.N.push_back(b);
        T[a].up = impi, T[b].up = impi;
      };
      phi.type = CONJ;
      int a = phi.N.front(),  b = phi.N.back();
      int c = copy(b),        d = copy(a);
      phi.N.clear();
      f(a,b);
      f(c,d);
    }
    for (int v : phi.N) dfs1(v);
  };
  dfs1(0);
  
  // remove negations
  function<void(int,bool)> dfs2 = [&](int u, bool neg) {
    auto& phi = T[u];
    if (!neg) {
      if (phi.type == NEGA) {
        auto& phi1 = T[phi.N.front()];
        if (phi1.type == NEGA) {
          auto& phi2 = T[phi1.N.front()];
          phi.type = phi2.type;
          phi.literal = phi2.literal;
          phi.N = phi2.N;
          for (int v : phi.N) T[v].up = u;
          phi1.type = CONJ, phi2.type = CONJ; // removing phi1 and phi2
        }
        else if (phi1.type != ATOM) { // CONJ or DISJ
          phi.type = (phi1.type == CONJ ? DISJ : CONJ);
          phi.N = phi1.N;
          for (int v : phi.N) T[v].up = u;
          phi1.type = CONJ; // removing phi1
          neg = true;
        }
      }
    }
    else {
      if (phi.type == NEGA) {
        auto& phi1 = T[phi.N.front()];
        phi.type = phi1.type;
        phi.literal = phi1.literal;
        phi.N = phi1.N;
        for (int v : phi.N) T[v].up = u;
        phi1.type = CONJ; // removing phi1
        neg = false;
      }
      else if (phi.type != ATOM) { // CONJ or DISJ
        phi.type = (phi.type == CONJ ? DISJ : CONJ);
      }
      else { // ATOM
        int atmi = copy(u);
        phi.type = NEGA;
        phi.N.push_back(atmi);
        T[atmi].up = u;
        neg = false;
      }
    }
    for (int v : phi.N) dfs2(v,neg);
  };
  dfs2(0,false);
}

// build DAG from tree T
void build(const vector<Vertex>& T) {
  map<int,int> lit_newu;
  map<int,multiset<int>> oldp_newc;
  V = 1;
  
  // create vertices for literals
  for (int i = 0; i < T.size(); i++) if (T[i].type == ATOM) {
    auto& phi = T[i];
    int& u = lit_newu[phi.literal];
    if (!u) {
      u = ++V;
      G[u].type = ATOM;
      G[u].literal = phi.literal;
    }
    oldp_newc[phi.up].insert(u);
  }
  
  // search tree bottom-up to create vertices for subformulas
  for (map<multiset<int>,int> newc_newp; !oldp_newc.empty();) {
    list<pair<int,int>> tmp;
    for (auto kv = oldp_newc.begin(); kv != oldp_newc.end();) {
      auto& phi = T[kv->first];
      if (kv->second.size() < phi.N.size()) { kv++; continue; }
      int& u = newc_newp[kv->second];
      if (!u) {
        u = (phi.up != kv->first) ? ++V : 1;
        G[u].type = phi.type;
        for (int v : kv->second) G[u].N.push_back(v);
      }
      if (phi.up != kv->first) tmp.emplace_back(phi.up,u);
      oldp_newc.erase(kv++);
    }
    for (auto& kv : tmp) oldp_newc[kv.first].insert(kv.second);
  }
}

// position of u in a reverse toposort
int pos(int u) {
  static int next = 1, dp[MAXN] = {};
  int& ans = dp[u];
  if (ans) return ans;
  for (int v : G[u].N) pos(v);
  return ans = next++;
}

// p(phi(u))
int p(int u) {
  static int dp[MAXN] = {};
  int& ans = dp[u];
  if (ans) return ans;
  switch (G[u].type) {
    case CONJ: ans = 0; for (int v : G[u].N) ans = clip(ans+p(v)); break;
    case DISJ: ans = 1; for (int v : G[u].N) ans = clip(ans*p(v)); break;
    default:   ans = 1; break;
  }
  return ans;
}

// Boy de la Tour's top-down renaming
vector<int> R;
int nextR;
void R_rec(int u, int a) {
  auto& phi = G[u];
  if (phi.p == 1) return;
  
  // check renaming condition
  bool renamed = false;
  if (a > 1) {
    a = 1;
    renamed = true;
  }
  
  // search children
  if (phi.type == CONJ) {
    for (int v : phi.N) R_rec(v,a);
    phi.p = 0; for (int v : phi.N) phi.p = clip(phi.p+G[v].p);
  }
  else { // phi.type == DISJ
    int n = phi.N.size();
    
    vector<int> dp(n,1); // dp[i] = prod(phi_j.p), i < j < n
    for (int i = n-2; 0 <= i; i--) dp[i] = clip(G[phi.N[i+1]].p*dp[i+1]);
    
    int ai = a; // ai = a*prod(phi_j.p), 0 <= j < i
    for (int i = 0; i < n; i++) {
      R_rec(phi.N[i],clip(ai*dp[i]));
      ai = clip(ai*G[phi.N[i]].p);
    }
    phi.p = 1; for (int v : phi.N) phi.p = clip(phi.p*G[v].p);
  }
  
  if (renamed) {
    R.push_back(u);
    phi.type |= (1<<7);
    phi.literal = nextR++;
    phi.p = 1;
  }
}

// pretty print formula root at u
void print(int u, bool enclose = true) {
  auto& phi = G[u];
  
  // literal
  if (phi.type == ATOM || phi.type&(1<<7)) {
    cout << 'p' << phi.literal;
    return;
  }
  
  // negation
  if (phi.type == NEGA) {
    cout << '~';
    print(phi.N.front());
    return;
  }
  
  // subformulas
  if (enclose) cout << '(';
  char op = phi.type == CONJ ? '&' : '|';
  bool printed = false;
  for (int v : phi.N) {
    if (printed) cout << " " << op << " ";
    printed = true;
    print(v);
  }
  if (enclose) cout << ')';
}

int main() {
  
  // input
  string phi;
  getline(cin,phi);
  
  // preprocess
  auto tree = parse(phi);
  nnf(tree.first);
  build(tree.first);
  auto toposort = [](int u, int v){ return pos(u) < pos(v); };
  for (int u = 1; u <= V; u++) {
    auto& phi = G[u];
    sort(phi.N.begin(),phi.N.end(),toposort);
    phi.p = p(u);
  }
  print(1,false);
  cout << endl;
  
  // renaming
  nextR = tree.second+1;
  R_rec(1,1);
  
  // output
  print(1);
  for (int u : R) {
    auto& phi = G[u];
    printf(" & (~p%d | ",phi.literal);
    phi.type &= ~(1<<7);
    print(u);
    phi.type |=  (1<<7);
    printf(")");
  }
  cout << endl;
  
  return 0;
}

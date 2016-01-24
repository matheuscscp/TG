#include <bits/stdc++.h>
using namespace std;

#define SHOW(X) (cout << ">>>>> " << #X << ": " << (X) << endl), fflush(stdout)

// IMPLEMENTACAO/TESTES
// procurar formulas menores nas maiores, mais ou menos R² * d(phi) (melhoria1)
// dp para otimizar o renaming limitando o nro maximo de literais   (melhoria2)
// procurar um benchmark e adaptar a entrada para ele
// adaptar a saida para algum provador, colocando em CNF

// MONOGRAFIA
// colocar exemplo 3.2

#define MAXN    100005
#define clip(X) min(X,10000)

// formula
enum {CONJ=0,DISJ,IMPL,EQUI,NEGA,ATOM};
struct Vertex {
  char type;
  int literal;
  int up;
  vector<int> down;
  int p;
  Vertex() : p(1) {}
};
vector<Vertex> G;

// parse string to create tree
// first: tree
// second: number of literals
pair<vector<Vertex>,int> parse(string phi) {
  // tells if a character is valid for a variable name
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
    else if (c == '=') { ret[S.top()].type = IMPL; i++; }
    else if (c == '<') { ret[S.top()].type = EQUI; i += 2; }
    else if (c == '(' || c == '~') { // push
      S.push(ret.size());
      ret.emplace_back();
      ret.back().type = (c == '(' ? CONJ : NEGA);
    }
    else if (c == ')') { // pop
      int u = S.top();
      S.pop();
      ret[u].up = S.top();
      ret[S.top()].down.push_back(u);
      if (ret[S.top()].type == NEGA) i--;
      // removing multiple parentheses
      if (ret[u].type != NEGA && ret[u].down.size() == 1) {
        int u1 = ret[u].down.front();
        ret[u].type = ret[u1].type;
        ret[u].down = ret[u1].down;
        for (int v : ret[u].down) ret[v].up = u;
      }
    }
    else { // literal
      string tmp = {phi[i]};
      while (i+1 < phi.size() && isvarsym(phi[i+1])) tmp += phi[++i];
      int& lit = id[tmp];
      if (!lit) lit = nextlit++;
      ret[S.top()].down.push_back(ret.size());
      ret.emplace_back();
      ret.back().type = ATOM;
      ret.back().literal = lit;
      ret.back().up = S.top();
      if (ret[S.top()].type == NEGA) { phi[i] = ')'; i--; }
    }
  }
  // removing multiple parentheses
  if (ret[0].down.size() == 1) {
    int u = ret[0].down.front();
    ret[0].type = ret[u].type;
    ret[0].down = ret[u].down;
    for (int v : ret[0].down) ret[v].up = 0;
  }
  return make_pair(ret,id.size());
}

void nnf(vector<Vertex>& T) {
  // copy tree rooted at u
  function<int(int)> copy = [&](int u) {
    int nu = T.size(); T.emplace_back();
    T[nu].type = T[u].type;
    T[nu].literal = T[u].literal;
    for (int v : T[u].down) {
      int tmp = copy(v);
      T[tmp].up = nu;
      T[nu].down.push_back(tmp);
    }
    return nu;
  };
  
  // create negation vertex as parent of u
  function<int(int)> neg = [&](int u) {
    int nu = T.size(); T.emplace_back();
    T[nu].type = NEGA;
    T[nu].down.push_back(u);
    T[u].up = nu;
    return nu;
  };
  
  // remove implications and equivalences
  function<void(int,bool)> dfs1 = [&](int u, bool pol) {
    if (T[u].type == NEGA) pol = !pol;
    else if (T[u].type == IMPL) {
      T[u].type = DISJ;
      int v = neg(T[u].down.front());
      T[u].down.front() = v;
      T[v].up = u;
    }
    else if (T[u].type == EQUI) {
      auto f = [&](int a, char op, int b) {
        int impi = T.size(); T.emplace_back();
        T[impi].type = op;
        T[u].down.push_back(impi);
        T[impi].up = u;
        T[impi].down.push_back(a), T[impi].down.push_back(b);
        T[a].up = impi, T[b].up = impi;
      };
      int a  = T[u].down.front(), b  = T[u].down.back();
      int ac = copy(a),           bc = copy(b);
      T[u].down.clear();
      if (!pol) f(a,IMPL,b), T[u].type = CONJ, f(bc,IMPL,ac);
      else      f(a,CONJ,b), T[u].type = DISJ, f(neg(ac),CONJ,neg(bc));
    }
    for (int v : T[u].down) dfs1(v,pol);
  };
  dfs1(0,false);
  
  // remove negations
  function<void(int,bool)> dfs2 = [&](int u, bool pol) {
    if (!pol) {
      if (T[u].type == NEGA) {
        auto& phi1 = T[T[u].down.front()];
        if (phi1.type == NEGA) {
          auto& phi2 = T[phi1.down.front()];
          T[u].type = phi2.type;
          T[u].literal = phi2.literal;
          T[u].down = phi2.down;
          for (int v : T[u].down) T[v].up = u;
          phi1.type = CONJ, phi2.type = CONJ; // removing phi1 and phi2
        }
        else if (phi1.type != ATOM) { // CONJ or DISJ
          T[u].type = (phi1.type == CONJ ? DISJ : CONJ);
          T[u].down = phi1.down;
          for (int v : T[u].down) T[v].up = u;
          phi1.type = CONJ; // removing phi1
          pol = true;
        }
      }
    }
    else {
      if (T[u].type == NEGA) {
        auto& phi1 = T[T[u].down.front()];
        T[u].type = phi1.type;
        T[u].literal = phi1.literal;
        T[u].down = phi1.down;
        for (int v : T[u].down) T[v].up = u;
        phi1.type = CONJ; // removing phi1
        pol = false;
      }
      else if (T[u].type != ATOM) { // CONJ or DISJ
        T[u].type = (T[u].type == CONJ ? DISJ : CONJ);
      }
      else { // ATOM
        int atmi = copy(u);
        T[u].type = NEGA;
        T[u].down.push_back(atmi);
        T[atmi].up = u;
        pol = false;
      }
    }
    for (int v : T[u].down) dfs2(v,pol);
  };
  dfs2(0,false);
}

// build DAG from tree T
void build(const vector<Vertex>& T) {
  map<int,int> lit_newu;
  map<int,multiset<int>> oldp_newc;
  G.emplace_back();
  
  // create vertices for literals
  for (auto& phi : T) if (phi.type == ATOM) {
    int& u = lit_newu[phi.literal];
    if (!u) {
      u = G.size(); G.emplace_back();
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
      if (kv->second.size() < phi.down.size()) { kv++; continue; }
      int& u = newc_newp[kv->second];
      if (!u) {
        u = 0;
        if (phi.up != kv->first) u = G.size(), G.emplace_back();
        G[u].type = phi.type;
        for (int v : kv->second) G[u].down.push_back(v);
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
  for (int v : G[u].down) pos(v);
  return ans = next++;
}

// p(phi(u))
int p(int u) {
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
    phi.type |= (1<<7);
    phi.literal = nextR++;
    phi.p = 1;
  }
}

// convert formula to human readable string
string arr2str(const vector<Vertex>& formula, int root = 0) {
  auto logicop = [](char type) {
    switch (type) {
      case CONJ: return "&";
      case DISJ: return "|";
      case IMPL: return "=>";
      case EQUI: return "<=>";
    }
    return "";
  };
  stringstream ss;
  function<void(int)> dfs = [&](int u) {
    auto& phi = formula[u];
    if (phi.type == ATOM || phi.type&(1<<7)) {
      ss << "p" << phi.literal;
      return;
    }
    if (phi.type == NEGA) {
      ss << "~";
      dfs(phi.down.front());
      return;
    }
    ss << "(";
    string op = logicop(phi.type);
    bool printed = false;
    for (int u : phi.down) {
      if (printed) ss << " " << op << " ";
      printed = true;
      dfs(u);
    }
    ss << ")";
  };
  auto& phi = formula[root];
  if (phi.type == ATOM || phi.type&(1<<7)) {
    ss << "p" << phi.literal;
    return ss.str();
  }
  if (phi.type == NEGA) {
    ss << "~";
    dfs(phi.down.front());
    return ss.str();
  }
  string op = logicop(phi.type);
  bool printed = false;
  for (int u : phi.down) {
    if (printed) ss << " " << op << " ";
    printed = true;
    dfs(u);
  }
  return ss.str();
}

int main() {
  
  // input
  string raw;
  getline(cin,raw);
  cout << "raw:        " << raw << endl;
  
  // preprocess
  auto tree = parse(raw);
  cout << "parsed:     " << arr2str(tree.first) << endl;
  nnf(tree.first);
  cout << "NNF:        " << arr2str(tree.first) << endl;
  build(tree.first);
  cout << "DAG:        " << arr2str(G) << endl;
  auto toposort = [](int u, int v){ return pos(u) < pos(v); };
  for (int u = 0; u < G.size(); u++) {
    auto& phi = G[u];
    sort(phi.down.begin(),phi.down.end(),toposort);
    phi.p = p(u);
  }
  cout << "toposorted: " << arr2str(G) << endl;
  
  // renaming
  nextR = tree.second+1;
  R_rec(0,1);
  
  // output
  cout << "renamed:    ";
  if (R.size() > 0) cout << "(";
  cout << arr2str(G);
  if (R.size() > 0) cout << ")";
  for (int u : R) {
    auto& phi = G[u];
    phi.type &= ~(1<<7);
    cout << " & (~p" << phi.literal << " | (" << arr2str(G,u) << "))";
    phi.type |=  (1<<7);
  }
  cout << endl;
  
  return 0;
}

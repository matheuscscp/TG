#include "libtg.hpp"

using namespace std;

// =============================================================================
// Globals
// =============================================================================

string raw;
vector<Vertex> T,G;
int nextvar = 1;
unordered_map<int,string> varname;
vector<int> R;
set<set<int>> finalcnf;

// =============================================================================
// Implementation
// =============================================================================

Vertex::Vertex() : type(0), p(1) {}

void Vertex::remove() {
  type = 0;
  vector<int>().swap(down);
}

void parse() {
  // advance index i (in string raw) until next c character
  auto find = [](char c, int& i) {
    while (raw[i] != c) i++;
  };
  
  // tells if a character is valid for a variable name
  auto isvarsym = [](char c) {
    return
      c == '_' ||
      ('0' <= c && c <= '9') ||
      ('a' <= c && c <= 'z') ||
      ('A' <= c && c <= 'Z')
    ;
  };
  
  // remove redundant parentheses
  auto compress = [](int u) {
    int up = T[u].up;
    while (T[u].type != NEGA && T[u].down.size() == 1) {
      auto& v = T[T[u].down[0]];
      T[u] = v;
      v.remove();
    }
    T[u].up = up;
    for (int v : T[u].down) T[v].up = u;
  };
  
  T.emplace_back(); // root
  stack<int> S;
  S.push(0);
  unordered_map<string,int> id;
  for (int i = 0; i < raw.size(); i++) {
    char c = raw[i];
         if (c == '&')   T[S.top()].type = CONJ;
    else if (c == '|')   T[S.top()].type = DISJ;
    else if (c == '=') { T[S.top()].type = IMPL; find('>', i); }
    else if (c == '<') { T[S.top()].type = EQUI; find('=', i); find('>', i); }
    else if (c == '(' || c == '~') { // push
      // setup new vertex
      int u = S.top(), v = T.size(); T.emplace_back();
      if (c == '~') T[v].type = NEGA;
      T[v].up = u; // up edge (v,u)
      
      T[u].down.push_back(v); // down edge (u,v)
      
      S.push(v);
    }
    else if (c == ')' || isvarsym(c)) { // pop
      if (c == ')') { compress(S.top()); S.pop(); }
      else { // propositional symbol
        // read whole symbol
        string tmp = {raw[i]};
        while (i+1 < raw.size() && isvarsym(raw[i+1])) tmp += raw[++i];
        
        // setup symbol ID
        int& var = id[tmp];
        if (!var) { var = nextvar++; varname[var] = tmp; }
        
        // setup new vertex
        int u = S.top(), v = T.size(); T.emplace_back();
        T[v].type = ATOM;
        T[v].variable = var;
        T[v].up = u; // up edge (v,u)
        
        T[u].down.push_back(v); // down edge (u,v)
      }
      while (T[S.top()].type == NEGA) S.pop(); // negation is unary!
    }
  }
  compress(0);
}

void nnf() {
  // copy tree rooted at u
  function<int(int)> copy = [&](int u) {
    int nu = T.size(); T.emplace_back();
    T[nu].type = T[u].type;
    T[nu].variable = T[u].variable;
    for (int v : T[u].down) {
      int tmp = copy(v);
      T[tmp].up = nu;
      T[nu].down.push_back(tmp);
    }
    return nu;
  };
  
  // create negation as parent of u
  function<int(int)> neg = [](int u) {
    int nu = T.size(); T.emplace_back();
    T[nu].type = NEGA;
    T[nu].down.push_back(u);
    T[u].up = nu;
    return nu;
  };
  
  // create binary vertex
  auto bin = [](int a, char op, int b) {
    int u = T.size(); T.emplace_back();
    T[u].type = op;
    T[u].down.push_back(a); T[u].down.push_back(b);
    T[a].up = u, T[b].up = u;
    return u;
  };
  
  function<void(int,bool)> dfs = [&](int u, bool pol) {
    // compress negation path
    int up = T[u].up;
    while (T[u].type == NEGA) {
      pol = !pol;
      auto& v = T[T[u].down[0]];
      T[u] = v;
      v.remove();
    }
    T[u].up = up;
    
    if (T[u].type == ATOM && !pol) return;
    
    switch (T[u].type) {
      case CONJ: if (pol) T[u].type = DISJ; break;
      case DISJ: if (pol) T[u].type = CONJ; break;
      case ATOM: // pol == true
        pol = false;
        T[u].down.push_back(copy(u));
        T[u].type = NEGA;
        break;
      case IMPL:
        T[u].type = (!pol ? DISJ : CONJ);
        T[u].down[0] = neg(T[u].down[0]);
        break;
      case EQUI: {
        T[u].type = CONJ;
        int a  = T[u].down[0], b  = T[u].down[1];
        int ac = copy(a),      bc = copy(b);
        if (!pol) {
          T[u].down[0] = bin( a,IMPL, b);
          T[u].down[1] = bin(bc,IMPL,ac);
        }
        else {
          T[u].down[0] = bin(      a,CONJ,      b);
          T[u].down[1] = bin(neg(ac),CONJ,neg(bc));
        }
        break;
      }
    }
    
    for (int v : T[u].down) { T[v].up = u; dfs(v,pol); }
  };
  dfs(0,false);
}

// (p|(q|r)|s) becomes (p|q|r|s)
static void flat(vector<Vertex>& formula, int u) {
  auto& phi = formula[u];
  for (bool changed = true; changed;) {
    changed = false;
    unordered_set<int> newc;
    for (int v : phi.down) {
      auto& psi = formula[v];
      if ((psi.type != CONJ && psi.type != DISJ) || psi.type != phi.type) {
        newc.insert(v);
        continue;
      }
      changed = true;
      for (int w : psi.down) newc.insert(w);
    }
    if (changed) {
      phi.down.clear();
      for (int v : newc) phi.down.push_back(v);
    }
  }
  for (int v : phi.down) formula[v].up = u;
}

void flat() {
  function<void(int)> dfs = [&](int u) {
    flat(T,u);
    for (int v : T[u].down) dfs(v);
  };
  dfs(0);
}

void dag() {
  unordered_map<int,int> var_newu;
  unordered_map<int,pair<set<int>,int>> oldp_newc;
  G.emplace_back(); // root is always u == 0
  
  // create vertices for variables
  for (auto& phi : T) if (phi.type == ATOM) {
    int& u = var_newu[phi.variable];
    if (!u) {
      u = G.size(); G.emplace_back();
      G[u].type = ATOM;
      G[u].variable = phi.variable;
    }
    oldp_newc[phi.up].first.insert(u);
    oldp_newc[phi.up].second++;
  }
  
  // search tree bottom-up to create vertices for subformulas
  for (map<set<int>,int> newc_newp; !oldp_newc.empty();) {
    list<pair<int,int>> tmp;
    for (auto kv = oldp_newc.begin(); kv != oldp_newc.end();) {
      auto& phi = T[kv->first];
      if (kv->second.second < phi.down.size()) { kv++; continue; }
      int& u = newc_newp[kv->second.first];
      if (!u) {
        if (kv->first) u = G.size(), G.emplace_back();
        G[u].type = phi.type;
        for (int v : kv->second.first) G[u].down.push_back(v);
      }
      if (kv->first) tmp.emplace_back(phi.up,u);
      oldp_newc.erase(kv++);
    }
    for (auto& kv : tmp) {
      oldp_newc[kv.first].first.insert(kv.second);
      oldp_newc[kv.first].second++;
    }
  }
}

void mindag() {
  for (bool changed = true; changed;) {
    changed = false;
    for (int u = 0; u < G.size(); u++) {
      if (G[u].type != CONJ && G[u].type != DISJ) continue;
      auto& e1 = G[u].down;
      sort(e1.begin(),e1.end());
      for (int v = 0; v < G.size(); v++) {
        if (v == u || G[v].type != G[u].type) continue;
        auto& e2 = G[v].down;
        sort(e2.begin(),e2.end());
        vector<int> newc;
        bool uchildofv = false;
        bool issubset = true;
        int i = 0;
        for (int j = 0; j < e2.size() && issubset; j++) {
          if (e2[j] == u) uchildofv = true;
          if (i == e1.size() || e1[i] > e2[j]) newc.push_back(e2[j]);
          else if (e1[i] < e2[j]) issubset = false;
          else i++;
        }
        if (i < e1.size() || !issubset) continue;
        changed = true;
        if (!uchildofv) newc.push_back(u);
        e2 = newc;
      }
    }
  }
}

void rename() {
  if (R.size() == 0) return;
  
  // create new variables
  for (int u : R) {
    auto& phi = G[u];
    phi.variable = nextvar++;
    stringstream ss;
    ss << "rnm" << phi.variable;
    varname[phi.variable] = ss.str();
  }
  
  // move old root to new vertex
  int oldroot = G.size(); G.emplace_back();
  G[oldroot] = G[0];
  
  // init new root
  G[0].type = CONJ;
  G[0].down.clear();
  G[0].down.push_back(oldroot);
  
  // add new definitions
  for (int u : R) {
    // create disjunction
    int dis = G.size(); G.emplace_back();
    G[dis].type = DISJ;
    G[0].down.push_back(dis);
    
    // create negation
    int neg = G.size(); G.emplace_back();
    G[neg].type = NEGA;
    G[dis].down.push_back(neg);
    G[neg].down.push_back(u);
    
    // move renamed formula
    int v = G.size(); G.emplace_back();
    G[v] = G[u];
    G[dis].down.push_back(v);
    
    // change renamed formula to variable
    G[u].type = ATOM;
  }
}

void cnf() {
  vector<bool> visited(G.size(),false);
  function<void(int)> dfs = [&](int u) {
    visited[u] = true;
    flat(G,u);
    if (G[u].type == DISJ) {
      int con = -1;
      for (auto it = G[u].down.begin(); it != G[u].down.end(); it++) {
        if (G[*it].type == CONJ) {
          con = *it;
          G[u].down.erase(it);
          break;
        }
      }
      if (con < 0) return;
      int rem = G.size(); G.emplace_back(); visited.push_back(false);
      G[rem] = G[u];
      G[u].type = CONJ;
      G[u].down.clear();
      for (int v : G[con].down) {
        int dis = G.size(); G.emplace_back(); visited.push_back(false);
        G[dis].type = DISJ;
        G[dis].down.push_back(rem);
        G[dis].down.push_back(v);
        G[u].down.push_back(dis);
      }
    }
    for (int v : G[u].down) if (!visited[v]) dfs(v);
    flat(G,u);
  };
  dfs(0);
}

void simplifycnf() {
  for (int u : G[0].down) {
    set<int> tmp;
    bool tautology = false;
    if (G[u].type == ATOM)      tmp.insert(G[u].variable);
    else if (G[u].type == NEGA) tmp.insert(G[G[u].down[0]].variable);
    else for (int v : G[u].down) {
      int lit;
      if (G[v].type == ATOM)  lit = G[v].variable;
      else                    lit = -G[G[v].down[0]].variable;
      if (tmp.find(-lit) == tmp.end()) tmp.insert(lit);
      else {
        tautology = true;
        break;
      }
    }
    if (!tautology) finalcnf.insert(tmp);
  }
}

// char vertex type to string operator
static string char2str(char type) {
  switch (type) {
    case CONJ: return "&";
    case DISJ: return "|";
    case IMPL: return "=>";
    case EQUI: return "<=>";
  }
}

ostream& operator<<(ostream& os, const vector<Vertex>& formula) {
  function<void(int)> dfs = [&](int u) {
    if (formula[u].type == ATOM) {
      os << varname[formula[u].variable];
      return;
    }
    if (formula[u].type == NEGA) {
      os << "~";
      dfs(formula[u].down[0]);
      return;
    }
    string op = char2str(formula[u].type);
    bool printed = false;
    if (u) os << "(";
    for (int v : formula[u].down) {
      if (printed) os << " " << op << " ";
      printed = true;
      dfs(v);
    }
    if (u) os << ")";
  };
  dfs(0);
  return os;
}

ostream& operator<<(ostream& os, const set<set<int>>& formula) {
  bool pr1 = false;
  for (auto& clause : formula) {
    if (pr1) os << " & ";
    pr1 = true;
    bool pr2 = false;
    os << "(";
    for (int l : clause) {
      if (pr2) os << " | ";
      pr2 = true;
      if (l < 0) os << "~";
      os << varname[abs(l)];
    }
    os << ")";
  }
  return os;
}

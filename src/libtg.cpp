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
set<set<int>> simplified;
FORMULA_t FORMULA;

// =============================================================================
// Implementation
// =============================================================================

Vertex::Vertex() : type(0) {}

void Vertex::remove() {
  type = 0;
  vector<int>().swap(down);
}

FORMULA_t::FORMULA_t() : simple(false) {}

int FORMULA_t::size() const {
  if (!simple) return ::size(G.size() > 0 ? G : T);
  if (simplified.size() == 0) return 1;
  int ret = simplified.size()-1;
  for (auto& clause : simplified) {
    if (clause.size() == 0) ret++;
    else if (clause.size() == 1) {
      ret++;
      if (*clause.begin() < 0) ret++;
    }
    else {
      ret += (clause.size()-1);
      for (int l : clause) {
        ret++;
        if (l < 0) ret++;
      }
    }
  }
  return ret;
}

uint_t FORMULA_t::clauses() const {
  if (!simple) return ::clauses(G.size() > 0 ? G : T);
  return simplified.size();
}

int FORMULA_t::symbols() const {
  if (!simple) return ::symbols(G.size() > 0 ? G : T);
  unordered_set<int> symbols;
  for (auto& clause : simplified) for (int l : clause) symbols.insert(abs(l));
  return symbols.size();
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
      if (c == ')') {
        int u = S.top();
        compress(u);
        if (T[u].type != ATOM && T[u].down.size() == 0) { // tautology
          int& var = id["tauto"];
          if (!var) { var = nextvar++; varname[var] = "tauto"; }
          T[u].type = DISJ;
          int u1 = T.size(); T.emplace_back();
          int u2 = T.size(); T.emplace_back();
          int u3 = T.size(); T.emplace_back();
          T[u].down.push_back(u1);
          T[u].down.push_back(u2);
          T[u1].type = ATOM;
          T[u1].variable = var;
          T[u1].up = u;
          T[u2].type = NEGA;
          T[u2].up = u;
          T[u2].down.push_back(u3);
          T[u3].type = ATOM;
          T[u3].variable = var;
          T[u3].up = u2;
        }
        S.pop();
      }
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
  bool is_tree = (&formula == &T);
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
      if (is_tree) psi.remove();
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
  vector<int> newu(T.size());
  unordered_map<int,int> var_newu, oldp_newc;
  G.emplace_back(); // root is always u == 0
  
  // formula is a single propositional symbol
  if (T[0].type == ATOM) {
    G[0] = T[0];
    return;
  }
  
  // create vertices for variables
  for (int i = 0; i < T.size(); i++) if (T[i].type == ATOM) {
    auto& phi = T[i];
    int& u = var_newu[phi.variable];
    if (!u) {
      u = G.size(); G.emplace_back();
      G[u].type = ATOM;
      G[u].variable = phi.variable;
    }
    newu[i] = u;
    oldp_newc[phi.up]++;
  }
  
  // search tree bottom-up to create vertices for subformulas
  map<pair<char,set<int>>,int> newp;
  map<pair<int,int>,int> newp_impl;
  while (!oldp_newc.empty()) {
    list<int> inc;
    for (auto kv = oldp_newc.begin(); kv != oldp_newc.end();) {
      auto& phi = T[kv->first];
      if (kv->second < phi.down.size()) { kv++; continue; }
      if (kv->first) inc.push_back(phi.up);
      int* u;
      vector<int> down;
      if (phi.type != IMPL) {
        set<int> down_set;
        for (int v : phi.down) {
          int tmp = newu[v];
          down.push_back(tmp);
          down_set.insert(tmp);
        }
        u = &newp[make_pair(phi.type,down_set)];
      }
      else {
        pair<int,int> down_pair;
        int tmp = newu[phi.down[0]];
        down.push_back(tmp);
        down_pair.first = tmp;
        tmp = newu[phi.down[1]];
        down.push_back(tmp);
        down_pair.second = tmp;
        u = &newp_impl[down_pair];
      }
      if (!(*u)) {
        if (kv->first) *u = G.size(), G.emplace_back();
        G[*u].type = phi.type;
        G[*u].down.swap(down);
      }
      newu[kv->first] = *u;
      oldp_newc.erase(kv++);
    }
    for (int u : inc) oldp_newc[u]++;
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
    G[u].down.clear();
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

void simplecnf() {
  FORMULA.simple = true;
  
  auto insert = [](set<int>& clause, int u) {
    int lit = (G[u].type == ATOM ? G[u].variable : -G[G[u].down[0]].variable);
    if (clause.find(-lit) != clause.end()) return false; // tautology
    clause.insert(lit);
    return true;
  };
  
  vector<bool> visited(G.size(),false);
  function<void(int)> dfs = [&](int u) {
    visited[u] = true;
    flat(G,u);
    if (G[u].type == DISJ) {
      set<int> clause;
      int con = -1;
      for (auto it = G[u].down.begin(); it != G[u].down.end(); it++) {
        if (G[*it].type == ATOM || G[*it].type == NEGA) {
          if (!insert(clause,*it)) { G[u].remove(); return; }
        }
        else {
          con = *it;
          G[u].down.erase(it);
          break;
        }
      }
      if (con < 0) { simplified.insert(clause); return; }
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
    else if (G[u].type != CONJ) {
      set<int> clause;
      insert(clause,u);
      simplified.insert(clause);
      return;
    }
    for (int v : G[u].down) if (!visited[v]) dfs(v);
    flat(G,u);
  };
  dfs(0);
}

int size(const vector<Vertex>& formula) {
  vector<int> dp(formula.size(),0);
  function<int(int)> dfs = [&](int u) {
    int& ret = dp[u];
    if (ret) return ret;
    if (formula[u].type == ATOM) return ret = 1;
    ret = (formula[u].type == NEGA ? 1 : 0);
    for (int v : formula[u].down) ret += (1+dfs(v));
    ret--;
    return ret;
  };
  return dfs(0);
}

uint_t clauses(const vector<Vertex>& formula) {
  vector<vector<uint_t>> dp(2,vector<uint_t>(formula.size(),uint_t(0)));
  function<uint_t(int,int)> f = [&](int i, int u) {
    // memoization
    uint_t& ret = dp[i][u];
    if (ret) return ret;
    
    // base
    if (formula[u].type == ATOM) return ret = uint_t(1);
    
    // transition
    int p1 = formula[u].down[0];
    int p2 = (formula[u].down.size() > 1 ? formula[u].down[1] : 0);
    if (i == 0) switch (formula[u].type) {
      case NEGA: ret = f(1,p1);                         break;
      case IMPL: ret = f(1,p1)*f(0,p2);                 break;
      case EQUI: ret = f(1,p1)*f(0,p2)+f(1,p2)*f(0,p1); break;
      case CONJ:
        ret = uint_t(0);
        for (int v : formula[u].down) ret += f(0,v);
        break;
      case DISJ:
        ret = uint_t(1);
        for (int v : formula[u].down) ret *= f(0,v);
        break;
    }
    else switch (formula[u].type) {
      case NEGA: ret = f(0,p1);                         break;
      case IMPL: ret = f(0,p1)+f(1,p2);                 break;
      case EQUI: ret = f(0,p1)*f(0,p2)+f(1,p1)*f(1,p2); break;
      case CONJ:
        ret = uint_t(1);
        for (int v : formula[u].down) ret *= f(1,v);
        break;
      case DISJ:
        ret = uint_t(0);
        for (int v : formula[u].down) ret += f(1,v);
        break;
    }
    
    return ret;
  };
  return f(0,0);
}

int symbols(const vector<Vertex>& formula) {
  unordered_set<int> symbols;
  vector<bool> visited(formula.size(),false);
  function<void(int)> dfs = [&](int u) {
    visited[u] = true;
    if (formula[u].type == ATOM) symbols.insert(formula[u].variable);
    else for (int v : formula[u].down) if (!visited[v]) dfs(v);
  };
  dfs(0);
  return symbols.size();
}

string op2str(char type) {
  switch (type) {
    case NEGA: return "~";
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
      os << op2str(NEGA);
      dfs(formula[u].down[0]);
      return;
    }
    bool printed = false;
    if (u) os << "(";
    for (int v : formula[u].down) {
      if (printed) os << " " << op2str(formula[u].type) << " ";
      printed = true;
      dfs(v);
    }
    if (u) os << ")";
  };
  dfs(0);
  return os;
}

ostream& operator<<(ostream& os, const FORMULA_t& formula) {
  if (!formula.simple) return os << (G.size() > 0 ? G : T);
  if (simplified.size() == 0) return os << "tauto " << op2str(IMPL) << " tauto";
  bool pr1 = false;
  for (auto& clause : simplified) {
    if (pr1) os << " " << op2str(CONJ) << " ";
    pr1 = true;
    bool pr2 = false;
    if (simplified.size() > 1 && clause.size() > 1) os << "(";
    for (int l : clause) {
      if (pr2) os << " " << op2str(DISJ) << " ";
      pr2 = true;
      if (l < 0) os << op2str(NEGA);
      os << varname[abs(l)];
    }
    if (simplified.size() > 1 && clause.size() > 1) os << ")";
  }
  return os;
}

#include "definitions.hpp"

// parse string raw to build tree T
void parse() {
  // tells if a character is valid for a variable name
  auto isvarsym = [](char c) {
    return
      c == '_' ||
      ('0' <= c && c <= '9') ||
      ('a' <= c && c <= 'z') ||
      ('A' <= c && c <= 'Z')
    ;
  };
  
  T.emplace_back();
  T.back().type = CONJ;
  T.back().up = 0;
  stack<int> S;
  S.push(0);
  unordered_map<string,int> id;
  for (int i = 0; i < raw.size(); i++) {
    char c = raw[i];
    if (c == ' ' || c == '\t') continue;
    else if (c == '&') T[S.top()].type = CONJ;
    else if (c == '|') T[S.top()].type = DISJ;
    else if (c == '=') { T[S.top()].type = IMPL; i++; }
    else if (c == '<') { T[S.top()].type = EQUI; i += 2; }
    else if (c == '(' || c == '~') { // push
      S.push(T.size());
      T.emplace_back();
      T.back().type = (c == '(' ? CONJ : NEGA);
    }
    else if (c == ')') { // pop
      int u = S.top();
      S.pop();
      T[u].up = S.top();
      T[S.top()].down.push_back(u);
      if (T[S.top()].type == NEGA) i--;
      // removing multiple parentheses
      if (T[u].type != NEGA && T[u].down.size() == 1) {
        int u1 = T[u].down.front();
        T[u].type = T[u1].type;
        T[u].down = T[u1].down;
        for (int v : T[u].down) T[v].up = u;
      }
    }
    else { // variable
      string tmp = {raw[i]};
      while (i+1 < raw.size() && isvarsym(raw[i+1])) tmp += raw[++i];
      int& var = id[tmp];
      if (!var) { var = nextvar++; varname[var] = tmp; }
      T[S.top()].down.push_back(T.size());
      T.emplace_back();
      T.back().type = ATOM;
      T.back().variable = var;
      T.back().up = S.top();
      if (T[S.top()].type == NEGA) { raw[i] = ')'; i--; }
    }
  }
  // removing multiple parentheses
  if (T[0].down.size() == 1) {
    int u = T[0].down.front();
    T[0].type = T[u].type;
    T[0].down = T[u].down;
    for (int v : T[0].down) T[v].up = 0;
  }
}

// put tree T in NNF
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
  
  // create negation vertex as parent of u
  function<int(int)> neg = [](int u) {
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
          T[u].variable = phi2.variable;
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
        T[u].variable = phi1.variable;
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

// apply "flattening", as (p & (q & r) & s) should become (p & q & r & s)
void flat(vector<Vertex>& formula, int u) {
  auto& phi = formula[u];
  for (bool changed = true; changed;) {
    changed = false;
    unordered_set<int> newc;
    for (int v : phi.down) {
      auto& psi = formula[v];
      if (psi.type == NEGA || psi.type == ATOM || psi.type != phi.type) {
        newc.insert(v);
        continue;
      }
      changed = true;
      for (int w : psi.down) {
        newc.insert(w);
        formula[w].up = u;
      }
    }
    if (changed) {
      phi.down.clear();
      for (int v : newc) phi.down.push_back(v);
    }
  }
}

// flat tree T
void flat() {
  function<void(int)> dfs = [&](int u) {
    flat(T,u);
    for (int v : T[u].down) dfs(v);
  };
  dfs(0);
}

// convert tree T to DAG G
void dag() {
  unordered_map<int,int> var_newu;
  unordered_map<int,pair<set<int>,int>> oldp_newc;
  G.emplace_back(); // root is always u=0
  
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
        u = 0;
        if (phi.up != kv->first) u = G.size(), G.emplace_back();
        G[u].type = phi.type;
        for (int v : kv->second.first) G[u].down.push_back(v);
      }
      if (phi.up != kv->first) tmp.emplace_back(phi.up,u);
      oldp_newc.erase(kv++);
    }
    for (auto& kv : tmp) {
      oldp_newc[kv.first].first.insert(kv.second);
      oldp_newc[kv.first].second++;
    }
  }
}

// find formulas inside other formulas, like (p & q) inside (p & ~r & q)
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

// apply renaming
void boydelatour();
void knapsack();
static function<void()> renalgos[] = {
  boydelatour,
  knapsack
};
enum {
  BOYDELATOUR = 0,
  KNAPSACK
};
void rename(int algo) {
  renalgos[algo](); // use selected algorithm to fill vector R
  if (R.size() == 0) return;
  
  // create new variables
  for (int u : R) {
    auto& phi = G[u];
    phi.variable = nextvar++;
    stringstream ss;
    ss << "$" << phi.variable;
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

// put DAG G in CNF, assuming it's already in NNF
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

// compute final CNF removing satisfied, super and repeated clauses from DAG G
void simplifycnf() {
  // remove repetions and satisfied clauses
  for (int u : G[0].down) {
    set<int> tmp;
    bool satisfied = false;
    if (G[u].type == ATOM)      tmp.insert(G[u].variable);
    else if (G[u].type == NEGA) tmp.insert(G[G[u].down.front()].variable);
    else for (int v : G[u].down) {
      int lit;
      if (G[v].type == ATOM)  lit = G[v].variable;
      else                    lit = -G[G[v].down.front()].variable;
      if (tmp.find(-lit) == tmp.end()) tmp.insert(lit);
      else {
        satisfied = true;
        break;
      }
    }
    if (!satisfied) finalcnf.insert(tmp);
  }
  // remove super clauses
  for (auto it = finalcnf.begin(); it != finalcnf.end();) {
    bool found = false;
    for (auto it2 = finalcnf.begin(); it2 != finalcnf.end(); it2++) {
      if (it2 == it || it2->size() > it->size()) continue;
      auto i = it->begin();
      auto i2 = it2->begin();
      while (i2 != it2->end() && i != it->end()) {
        if (*i2 > *i) i++;
        else if (*i2 == *i) i2++, i++;
        else break;
      }
      if (i2 == it2->end()) {
        found = true;
        break;
      }
    }
    if (!found) it++;
    else        finalcnf.erase(it++);
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
    if (phi.type == ATOM) {
      ss << varname[phi.variable];
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
  if (phi.type == ATOM) {
    ss << varname[phi.variable];
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

// print final CNF
void print() {
  auto printl = [](int l) {
    if (l < 0) cout << "~";
    cout << varname[abs(l)];
  };
  bool p1 = false;
  for (auto& C : finalcnf) {
    if (p1) cout << " & ";
    p1 = true;
    if (C.size() > 1) cout << "(";
    bool p2 = false;
    for (int l : C) {
      if (p2) cout << " | ";
      p2 = true;
      printl(l);
    }
    if (C.size() > 1) cout << ")";
  }
  cout << endl;
}

int main() {
  
  getline(cin,raw);
  DBG(cout << "raw:        " << raw << endl);
  parse();
  DBG(cout << "parsed:     " << arr2str(T) << endl);
  nnf();
  DBG(cout << "NNF:        " << arr2str(T) << endl);
  flat();
  DBG(cout << "flat:       " << arr2str(T) << endl);
  dag();
  DBG(cout << "DAG:        " << arr2str(G) << endl);
  mindag();
  DBG(cout << "min DAG:    " << arr2str(G) << endl);
  rename(KNAPSACK);
  DBG(cout << "renamed:    " << arr2str(G) << endl);
  cnf();
  DBG(cout << "CNF:        " << arr2str(G) << endl);
  simplifycnf();
  DBG(cout << "simple CNF: ");
  print();
  
  return 0;
}

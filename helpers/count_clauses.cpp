#include <bits/stdc++.h>
#include "src/uint_t.hpp"

using namespace std;

enum {CONJ=1,DISJ,IMPL,EQUI,NEGA,ATOM};
struct Node {
  vector<int> down;
  int type;
  int lit;
  Node() : type(0), lit(0) {}
};
string raw;
int rawi;
vector<Node> T;
stack<int> S;
map<string,int> literal;
int nextlit = 1;

bool issym(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_';
}

void compress(int u) {
  while (T[u].down.size() == 1 && T[u].type == 0) {
    auto& v = T[T[u].down[0]];
    T[u] = v;
    v.down = vector<int>();
  }
}

void handle() {
  char c = raw[rawi];
  if (c == '(' || c == '~') {
    int u = S.top(), v = T.size(); T.emplace_back();
    S.push(v);
    T[u].down.push_back(v);
    if (c == '~') T[v].type = NEGA;
  }
  else if (c == ')') {
    compress(S.top()); S.pop();
    while (T[S.top()].type == NEGA) S.pop();
  }
  else if (c == '&') {
    T[S.top()].type = CONJ;
  }
  else if (c == '|') {
    T[S.top()].type = DISJ;
  }
  else if (c == '=') {
    while (raw[rawi] != '>') rawi++;
    T[S.top()].type = IMPL;
  }
  else if (c == '<') {
    while (raw[rawi] != '=') rawi++;
    while (raw[rawi] != '>') rawi++;
    T[S.top()].type = EQUI;
  }
  else if (issym(c)) {
    string p;
    for (; rawi < raw.size() && issym(raw[rawi]); rawi++) p += raw[rawi];
    if (rawi < raw.size()) rawi--;
    int& lit = literal[p];
    if (!lit) lit = nextlit++;
    int u = T.size(); T.emplace_back();
    T[u].type = ATOM;
    T[u].lit = lit;
    T[S.top()].down.push_back(u);
    while (T[S.top()].type == NEGA) S.pop();
  }
}

typedef uint_t ptype;
ptype p(int u, bool barra = false) {
  ptype tmp;
  if (!barra) switch (T[u].type) {
    case ATOM: return ptype(1);
    case NEGA: return p(T[u].down[0],true);
    case IMPL: return p(T[u].down[0],true)*p(T[u].down[1]);
    case EQUI: return (p(T[u].down[0])*p(T[u].down[1],true))+(p(T[u].down[0],true)*p(T[u].down[1]));
    case CONJ: tmp = 0; for (int v : T[u].down) tmp += p(v); break;
    case DISJ: tmp = 1; for (int v : T[u].down) tmp *= p(v); break;
  }
  else switch (T[u].type) {
    case ATOM: return ptype(1);
    case NEGA: return p(T[u].down[0]);
    case IMPL: return p(T[u].down[0])+p(T[u].down[1],true);
    case EQUI: return (p(T[u].down[0])*p(T[u].down[1]))+(p(T[u].down[0],true)*p(T[u].down[1],true));
    case CONJ: tmp = 1; for (int v : T[u].down) tmp *= p(v,true); break;
    case DISJ: tmp = 0; for (int v : T[u].down) tmp += p(v,true); break;
  }
  return tmp;
}

int main() {
  getline(cin,raw);
  T.emplace_back();
  S.push(0);
  for (rawi = 0; rawi < raw.size(); rawi++) handle();
  compress(0);
  cout << p(0) << endl;
  return 0;
}

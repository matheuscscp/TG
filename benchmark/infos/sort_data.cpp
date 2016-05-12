#include <bits/stdc++.h>

using namespace std;

int main() {
  map<string, vector<string>> m;
  for (string s; cin >> s;) {
    string name;
    int i;
    for (i = 1; s[i] != ','; i++) name += s[i];
    auto& fields = m[name];
    while (i < s.size()-1) {
      string field;
      for (i++; s[i] != ','; i++) field += s[i];
      fields.push_back(field);
    }
  }
  for (auto& kv : m) {
    printf(",%s,",kv.first.c_str());
    for (auto& f : kv.second) printf("%s,",f.c_str());
    printf("\n");
  }
  return 0;
}

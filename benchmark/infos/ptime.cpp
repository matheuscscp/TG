#include <bits/stdc++.h>

using namespace std;

map<string, vector<string>> read(const string& fn) {
  fstream f(fn.c_str(),fstream::in);
  map<string, vector<string>> m;
  for (string s; f >> s;) {
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
  f.close();
  return m;
}

int main(int argc, char** argv) {
  string tmp = argv[1];
  auto m1 = read(tmp);
  int n = tmp.size();
  tmp[n-4] = 'p';
  tmp[n-3] = 't';
  tmp[n-2] = 'i';
  tmp[n-1] = 'm';
  tmp += "e";
  auto m2 = read(tmp);
  for (auto& kv : m1) {
    auto& ptime = m2[kv.first];
    if (kv.second.size() == 1 || ptime.size() == 1) continue;
    printf(",%s,%s,\n",kv.second[1].c_str(),ptime[1].c_str());
  }
  return 0;
}

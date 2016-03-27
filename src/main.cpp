#include "libtg.hpp"

using namespace std;

// argv[0]
string exe;

void usage() {
  cerr << "Usage mode: " << exe << " [options...] \n";
  cerr << endl;
  cerr << "I/O options:\n";
  cerr << "\t-i\t<file path>\tRead formula from file.\n";
  cerr << "\t-oinfo\t<file path>\tAppend formula information to file.\n";
  cerr << "\t-ocnf\t<file path>\tWrite CNF to file.\n";
  cerr << "\t-oproof\t<file path>\tWrite proof formula to file.\n";
  cerr << endl;
  cerr << "Pipeline options:\n";
  cerr << "\t-f\tFlat ((p|(q|r)|s) ---> (p|q|r|s)). Before DAG.\n";
  cerr << "\t-m\tMinimize DAG edges ((p&q)|(p&r&q) ---> (p&q)|((p&q)&r)).\n";
  cerr << "\t-a\t<algorithm number>\tSelect renaming algorithm.\n";
  cerr << "\t-s\tSimplify CNF. (see note 1)\n";
  cerr << "\t-ionly\tOnly compute formula information.\n";
  cerr << endl;
  cerr << "Additional options:\n";
  cerr << "\t-h\tDisplay usage mode.\n";
  cerr << "\t-syntax\tDisplay formula syntax.\n";
  cerr << endl;
  cerr << "Renaming algorithms (see note 2):\n";
  cerr << "\t1\tKnapsack 0-1 based algorithm.\n";
  cerr << "\t2\tBoy de la Tour's algorithm.\n";
  cerr << endl;
  cerr << "Notes:\n";
  cerr << "\t1) CNF simplification removes tautologies, repeated literals\n";
  cerr << "\tand repeated clauses.\n";
  cerr << "\t2) If option -a is not set or an invalid algorithm is passed,\n";
  cerr << "\tthen no renaming will happen at all.\n";
  cerr << "\t3) Formula information format:\n";
  cerr << "\t<file path>,<size>,<number of clauses>,<number of symbols>,\n";
  cerr << endl;
  exit(-1);
}

void syntax() {
  cerr << exe << ": Formula syntax.\n";
  cerr << endl;
  cerr << "Propositional symbols are strings of one or more of the following\n";
  cerr << "ASCII characters:\n";
  cerr << "\t_\t(underscore)\n";
  cerr << "\t0 to 9\t(decimal digits)\n";
  cerr << "\ta to z\t(lowercase letters)\n";
  cerr << "\tA to Z\t(uppercase letters)\n";
  cerr << endl;
  cerr << "Logic connectives are the following strings of ASCII characters:\n";
  cerr << "\t~\t(tilde)\t\t\t\t\t\tNegation.\n";
  cerr << "\t&\t(ampersand)\t\t\t\t\tConjunction.\n";
  cerr << "\t|\t(pipe)\t\t\t\t\t\tDisjunction.\n";
  cerr << "\t=>\t(equals and greater than signs)\t\t\tImplication.\n";
  cerr << "\t<=>\t(less than, equals and greater than signs)\tEquivalence.\n";
  cerr << endl;
  cerr << "Notes:\n";
  cerr << "\t1) Only one line is read by the parser.\n";
  cerr << "\t2) Whitespaces and multiple parentheses are completely ignored.\n";
  cerr << "\tExample:\n";
  cerr << "\t\t(((p => q)))\tis the same as\t\tp=>q\n";
  cerr << endl;
  exit(-1);
}

class Args {
  private:
    int argc;
    char** argv;
    unordered_map<string,int> idx;
  public:
    void init(int argc, char** argv) {
      this->argc = argc;
      this->argv = argv;
      for (int i = argc-1; 1 <= i; i--) idx[argv[i]] = i;
    }
    bool find(const string& str) const {
      return idx.find(str) != idx.end();
    }
    template <typename T>
    T opt(const string& name) const {
      auto it = idx.find(name);
      if (it == idx.end()) usage();
      int i = it->second+1;
      if (argc <= i) usage();
      stringstream ss;
      ss << argv[i];
      T ret;
      ss >> ret;
      if (ss.fail()) usage();
      return ret;
    }
    void checkflag(const string& name) const {
      if (!find(name)) usage();
    }
    template <typename T>
    void checkopt(const string& name, bool required = false) const {
      if (find(name)) opt<T>(name);
      else if (required) usage();
    }
} args;

// files
unordered_map<string,fstream*> files;
ostream& get_output_stream(const string& name, bool append = false) {
  if (!args.find("-o"+name)) return cout;
  string fn = args.opt<string>("-o"+name);
  auto it = files.find(fn);
  if (it != files.end()) return (ostream&)*it->second;
  fstream::openmode md = fstream::out;
  if (append) md = md | fstream::app;
  fstream* file = new fstream(fn,md);
  if (!file->is_open()) {
    cerr << exe << ": ";
    cerr << "error opening `" << fn << "' for " << name << " output.\n";
    exit(-1);
  }
  files[fn] = file;
  return (ostream&)*file;
}
void close_files() {
  for (auto& kv : files) {
    kv.second->close();
    delete kv.second;
  }
}

int main(int argc, char** argv) {
  // init args
  exe = argv[0];
  args.init(argc,argv);
  
  // display help
  if (args.find("-h")) usage();
  if (args.find("-syntax")) syntax();
  
  // init input stream
  string in_fn = "stdin";
  if (args.find("-i")) {
    in_fn = args.opt<string>("-i");
    if (!freopen(in_fn.c_str(),"r",stdin)) {
      cerr << exe << ": `" << in_fn << "' not found.\n";
      return -1;
    }
    reverse(in_fn.begin(),in_fn.end());
    auto pos = in_fn.find("/");
    if (pos != string::npos) in_fn = in_fn.substr(0,pos);
    reverse(in_fn.begin(),in_fn.end());
  }
  
  // init output streams
  ostream& info_stream  = get_output_stream("info", true);
  ostream& cnf_stream   = get_output_stream("cnf");
  ostream& proof_stream = get_output_stream("proof");
  
  // additional argument checks
  args.checkopt<int>("-a");
  
  // input
  getline(cin,raw);
  parse();
  stringstream ss;
  ss << T;
  string original = ss.str();
  
  // info only
  if (args.find("-ionly")) {
    info_stream << in_fn << ",";
    info_stream << size(T) << ",";
    info_stream << clauses(T) << ",";
    info_stream << symbols(T) << ",\n";
    cnf_stream << original << endl;
    close_files();
    return 0;
  }
  
  // pipeline
  nnf();
  if (args.find("-f")) flat();
  dag();
  if (args.find("-m")) mindag();
  if (args.find("-a")) switch (args.opt<int>("-a")) {
    case 1: knapsack();     break;
    case 2: boydelatour();  break;
  }
  rename();
  if (args.find("-s")) simplecnf();
  else cnf();
  
  // output
  info_stream << in_fn << ",";
  info_stream << CNF.size() << ",";
  info_stream << CNF.clauses() << ",";
  info_stream << CNF.symbols() << ",\n";
  cnf_stream << CNF << endl;
  proof_stream << "(" << original << ") <-> (" << CNF << ")\n";
  
  close_files();
  
  return 0;
}

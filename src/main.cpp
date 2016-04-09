#include "libtg.hpp"

using namespace std;

// argv[0]
string exe;

void usage() {
  cerr << "Usage mode: " << exe << " [options...] \n";
  cerr << endl;
  cerr << "Help options:\n";
  cerr << "\t-h\tDisplay usage mode.\n";
  cerr << "\t-syntax\tDisplay formula syntax.\n";
  cerr << endl;
  cerr << "I/O options:\n";
  cerr << "\t-i <file path>\t\tRead formula from file.\n";
  cerr << "\t-oformula <file path>\tWrite formula to file.\n";
  cerr << "\t-oproof <file path>\tWrite proof formula to file.\n";
  cerr << "\t-oinfo <file path>\tWrite formula information to file.\n";
  cerr << endl;
  cerr << "Pipeline options (stages are executed in the following order):\n";
  cerr << "\t-ionly\tOnly compute and output formula information and exit.\n";
  cerr << "\t-n\tPut formula in negation normal form (NNF).\n";
  cerr << "\t-f\tFlatten formula: (p|(q|r)|s) becomes (p|q|r|s).\n";
  cerr << "\t-d\tConvert formula tree to directed acyclic graph (DAG).\n";
  cerr << "\t-m\tMinimize DAG edges: (p&q)|(p&r&q) becomes (p&q)|((p&q)&r).\n";
  cerr << "\t-r <op>\tSelect a renaming option.\n";
  cerr << "\t-c <op>\tSelect a conjunctive normal form (CNF) option.\n";
  cerr << endl;
  cerr << "Renaming options: (see note 1)\n";
  cerr << "\t1\tBoy de la Tour's greedy depth-first search algorithm.\n";
  cerr << "\t2\tKnapsack 0-1 dynamic programming algorithm.\n";
  cerr << endl;
  cerr << "CNF options: (see note 1)\n";
  cerr << "\t1\tApply CNF with no simplification.\n";
  cerr << "\t2\tRemove tautologies, repeated literals and repeated clauses.\n";
  cerr << endl;
  cerr << "Knapsack renaming options:\n";
  cerr << "\t-K <n>\tChoose at most n subformulas.\n";
  cerr << "\t-Kpct\tChoose at most n\% of the subformulas (-K <n> required).\n";
  cerr << endl;
  cerr << "Notes:\n";
  cerr << "\t1) The behavior for applying renaming/CNF (-r and -c options)\n";
  cerr << "\twithout applying NNF/DAG (-n and -d options) is undefined.\n";
  cerr << "\t2) Formula information format:\n";
  cerr << "\t,<file path>,<size>,<number of clauses>,<number of symbols>,\n";
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
ostream& get_output_stream(const string& name) {
  if (!args.find("-o"+name)) return cout;
  string fn = args.opt<string>("-o"+name);
  auto it = files.find(fn);
  if (it != files.end()) return (ostream&)*it->second;
  fstream* file = new fstream(fn,fstream::out);
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
  
  // arg checks
  if (args.find("-r")) {
    int tmp = args.opt<int>("-r");
    if (tmp != 1 && tmp != 2) usage();
  }
  if (args.find("-c")) {
    int tmp = args.opt<int>("-c");
    if (tmp != 1 && tmp != 2) usage();
  }
  args.checkopt<unsigned>("-K");
  if (args.find("-Kpct")) args.opt<unsigned>("-K");
  
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
  ostream& formula_stream = get_output_stream("formula");
  ostream& proof_stream   = get_output_stream("proof");
  ostream& info_stream    = get_output_stream("info");
  
  // input
  getline(cin,raw);
  parse();
  stringstream ss;
  ss << T;
  string original = ss.str();
  
  // info only
  if (args.find("-ionly")) {
    info_stream << "," << in_fn << ",";
    info_stream << size(T) << ",";
    info_stream << clauses(T) << ",";
    info_stream << symbols(T) << ",\n";
    close_files();
    return 0;
  }
  
  // ==============
  // BEGIN pipeline
  // ==============
  
  if (args.find("-n")) nnf();
  if (args.find("-f")) flat();
  if (args.find("-d")) dag();
  if (args.find("-m")) mindag();
  
  bool renamed = false;
  if (args.find("-r")) switch (args.opt<int>("-r")) {
    case 1:
      renamed = true;
      boydelatour();
      break;
    case 2:
      renamed = true;
      knapsack(args.find("-K")?args.opt<unsigned>("-K"):0,args.find("-Kpct"));
      break;
  }
  rename();
  
  if (args.find("-c")) switch (args.opt<int>("-c")) {
    case 1: cnf();        break;
    case 2: simplecnf();  break;
  }
  
  // ============
  // END pipeline
  // ============
  
  // output
  formula_stream << FORMULA << "\n";
  if (renamed) {
    proof_stream << "(" << FORMULA << ") ";
    proof_stream << op2str(IMPL);
    proof_stream << " (" << original << ")\n";
  }
  else {
    proof_stream << "(" << FORMULA << ") ";
    proof_stream << op2str(EQUI);
    proof_stream << " (" << original << ")\n";
  }
  info_stream << "," << in_fn << ",";
  info_stream << FORMULA.size() << ",";
  info_stream << FORMULA.clauses() << ",";
  info_stream << FORMULA.symbols() << ",\n";
  
  close_files();
  
  return 0;
}

#include <nafil/file-loader.h>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace nafil;
using namespace boost;

// Load a single line from a corpus
bool FileLoader::LoadOneLine(
    istream & in, SymbolSet<int> & vocab, vector<int> & out, bool add) {
    out.clear();
    string line;
    if(!getline(in, line)) return false;
    vector<string> vec;
    algorithm::split(vec, line, is_any_of(" "));
    out.resize(vec.size());
    for(int i =0; i < (int)vec.size(); i++)
        out[i] = vocab.GetId(vec[i], add);
    return true;    
}

void FileLoader::LoadCorpus(const string & file,
                             SymbolSet<int> & vocab,
                             vector< vector<int> > & corp,
                             bool add) {
    // Ensure that the smallest vocabulary entry will be null
    vocab.GetId("", true);
    ifstream in(file.c_str());
    if(!in) THROW_ERROR("Could not open file " << file);
    vector<int> out;
    while(LoadOneLine(in, vocab, out, add))
        corp.push_back(out);
}

void FileLoader::LoadProbabilities(const string & file,
                             SymbolSet<int> & src_vocab,
                             SymbolSet<int> & trg_vocab,
                             PairProbMap & s_given_t) {
    string line;
    vector<string> vec;
    // scan the file
    ifstream in1(file.c_str());
    if(!in1) THROW_ERROR("Could not open file " << file);
    while(getline(in1, line)) {
        algorithm::split(vec, line, is_any_of(" "));
        if(vec.size() != 3) THROW_ERROR("Invalid probability " << line);
        src_vocab.GetId(vec[0], true); trg_vocab.GetId(vec[1], true);
    }
    in1.close();
    // Read and save probabilities
    ifstream in2(file.c_str());
    int tv_size = trg_vocab.size();
    while(getline(in2, line)) {
        algorithm::split(vec, line, is_any_of(" "));
        src_vocab.GetId(vec[0], true); trg_vocab.GetId(vec[1], true);
        WordPairId id = HashPair(src_vocab.GetId(vec[0], true), trg_vocab.GetId(vec[1], true), tv_size);
        s_given_t.insert(MakePair(id,atof(vec[2].c_str())));
    }
    in2.close();
}

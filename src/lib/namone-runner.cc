#include <nafil/namone-runner.h>
#include <nafil/util.h>
#include <nafil/symbol-set.h>
#include <nafil/model-one-probs.h>
#include <nafil/file-loader.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <string>
#include <set>
#include <algorithm>

using namespace nafil;
using namespace std;
using namespace boost;

// Run the model
void NamoneRunner::Run(const ConfigNamoneRunner & config) {
    string line;
    // Load the configuration
    double unk_prob = config.GetDouble("unk_prob");
    string src_data = config.GetMainArgs()[0];
    string trg_data = config.GetMainArgs()[1];
    string prob_data = config.GetMainArgs()[2];
    // Read the probabilities
    SymbolSet<int> src_vocab, trg_vocab;
    PairProbMap s_given_t;
    FileLoader::LoadProbabilities(prob_data, src_vocab, trg_vocab, s_given_t);
    int tv_size = trg_vocab.size();    
    // Read in the lines
    string src_line, trg_line;
    vector<int> src_sent, trg_sent;
    ifstream src_in(src_data.c_str()), trg_in(trg_data.c_str());
    while(true) {
        getline(src_in, src_line); getline(trg_in, trg_line); 
        if(FileLoader::LoadOneLine(src_in, src_vocab, src_sent, false) != 
           FileLoader::LoadOneLine(trg_in, trg_vocab, trg_sent, false))
            THROW_ERROR("File sizes don't match");
        double prob = ModelOneProbs::GetModelOneLogProb(src_sent, trg_sent, tv_size, s_given_t, unk_prob);
        cout << prob << endl;
    }
}

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
    bool mono = config.GetBool("mono");
    double unk_prob = config.GetDouble("unk_prob");
    const vector<string> & args = config.GetMainArgs();
    SymbolSet<int> src_vocab, trg_vocab;
    PairProbMap s_given_t;
    int tv_size;
    if(mono) {
        if(args.size() != 2)
            THROW_ERROR("Must provide exactly E_DATA and E_GIVEN_E for monolingual processing");
        FileLoader::LoadProbabilities(config.GetMainArgs()[1], src_vocab, src_vocab, s_given_t);
        tv_size = src_vocab.size();
    } else {
        if(args.size() != 3)
            THROW_ERROR("Must provide exactly F_DATA E_DATA and F_GIVEN_E for bililingual processing");
        FileLoader::LoadProbabilities(config.GetMainArgs()[2], src_vocab, trg_vocab, s_given_t);
        tv_size = trg_vocab.size();
    }
    // Read in the lines
    string src_line, trg_line;
    vector<int> src_sent, trg_sent;
    ifstream *src_in = new ifstream(config.GetMainArgs()[0].c_str()), *trg_in = NULL;
    if(!mono)
        trg_in = new ifstream(config.GetMainArgs()[1].c_str());
    while(true) {
        bool has_src = FileLoader::LoadOneLine(*src_in, src_vocab, src_sent, false);
        if(!mono) {
            bool has_trg = FileLoader::LoadOneLine(*trg_in, trg_vocab, trg_sent, false);
            if(has_src != has_trg) THROW_ERROR("File sizes don't match");
        }
        if(!has_src) break; 
        double prob = ModelOneProbs::GetModelOneLogProb(src_sent, (mono?src_sent:trg_sent),tv_size, s_given_t, unk_prob);
        if(config.GetBool("norm"))
            prob /= src_sent.size() - (mono?1:0);
        cout << prob << endl;
    }
}

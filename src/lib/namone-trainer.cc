#include <nafil/namone-trainer.h>
#include <nafil/config-namone-trainer.h>
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
void NamoneTrainer::Run(const ConfigNamoneTrainer & config) {
    string line;
    const std::vector<std::string> & args = config.GetMainArgs();
    double prob_cutoff = config.GetDouble("prob_cutoff");
    // Load the corpus
    SymbolSet<int> src_vocab, trg_vocab;
    vector< vector<int> > src_corp, trg_corp;
    FileLoader::LoadCorpus(args[0], src_vocab, src_corp);
    FileLoader::LoadCorpus(args[1], trg_vocab, trg_corp);
    if(src_corp.size() != trg_corp.size())
        THROW_ERROR("src_corp.size ("<<src_corp.size()<<") != trg_corp.size ("<<trg_corp.size()<<")");
    // Train Model one
    PairProbMap s_given_t;
    ModelOneProbs::TrainModelOne(src_corp, trg_corp, src_vocab.size(), trg_vocab.size(), s_given_t);
    // Write probabilities
    FileLoader::WriteProbabilities(cout, src_vocab, trg_vocab, s_given_t, prob_cutoff);
}

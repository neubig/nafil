#include <nafil/file-loader.h>
#include <nafil/nafil-runner.h>
#include <nafil/util.h>
#include <nafil/symbol-set.h>
#include <nafil/model-one-probs.h>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <fstream>
#include <string>
#include <algorithm>
#include <cfloat>

using namespace nafil;
using namespace std;
using namespace boost;

vector<int> NafilRunner::GetNumCognates(
    const vector<nt> & src_sent,
    const vector<int> & trg_sent,
    SymbolSet<int> & src_vocab,
    SymbolSet<int> & trg_vocab,
    int max_size) {
    vector<int> ret(max_size, 0);
    tr1::unordered_set<string> in_src, in_trg;
    const int src_len = src_sent.size(), trg_len = trg_sent.size();
    for(int i = 0; i < src_len; i++)
        in_src.insert(src_vocab.GetSymbol(src_sent[i]));
    for(int i = 0; i < trg_len; i++)
        in_trg.insert(trg_vocab.GetSymbol(trg_sent[i]));
    for(tr1::unordered_set<string>::const_iterator it = in_src.begin();
            it != in_src.end(); it++) {
        // TODO: Figure out why length is zero
        if(it->length() > 0 && in_trg.find(*it) != in_trg.end())
            ret[min(max_size,(int)it->length())-1]++;
    }
    return ret;
}

void SwapCorpus(double ratio,
                vector< vector<int> > & src_corp,
                vector< vector<int> > & trg_corp,
                vector< bool > & correct) {
    if(ratio <= 0.0) return;
    int thresh = RAND_MAX * ratio;
    int len = src_corp.size();
    for(int i = 1; i < len; i++) {
        if(rand() < thresh) {
            correct[i] = false; correct[i-1] = false;
            swap(trg_corp[i], trg_corp[i-1]);
        }
    }
}

int NafilRunner::CalculateMonotonicity(const vector<pair<int,int> > & merged) {
    const int merged_len = merged.size();
    int ret = 0;
    for(int i = 1; i < merged_len; i++)
        if(merged[i-1].second <= merged[i].second)
            ret++;
    return ret;
}
int NafilRunner::CalculateDistortion(const vector<pair<int,int> > & merged) {
    const int merged_len = merged.size();
    int ret = 0;
    for(int i = 1; i < merged_len; i++)
        ret += abs(merged[i-1].second - merged[i].second);
    return ret;
}

void NafilRunner::MakeFeatures(
            const vector<int> & src_sent,
            const vector<int> & trg_sent,
            SymbolSet<int> & src_vocab,
            SymbolSet<int> & trg_vocab,
            const PairProbMap & s_given_t,
            const PairProbMap & t_given_s,
            const ConfigNafilRunner & config,
            vector<double> & feats) {
    int src_len = src_sent.size();
    int trg_len = trg_sent.size();
    int src_size = src_vocab.size();
    int trg_size = trg_vocab.size();
    int min_len = min(src_len, trg_len);
    int max_len = max(src_len, trg_len);
    // Make length ratio feature
    if(config.GetBool("use_len_ratio")) {
        feats.push_back((double)max_len/min_len-1.0);
        feats.push_back((double)src_len/trg_len-1.0);
        feats.push_back(src_len > trg_len ? 1 : 0);
        feats.push_back(trg_len > src_len ? 1 : 0);
        feats.push_back(src_len == trg_len ? 1 : 0);
    }
    // Make model one probability feature
    if(config.GetBool("use_model_one")) {
        double sgt_prob = ModelOneProbs::GetModelOneLogProb(src_sent, trg_sent, trg_size, s_given_t)/src_len;
        double tgs_prob = ModelOneProbs::GetModelOneLogProb(trg_sent, src_sent, src_size, t_given_s)/trg_len;
        feats.push_back(sgt_prob);
        feats.push_back(tgs_prob);
    }
    // Make alignment probability features
    if(config.GetBool("use_alignments")) {
        vector<int> s_align_t = ModelOneProbs::GetModelOneAlignments(src_sent, trg_sent, trg_size, s_given_t);
        vector<int> t_align_s = ModelOneProbs::GetModelOneAlignments(trg_sent, src_sent, src_size, t_given_s);
        vector<pair<int,int> > merged = ModelOneProbs::MergeIntersect(s_align_t, t_align_s);
        int merged_size = merged.size();
        feats.push_back(merged_size/(double)min_len);
        feats.push_back((min_len-merged_size)/(double)min_len);
        feats.push_back(merged_size);
        int possible_monotonic = max(0,merged_size-1);
        int num_monotonic = CalculateMonotonicity(merged);
        int distortion = CalculateDistortion(merged);
        feats.push_back(possible_monotonic ? (2*num_monotonic-possible_monotonic)/(double)possible_monotonic : 0);
        feats.push_back(num_monotonic);
        feats.push_back(possible_monotonic ? 1 : 0);
        feats.push_back(possible_monotonic ? distortion/(double)possible_monotonic : 0);
    }
    if(config.GetBool("use_cognates")) {
        vector<int> cognates = GetNumCognates(src_sent, trg_sent, src_vocab, trg_vocab, 4);
        for(int i = 0; i < (int)cognates.size(); i++)
            feats.push_back(cognates[i]);
    }
}

// Run the model
void NafilRunner::Run(const ConfigNafilRunner & config) {
    srand(time(NULL));
    const std::vector<std::string> & args = config.GetMainArgs();
    // Load the corpus
    SymbolSet<int> src_vocab, trg_vocab;
    vector< vector<int> > src_corp, trg_corp;
    FileLoader::LoadCorpus(args[0], src_vocab, src_corp);
    FileLoader::LoadCorpus(args[1], trg_vocab, trg_corp);
    if(src_corp.size() != trg_corp.size())
        THROW_ERROR("src_corp.size ("<<src_corp.size()<<") != trg_corp.size ("<<trg_corp.size()<<")");
    int len = src_corp.size();
    // Swap part of the corpus to create artificial negative samples
    vector<bool> correct(src_corp.size(), true);
    SwapCorpus(config.GetDouble("swap"), src_corp, trg_corp, correct);
    // Train model one in both directions
    PairProbMap s_given_t, t_given_s;
    if(config.GetBool("use_alignments") || config.GetBool("use_model_one")) {
        ModelOneProbs::TrainModelOne(src_corp, trg_corp, src_vocab.size(), trg_vocab.size(), s_given_t);
        ModelOneProbs::TrainModelOne(trg_corp, src_corp, trg_vocab.size(), src_vocab.size(), t_given_s);
    }
    // Make the values
    cerr << "Printing features" << endl;
    for(int i = 0; i < len; i++) {
        vector<double> feats;
        MakeFeatures(src_corp[i], trg_corp[i], src_vocab, trg_vocab, s_given_t, t_given_s, config, feats);
        cout << (correct[i]?1:-1);
        for(int j = 0; j < (int)feats.size(); j++)
            if(feats[j])
                cout << " " << j+1 << ":" << feats[j];
        cout << endl;
    }
}

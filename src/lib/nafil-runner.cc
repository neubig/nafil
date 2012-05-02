#include <nafil/nafil-runner.h>
#include <nafil/util.h>
#include <nafil/symbol-set.h>
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

void NafilRunner::LoadCorpus(const string & file,
                             SymbolSet<int> & vocab,
                             vector< vector<int> > & corp) {
    // Ensure that the smallest vocabulary entry will be null
    vocab.GetId("", true);
    ifstream in(file.c_str());
    if(!in) THROW_ERROR("Could not open file " << file);
    string line;
    while(getline(in, line)) {
        vector<string> vec;
        algorithm::split(vec, line, is_any_of(" "));
        corp.push_back(vector<int>());
        BOOST_FOREACH(string str, vec) {
            corp.rbegin()->push_back(vocab.GetId(str, true));
        }
    }
}

vector<int> NafilRunner::GetNumCognates(
    const vector<int> & src_sent,
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
        if(in_trg.find(*it) != in_trg.end())
            ret[min(max_size,(int)it->length())-1]++;
    }
    return ret;
}
 
// Calculate model one probabilities and put them in conds
//  conds[pair<int,int>(e,f)] = p(e|f)
void NafilRunner::TrainModelOne(const vector< vector<int> > & es, 
                                const vector< vector<int> > & fs, 
                                int eSize, int fSize,
                                PairProbMap & conds) {
    PairProbMap count;
    std::vector<double> total(fSize), tBuff;
    std::vector< WordPairId > pBuff;
    // initialize the probability
    std::cerr << "Initializing model 1" << std::endl;
    int i,j,k,iter=0;
    double uniProb = 1.0/eSize;
    for(i = 0; i < (int)es.size(); i++) {
        for(j = 0; j < (int)es[i].size(); j++) {
            for(k = 0; k < (int)fs[i].size(); k++) {
                WordPairId id = HashPair(es[i][j],fs[i][k],fSize);
                conds.insert(PairProbMap::value_type(id,uniProb));
                count.insert(PairProbMap::value_type(id,0.0));
            }
            WordPairId id = HashPair(es[i][j],0,fSize);
            conds.insert(PairProbMap::value_type(id,uniProb));
            count.insert(PairProbMap::value_type(id,0.0));
        }
    }
    cerr << "Finished initializing, starting training" << endl;
    // train the model
    int maxIters = 100;
    double lastLik = 0.0, likCut = 0.001, sTotal, lik = 0.0, norm;
    do {
        // reset the values
        lastLik = lik;
        lik = 0.0;
        for(PairProbMap::iterator it = count.begin(); it != count.end(); it++)
            it->second = 0.0;
        fill(total.begin(),total.end(),0.0);
        // E step
        for(i = 0; i < (int)es.size(); i++) {
            const int esSize = es[i].size(), fsSize = fs[i].size();
            if(esSize*fsSize == 0)
                continue;
            if((int)pBuff.size() <= fsSize) { 
                pBuff.resize(fsSize+1); 
                tBuff.resize(fsSize+1); 
            }
            for(j = 0; j < esSize; j++) {
                sTotal = 0;
                // do words + null
                for(k = 0; k < fsSize; k++) {
                    pBuff[k] = HashPair(es[i][j],fs[i][k],fSize);
                    tBuff[k] = conds.find(pBuff[k])->second;
                    sTotal += tBuff[k];
                }
                pBuff[k] = HashPair(es[i][j],0,fSize);
                tBuff[k] = conds.find(pBuff[k])->second;
                sTotal += tBuff[k];
                // likelihood
                lik += log(sTotal/(fsSize+1));
                // do words + null
                for(k = 0; k < (int)fsSize+1; k++) {
                    norm = tBuff[k]/sTotal;
                    count[pBuff[k]] += norm;
                    SafeAccess(total, pBuff[k]%fSize) += norm;
                }
            }
        }
        // M step
        //  divide the number of times it->first.second generates it->first.first divided
        //  by the total number of times it->first.second appears
        for(PairProbMap::iterator it = count.begin(); it != count.end(); it++)
            conds[it->first] = it->second/SafeAccess(total, it->first%fSize);
        std::cerr << " Iteration " << ++iter << ": likelihood "<<lik<<std::endl;
    } while((lastLik == 0.0 || (lastLik-lik) < lik * likCut) && --maxIters > 0);
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
vector<pair<int,int> > NafilRunner::MergeIntersect(
            const vector<int> & src_align,
            const vector<int> & trg_align) {
    vector<pair<int,int> > ret;
    const int src_len = src_align.size();
    for(int i = 0; i < src_len; i++)
        if(src_align[i] != -1 && trg_align[src_align[i]] == i)
            ret.push_back(MakePair(i,src_align[i]));
    return ret;
}

vector<int> NafilRunner::GetModelOneAlignments(
            const vector<int> & src_sent,
            const vector<int> & trg_sent,
            int trg_size,
            const PairProbMap & s_given_t) {
    const int src_len = src_sent.size(), trg_len = trg_sent.size();
    vector<int> ret(src_len,-1);
    for(int i = 0; i < src_len; i++) {
        PairProbMap::const_iterator it = s_given_t.find(HashPair(src_sent[i], 0, trg_size));
        double max_prob = (it == s_given_t.end() ? -DBL_MAX : it->second);
        for(int j = 0; j < trg_len; j++) {
            it = s_given_t.find(HashPair(src_sent[i],trg_sent[j],trg_size));
            if(it != s_given_t.end() && it->second > max_prob) {
                max_prob = it->second;
                ret[i] = j;
            }
        }
    }
    return ret; 
}

double NafilRunner::GetModelOneLogProb(
            const vector<int> & src_sent,
            const vector<int> & trg_sent,
            int trg_size,
            const PairProbMap & s_given_t) {
    int src_len = src_sent.size();
    int trg_len = trg_sent.size();
    double ret = 0;
    PairProbMap::const_iterator it;
    for(int i = 0; i < src_len; i++) {
        double curr_prob = 0;
        // Get the null probability
        it = s_given_t.find(HashPair(src_sent[i],0,trg_size));
        if(it != s_given_t.end()) curr_prob += it->second;
        // Get the other probabilities
        for(int j = 0; j < trg_len; j++) {
            it = s_given_t.find(HashPair(src_sent[i],trg_sent[j],trg_size));
            if(it != s_given_t.end()) curr_prob += it->second;
        }
        ret += log(curr_prob / (trg_len+1));
    }
    return ret;
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
        double sgt_prob = GetModelOneLogProb(src_sent, trg_sent, trg_size, s_given_t)/src_len;
        double tgs_prob = GetModelOneLogProb(trg_sent, src_sent, src_size, t_given_s)/trg_len;
        feats.push_back(sgt_prob);
        feats.push_back(tgs_prob);
    }
    // Make alignment probability features
    if(config.GetBool("use_alignments")) {
        vector<int> s_align_t = GetModelOneAlignments(src_sent, trg_sent, trg_size, s_given_t);
        vector<int> t_align_s = GetModelOneAlignments(trg_sent, src_sent, src_size, t_given_s);
        vector<pair<int,int> > merged = MergeIntersect(s_align_t, t_align_s);
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
    LoadCorpus(args[0], src_vocab, src_corp);
    LoadCorpus(args[1], trg_vocab, trg_corp);
    if(src_corp.size() != trg_corp.size())
        THROW_ERROR("src_corp.size ("<<src_corp.size()<<") != trg_corp.size ("<<trg_corp.size()<<")");
    int len = src_corp.size();
    // Swap part of the corpus to create artificial negative samples
    vector<bool> correct(src_corp.size(), true);
    SwapCorpus(config.GetDouble("swap"), src_corp, trg_corp, correct);
    // Train model one in both directions
    PairProbMap s_given_t, t_given_s;
    if(config.GetBool("use_alignments") || config.GetBool("use_model_one")) {
        TrainModelOne(src_corp, trg_corp, src_vocab.size(), trg_vocab.size(), s_given_t);
        TrainModelOne(trg_corp, src_corp, trg_vocab.size(), src_vocab.size(), t_given_s);
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

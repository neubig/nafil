
#include <nafil/util.h>
#include <nafil/model-one-probs.h>
#include <boost/foreach.hpp>
#include <iostream>
#include <cfloat>

using namespace nafil;
using namespace std;
using namespace boost;
 
// Calculate model one probabilities and put them in conds
//  conds[pair<int,int>(e,f)] = p(e|f)
void ModelOneProbs::TrainModelOne(const vector< vector<int> > & es, 
                                const vector< vector<int> > & fs, 
                                int eSize, int fSize,
                                PairProbMap & conds,
                                bool monolingual) {
    if(monolingual && (&es != &fs))
        THROW_ERROR("Attempting monolingual model one training with non-identical training corpora");
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
                if(monolingual && j == k)
                    continue;
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
                    if(monolingual && j == k) continue;
                    pBuff[k] = HashPair(es[i][j],fs[i][k],fSize);
                    tBuff[k] = conds.find(pBuff[k])->second;
                    sTotal += tBuff[k];
                }
                pBuff[k] = HashPair(es[i][j],0,fSize);
                tBuff[k] = conds.find(pBuff[k])->second;
                sTotal += tBuff[k];
                // likelihood
                lik += log(sTotal/(fsSize+(monolingual?0:1)));
                // do words + null
                for(k = 0; k < (int)fsSize+1; k++) {
                    if(monolingual && j == k) continue;
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

vector<pair<int,int> > ModelOneProbs::MergeIntersect(
            const vector<int> & src_align,
            const vector<int> & trg_align) {
    vector<pair<int,int> > ret;
    const int src_len = src_align.size();
    for(int i = 0; i < src_len; i++)
        if(src_align[i] != -1 && trg_align[src_align[i]] == i)
            ret.push_back(MakePair(i,src_align[i]));
    return ret;
}

vector<int> ModelOneProbs::GetModelOneAlignments(
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

double ModelOneProbs::GetModelOneLogProb(
            const vector<int> & src_sent,
            const vector<int> & trg_sent,
            int trg_size,
            const PairProbMap & s_given_t,
            double unk_prob) {
    int src_len = src_sent.size();
    int trg_len = trg_sent.size();
    double ret = 0;
    PairProbMap::const_iterator it;
    for(int i = 0; i < src_len; i++) {
        double curr_prob = 0;
        // Get the null probability
        it = s_given_t.find(HashPair(src_sent[i],0,trg_size));
        curr_prob += (it != s_given_t.end() ? it->second : unk_prob);
        // Get the other probabilities
        for(int j = 0; j < trg_len; j++) {
            it = s_given_t.find(HashPair(src_sent[i],trg_sent[j],trg_size));
            if(it != s_given_t.end()) curr_prob += it->second;
        }
        ret += log(curr_prob / (trg_len+1));
    }
    return ret;
}

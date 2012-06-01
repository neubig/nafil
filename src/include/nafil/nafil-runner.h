#ifndef NAFIL_RUNNER_H__ 
#define NAFIL_RUNNER_H__

#include <iostream>
#include <fstream>
#include <nafil/config-nafil-runner.h>
#include <nafil/symbol-set.h>
#include <nafil/model-one-probs.h>
#include <tr1/unordered_map>

namespace nafil {

// A class to build features for the filterer
class NafilRunner {
public:

    NafilRunner() { }
    ~NafilRunner() { }
    
    // Run the model
    void Run(const ConfigNafilRunner & config);

    int CalculateMonotonicity(const std::vector<std::pair<int,int> > & merged);
    int CalculateDistortion(const std::vector<std::pair<int,int> > & merged);

    void MakeFeatures(
            const std::vector<int> & src_sent,
            const std::vector<int> & trg_sent,
            nafil::SymbolSet<int> & src_vocab,
            nafil::SymbolSet<int> & trg_vocab,
            const PairProbMap & s2t,
            const PairProbMap & t2s,
            const ConfigNafilRunner & config,
            std::vector<double> & feats);

    std::vector<int> GetNumCognates(
            const std::vector<int> & src_sent,
            const std::vector<int> & trg_sent,
            nafil::SymbolSet<int> & src_vocab,
            nafil::SymbolSet<int> & trg_vocab,
            int max_size);

private:

};

}

#endif


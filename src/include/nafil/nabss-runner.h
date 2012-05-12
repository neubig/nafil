#ifndef NABSS_RUNNER_H__ 
#define NABSS_RUNNER_H__

#include <iostream>
#include <fstream>
#include <nafil/config-nabss-runner.h>
#include <nafil/symbol-set.h>
#include <tr1/unordered_map>

namespace nafil {

// A class to build features for the filterer
class NabssRunner {
public:

    typedef std::pair<int,int> PairIntInt;
    typedef std::tr1::unordered_map<int, int> NGramMap;

    NabssRunner() { }
    ~NabssRunner() { }

    int AddNGramCounts(
        const std::string & line,
        int n_gram_len,
        NGramMap & n_gram_counts,
        SymbolSet<int> & n_gram_ids,
        bool add);

    int CalculateCoverageScore(
        const NGramMap & n_gram_counts,
        const NGramMap & my_counts,
        int threshold);
    
    // Run the model
    void Run(const ConfigNabssRunner & config);

private:

};

}

#endif


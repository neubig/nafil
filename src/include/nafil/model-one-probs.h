#ifndef MODEL_ONE_PROBS_H__ 
#define MODEL_ONE_PROBS_H__

#include <vector>
#include <nafil/util.h>

namespace nafil {
    
class ModelOneProbs {

public:
    // Train model one probabilities given a corpus pair
    // If monolingual is false, all vectors in es and fs must be identical
    // and words in the identical position in the vector will not be used
    static void TrainModelOne(
                       const std::vector< std::vector<int> > & es, 
                       const std::vector< std::vector<int> > & fs, 
                       int eSize, int fSize,
                       PairProbMap & conds,
                       bool monolingual = false);

    static double GetModelOneLogProb(
                const std::vector<int> & src_sent,
                const std::vector<int> & trg_sent,
                int trg_size,
                const PairProbMap & s_given_t,
                double unk_prob = 1e-10
                );

    static std::vector<int> GetModelOneAlignments(
            const std::vector<int> & src_sent,
            const std::vector<int> & trg_sent,
            int trg_size,
            const PairProbMap & s_given_t);

    static std::vector<std::pair<int,int> > MergeIntersect(
            const std::vector<int> & src_align,
            const std::vector<int> & trg_align);

};

}

#endif

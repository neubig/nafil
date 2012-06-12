#ifndef TEST_MODEL_ONE_PROBS_H__
#define TEST_MODEL_ONE_PROBS_H__

#include "test-base.h"
#include <nafil/util.h>
#include <nafil/model-one-probs.h>

namespace nafil {

class TestModelOneProbs : public TestBase {

public:

    TestModelOneProbs() { }
    ~TestModelOneProbs() { }

    int TestModelOneProb() {
        vector<int> src_sent(2,1), trg_sent(2,1);
        src_sent[1] = 2; trg_sent[1] = 2;
        PairProbMap s_given_t;
        int trg_size = 4;
        s_given_t.insert(MakePair(HashPair(1,0,trg_size), 0.1));
        s_given_t.insert(MakePair(HashPair(1,2,trg_size), 0.2));
        s_given_t.insert(MakePair(HashPair(1,3,trg_size), 0.2));
        s_given_t.insert(MakePair(HashPair(2,0,trg_size), 0.1));
        s_given_t.insert(MakePair(HashPair(2,1,trg_size), 0.5));
        double exp = log(0.1*0.2);
        double act = ModelOneProbs::GetModelOneLogProb(src_sent, trg_sent, trg_size, s_given_t);
        return CheckAlmost(exp, act);
    }

    int TestModelOneViterbi() {
        vector<int> src_sent(3,1), trg_sent(2,1);
        src_sent[1] = 2; src_sent[2] = 3; trg_sent[1] = 2;
        int trg_size = 4;
        PairProbMap s_given_t;
        s_given_t.insert(MakePair(HashPair(1,0,trg_size), 0.1));
        s_given_t.insert(MakePair(HashPair(1,2,trg_size), 0.2));
        s_given_t.insert(MakePair(HashPair(1,3,trg_size), 0.2));
        s_given_t.insert(MakePair(HashPair(2,0,trg_size), 0.1));
        s_given_t.insert(MakePair(HashPair(2,1,trg_size), 0.5));
        s_given_t.insert(MakePair(HashPair(3,0,trg_size), 0.1));
        vector<int> exp(3,-2), act = ModelOneProbs::GetModelOneAlignments(src_sent, trg_sent, trg_size, s_given_t);
        exp[0] = 1;
        exp[1] = 0;
        exp[2] = -1;
        return CheckVector(exp, act);
    }
    
    int TestTrainOverflow() {
        vector<vector<int> > es, fs;
        int size = 600000;
        es.push_back(vector<int>(1,1));
        es.push_back(vector<int>(1,500000));
        fs.push_back(vector<int>(1,1));
        fs.push_back(vector<int>(1,500000));
        PairProbMap conds;
        ModelOneProbs::TrainModelOne(es,fs,size,size,conds);
        int ret = 1;
        double prob1 = conds[HashPair(1,1,size)];
        double prob2 = conds[HashPair(500000,500000,size)];
        if(prob1 != prob2) {
            cerr << "Probs don't match: " << prob1 << " != " <<prob2 << endl;
            ret = 0;
        }
        return ret;
    }

    int TestTrainMono() {
        vector<vector<int> > es;
        int size = 3;
        vector<int> sent(2,0);
        sent[0] = 1; sent[1] = 2;
        es.push_back(sent);
        PairProbMap conds;
        ModelOneProbs::TrainModelOne(es,es,size,size,conds,true);
        int ret = 1;
        double prob12 = conds[HashPair(1,2,size)];
        double prob21 = conds[HashPair(2,1,size)];
        if(prob12 != prob21) {
            cerr << "Probs don't match: " << prob12 << " != " <<prob21 << endl;
            ret = 0;
        }
        PairProbMap::const_iterator it11 = conds.find(HashPair(1,1,size));
        PairProbMap::const_iterator it22 = conds.find(HashPair(2,2,size));
        if(it11 != conds.end()) {
            cerr << "Found P(1|1) == " << *it11 << endl;
            ret = 0;
        }
        if(it22 != conds.end()) {
            cerr << "Found P(2|2) == " << *it22 << endl;
            ret = 0;
        }
        return ret;
    }

    int TestMergeIntersect() {
        vector<int> s_given_t(3,0), t_given_s(2,0);
        s_given_t[2] = 1; t_given_s[1] = 2;
        vector<pair<int,int> > exp, act = ModelOneProbs::MergeIntersect(s_given_t, t_given_s);
        exp.push_back(MakePair(0,0));
        exp.push_back(MakePair(2,1));
        return CheckVector(exp, act);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestModelOneProb()" << endl; if(TestModelOneProb()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestModelOneViterbi()" << endl; if(TestModelOneViterbi()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestMergeIntersect()" << endl; if(TestMergeIntersect()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestTrainOverflow()" << endl; if(TestTrainOverflow()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestTrainMono()" << endl; if(TestTrainMono()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestModelOneProbs Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:

};

}

#endif

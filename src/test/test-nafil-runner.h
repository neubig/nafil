#ifndef TEST_NAFIL_RUNNER_H__
#define TEST_NAFIL_RUNNER_H__

#include "test-base.h"
#include <nafil/nafil-runner.h>

namespace nafil {

class TestNafilRunner : public TestBase {

public:

    TestNafilRunner() { }
    ~TestNafilRunner() { }

    int TestGetNumCognates() {
        vector<int> src_sent(3,0), trg_sent(3,0);
        SymbolSet<int> src_vocab, trg_vocab;
        src_sent[0] = src_vocab.GetId("hello", true);
        src_sent[1] = src_vocab.GetId("joseph", true);
        src_sent[2] = src_vocab.GetId("!", true);
        trg_sent[0] = trg_vocab.GetId("hola", true);
        trg_sent[1] = trg_vocab.GetId("joseph", true);
        trg_sent[2] = trg_vocab.GetId("!", true);
        vector<int> num = nr.GetNumCognates(src_sent, trg_sent, src_vocab, trg_vocab, 4);
        vector<int> exp(4,0);
        exp[0] = 1;
        exp[3] = 1;
        return CheckVector(exp,num);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestGetNumCognates()" << endl; if(TestGetNumCognates()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestNafilRunner Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    NafilRunner nr;

};

}

#endif

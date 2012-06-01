#ifndef TEST_FILE_LOADER_H__
#define TEST_FILE_LOADER_H__

#include "test-base.h"
#include <nafil/file-loader.h>
#include <fstream>

namespace nafil {

class TestFileLoader : public TestBase {

public:

    TestFileLoader() { }
    ~TestFileLoader() { }

    int TestLoadCorpus() {
        ofstream out("/tmp/test-nafil.txt");
        out << "hello goodbye\nhello birdy\n";
        out.close();
        vector< vector<int> > exp(2,vector<int>(2,1)), act;
        exp[0][1] = 2; exp[1][1] = 3;
        SymbolSet<int> vocab;
        nr.LoadCorpus("/tmp/test-nafil.txt", vocab, act);
        int ret = 1;
        if(act.size() != exp.size()) {
            cerr << "act.size (" << act.size() << ") != exp.size (" <<exp.size()<<")"<<endl;
            ret = 0;
        } else {
            for(int i = 0; i < (int)exp.size(); i++)
                ret = min(ret, CheckVector(exp[i], act[i]));
        }
        return ret;
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestLoadCorpus()" << endl; if(TestLoadCorpus()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestFileLoader Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:
    FileLoader nr;

};

}

#endif

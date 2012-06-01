#ifndef FILE_LOADER_H__
#define FILE_LOADER_H__

#include <vector>
#include <nafil/symbol-set.h>

namespace nafil {

class FileLoader {

public:

    static bool LoadOneLine(std::istream & in, 
                            SymbolSet<int> & vocab, 
                            std::vector<int> & out, 
                            bool add = true);

    static void LoadCorpus(const std::string & file,
                    nafil::SymbolSet<int> & vocab,
                    std::vector< std::vector<int> > & corp,
                    bool add = true);

    static void LoadProbabilities(const std::string & file,
                             SymbolSet<int> & src_vocab,
                             SymbolSet<int> & trg_vocab,
                             PairProbMap & s_given_t);

};

}

#endif

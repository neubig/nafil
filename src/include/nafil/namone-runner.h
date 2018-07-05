#ifndef NAMONE_RUNNER_H__ 
#define NAMONE_RUNNER_H__

#include <iostream>
#include <fstream>
#include <nafil/config-namone-runner.h>
#include <nafil/symbol-set.h>
#include <unordered_map>

namespace nafil {

// A class to build features for the filterer
class NamoneRunner {
public:

    NamoneRunner() { }
    ~NamoneRunner() { }

    // Run the model
    void Run(const ConfigNamoneRunner & config);

private:

};

}

#endif


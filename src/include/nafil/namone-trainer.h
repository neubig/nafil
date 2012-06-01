#ifndef NAMONE_TRAINER_H__ 
#define NAMONE_TRAINER_H__

#include <iostream>
#include <fstream>
#include <nafil/config-namone-trainer.h>
#include <nafil/symbol-set.h>
#include <tr1/unordered_map>

namespace nafil {

// A class to build features for the filterer
class NamoneTrainer {
public:

    NamoneTrainer() { }
    ~NamoneTrainer() { }

    // Run the model
    void Run(const ConfigNamoneTrainer & config);

private:

};

}

#endif


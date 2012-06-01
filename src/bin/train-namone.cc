#include <nafil/config-namone-trainer.h>
#include <nafil/namone-trainer.h>

using namespace nafil;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigNamoneTrainer conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    NamoneTrainer trainer;
    trainer.Run(conf);
}

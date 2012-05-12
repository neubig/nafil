#include <nafil/config-nabss-runner.h>
#include <nafil/nabss-runner.h>

using namespace nafil;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigNabssRunner conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    NabssRunner runner;
    runner.Run(conf);
}

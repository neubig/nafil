#include <nafil/config-nafil-runner.h>
#include <nafil/nafil-runner.h>

using namespace nafil;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigNafilRunner conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    NafilRunner runner;
    runner.Run(conf);
}

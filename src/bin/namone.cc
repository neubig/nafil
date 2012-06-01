#include <nafil/config-namone-runner.h>
#include <nafil/namone-runner.h>

using namespace nafil;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigNamoneRunner conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    NamoneRunner runner;
    runner.Run(conf);
}

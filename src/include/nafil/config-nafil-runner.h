#ifndef CONFIG_NAFIL_RUNNER_H__
#define CONFIG_NAFIL_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <nafil/util.h>
#include <nafil/config-base.h>

namespace nafil {

class ConfigNafilRunner : public ConfigBase {

public:

    ConfigNafilRunner() : ConfigBase() {
        minArgs_ = 2;
        maxArgs_ = 2;

        SetUsage(
"~~~ nafil ~~~\n"
"  by Graham Neubig\n"
"\n"
"Calculates features for a text filterer.\n"
"  Usage: nafil [INPUT] [FEATURES]\n"
);

        AddConfigEntry("swap", "0.0", "The fraction of sentences to swap");
        AddConfigEntry("use_len_ratio", "true", "Use length ratio features");
        AddConfigEntry("use_model_one", "true", "Use model one features");
        AddConfigEntry("use_alignments", "true", "Use alignment features");
        AddConfigEntry("use_cognates", "true", "Use cognate features");

    }
	
};

}

#endif

#ifndef CONFIG_NABSS_RUNNER_H__
#define CONFIG_NABSS_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <nafil/util.h>
#include <nafil/config-base.h>

namespace nafil {

class ConfigNabssRunner : public ConfigBase {

public:

    ConfigNabssRunner() : ConfigBase() {
        minArgs_ = 2;
        maxArgs_ = 2;

        SetUsage(
"~~~ nabss ~~~\n"
"  by Graham Neubig\n"
"\n"
"Performs extremely simple bilingual sentence filtering, keeping only sentences\n"
"with unusual n-grams.\n"
"  Usage: nabss [IN_DOMAIN] [OUT_DOMAIN] > [OUTPUT]\n"
);

        AddConfigEntry("n_gram_len", "3", "The max length of the n-grams to consider");
        AddConfigEntry("threshold", "5", "The threshold for unusual n-grams");
        AddConfigEntry("process_n", "1000000", "Only consider the top n sentences");
        AddConfigEntry("output_n", "50000", "Only output the top n sentences");

    }
	
};

}

#endif

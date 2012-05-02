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
        minArgs_ = 3;
        maxArgs_ = 4;

        SetUsage(
"~~~ nabss ~~~\n"
"  by Graham Neubig\n"
"\n"
"Performs extremely simple bilingual sentence filtering, keeping only sentences\n"
"with unusual n-grams.\n"
"  Usage: nabss < [INPUT]\n"
);

        AddConfigEntry("threshold", "5", "The threshold for unusual n-grams");
        AddConfigEntry("use_len_ratio", "true", "Use length ratio features");
        AddConfigEntry("use_model_one", "true", "Use model one features");

    }
	
};

}

#endif

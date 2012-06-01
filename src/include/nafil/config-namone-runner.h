#ifndef CONFIG_NAMONE_RUNNER_H__
#define CONFIG_NAMONE_RUNNER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <nafil/util.h>
#include <nafil/config-base.h>

namespace nafil {

class ConfigNamoneRunner : public ConfigBase {

public:

    ConfigNamoneRunner() : ConfigBase() {
        minArgs_ = 3;
        maxArgs_ = 3;

        SetUsage(
"~~~ namone ~~~\n"
"  by Graham Neubig\n"
"\n"
"Calculates IBM Model 1 probabilities for sentence pairs.\n"
"  Usage: namone F_FILE E_FILE F_GIVEN_E_PROBS > OUTPUT\n"
);

        AddConfigEntry("unk_prob", "1e-7", "The probability to assign to unknown words");

    }
	
};

}

#endif

#ifndef CONFIG_NAMONE_TRAINER_H__
#define CONFIG_NAMONE_TRAINER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <nafil/util.h>
#include <nafil/config-base.h>

namespace nafil {

class ConfigNamoneTrainer : public ConfigBase {

public:

    ConfigNamoneTrainer() : ConfigBase() {
        minArgs_ = 2;
        maxArgs_ = 2;

        SetUsage(
"~~~ train-namone ~~~\n"
"  by Graham Neubig\n"
"\n"
"Trains IBM Model 1 using a parallel corpus.\n"
"  Usage: namone F_FILE E_FILE > OUTPUT\n"
);

        AddConfigEntry("prob_cutoff", "1e-7", "Don't output any values less than this");

    }
	
};

}

#endif

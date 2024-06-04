// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "BayModule.hpp"

struct BayNorm : BayOutputModule {

  BayNorm() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i < OUTPUTS_LEN; i++) {
      configInput(POLY_INPUT+i, string::f("Port %d", i + 1));
      configOutput(POLY_OUTPUT+i, string::f("Port %d", i + 1));
    }
  }

  void process(const ProcessArgs& args) override {
    BayModule::process(args);
  }
  
};

struct BayNormWidget : VenomWidget {
  BayNormWidget(BayNorm* module) {
    setModule(module);
    setVenomPanel("BayNorm");

    for (int i=0; i<BayNorm::OUTPUTS_LEN; i++) {
      addInput(createInputCentered<PolyPort>(Vec(21.5f,48.5f+i*42), module, BayNorm::POLY_INPUT + i));
      addOutput(createOutputCentered<PolyPort>(Vec(53.5f,48.5f+i*42), module, BayNorm::POLY_OUTPUT + i));
    }
  }

};

Model* modelBayNorm = createModel<BayNorm, BayNormWidget>("BayNorm");

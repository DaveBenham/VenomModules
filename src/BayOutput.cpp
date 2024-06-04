// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "BayModule.hpp"

struct BayOutput : BayOutputModule {

  BayOutput() {
    venomConfig(PARAMS_LEN, 0, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i < OUTPUTS_LEN; i++) {
      configOutput(POLY_OUTPUT+i, string::f("Port %d", i + 1));
    }
  }

  void process(const ProcessArgs& args) override {
    BayModule::process(args);
  }
  
};

struct BayOutputWidget : VenomWidget {
  BayOutputWidget(BayOutput* module) {
    setModule(module);
    setVenomPanel("BayOutput");

    for (int i=0; i<BayOutput::OUTPUTS_LEN; i++) {
      addOutput(createOutputCentered<PolyPort>(Vec(30.f,48.5f+i*42), module, BayOutput::POLY_OUTPUT + i));
    }
  }

};

Model* modelBayOutput = createModel<BayOutput, BayOutputWidget>("BayOutput");

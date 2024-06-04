// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "BayModule.hpp"

struct BayInput : BayInputModule {

  BayInput() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, 0, LIGHTS_LEN);
    for (int i=0; i < INPUTS_LEN; i++) {
      configInput(POLY_INPUT+i, string::f("Port %d", i + 1));
    }
  }

  void process(const ProcessArgs& args) override {
    BayModule::process(args);
  }
  
};

struct BayInputWidget : VenomWidget {
  BayInputWidget(BayInput* module) {
    setModule(module);
    setVenomPanel("BayInput");

    for (int i=0; i<8; i++) {
      addInput(createInputCentered<PolyPort>(Vec(30.f,48.5f+i*42), module, BayInput::POLY_INPUT + i));
    }
  }

};

Model* modelBayInput = createModel<BayInput, BayInputWidget>("BayInput");

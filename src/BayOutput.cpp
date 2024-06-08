// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "BayModule.hpp"

struct BayOutput : BayOutputModule {

  BayOutput() {
    venomConfig(PARAMS_LEN, 0, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i < OUTPUTS_LEN; i++) {
      configOutput(POLY_OUTPUT+i, string::f("Port %d", i + 1));
    }
  }

  void process(const ProcessArgs& args) override {
    BayOutputModule::process(args);
    if (srcMod) {
      for (int i=0; i<OUTPUTS_LEN; i++) {
        int cnt = srcMod->inputs[i].getChannels();
        for (int c=0; c<cnt; c++)
          outputs[i].setVoltage(srcMod->inputs[i].getVoltage(c), c);
        outputs[i].setChannels(cnt);
      }
    }
    else {
      for (int i=0; i<OUTPUTS_LEN; i++) {
        outputs[i].setVoltage(0.f);
        outputs[i].setChannels(0);
      }
    }
  }
};

struct BayOutputWidget : BayOutputModuleWidget {

  BayOutputWidget(BayOutput* module) {
    module->bayOutputType = 0;
    setModule(module);
    setVenomPanel("BayOutput");

    for (int i=0; i<BayOutput::OUTPUTS_LEN; i++) {
      addOutput(createOutputCentered<PolyPort>(Vec(30.f,48.5f+i*42), module, BayOutput::POLY_OUTPUT + i));
    }
  }

};

Model* modelBayOutput = createModel<BayOutput, BayOutputWidget>("BayOutput");

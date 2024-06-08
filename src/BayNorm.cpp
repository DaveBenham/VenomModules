// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "BayModule.hpp"

struct BayNorm : BayOutputModule {

  BayNorm() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i < OUTPUTS_LEN; i++) {
      configInput(POLY_INPUT+i, string::f("Port %d", i + 1));
      configOutput(POLY_OUTPUT+i, string::f("Port %d", i + 1));
      modName = "Bay Norm";
    }
  }

  void process(const ProcessArgs& args) override {
    BayOutputModule::process(args);
    if (srcMod) {
      for (int i=0; i<OUTPUTS_LEN; i++) {
        int cnt = std::max(srcMod->inputs[i].getChannels(), inputs[i].getChannels());
        for (int c=0; c<cnt; c++)
          outputs[i].setVoltage(srcMod->inputs[i].getNormalVoltage(inputs[i].getVoltage(c), c), c);
        outputs[i].setChannels(cnt);
      }
    }
    else {
      for (int i=0; i<OUTPUTS_LEN; i++) {
        int cnt = inputs[i].getChannels();
        for (int c=0; c<cnt; c++)
          outputs[i].setVoltage(inputs[i].getVoltage(c), c);
        outputs[i].setChannels(cnt);
      }
    }
  }
  
};

struct BayNormWidget : BayOutputModuleWidget {

  BayNormWidget(BayNorm* module) {
    setModule(module);
    setVenomPanel("BayNorm");
    if (module) {
      module->bayOutputType = 1;
    }

    BayOutputLabelsWidget* text = createWidget<BayOutputLabelsWidget>(Vec(0.f,0.f));
    text->mod = module;
    text->box.size = box.size;
    text->modName = "BAY NORM";
    addChild(text);

    for (int i=0; i<BayNorm::OUTPUTS_LEN; i++) {
      addInput(createInputCentered<PolyPort>(Vec(20.5f,48.5f+i*42), module, BayNorm::POLY_INPUT + i));
      addOutput(createOutputCentered<PolyPort>(Vec(54.5f,48.5f+i*42), module, BayNorm::POLY_OUTPUT + i));
    }
  }

};

Model* modelBayNorm = createModel<BayNorm, BayNormWidget>("BayNorm");

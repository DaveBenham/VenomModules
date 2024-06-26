// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "BayModule.hpp"

struct BayOutput : BayOutputModule {

  BayOutput() {
    venomConfig(PARAMS_LEN, 0, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i < OUTPUTS_LEN; i++) {
      configOutput(POLY_OUTPUT+i, string::f("Port %d", i + 1));
      modName = "Bay Output";
    }
  }

  void process(const ProcessArgs& args) override {
    BayOutputModule::process(args);
    if (srcMod && !srcMod->isBypassed()) {
      for (int i=0; i<OUTPUTS_LEN; i++) {
        int cnt = srcMod->inputs[i].getChannels();
        outputs[i].channels = std::max(cnt,1);
        for (int c=0; c<cnt; c++)
          outputs[i].setVoltage(srcMod->inputs[i].getVoltage(c), c);
        if (zeroChannel==true && cnt==0)
          outputs[i].channels = 0;
        else
          outputs[i].setChannels(cnt);
      }
    }
    else {
      for (int i=0; i<OUTPUTS_LEN; i++) {
        if (zeroChannel==true)
          outputs[i].channels = 0;
        else {
          outputs[i].channels = 1;
          outputs[i].setVoltage(0.f);
          outputs[i].setChannels(0);
        }
      }
    }
  }
};

struct BayOutputWidget : BayOutputModuleWidget {

  BayOutputWidget(BayOutput* module) {
    setModule(module);
    setVenomPanel("BayOutput");
    if (module) {
      module->bayOutputType = 0;
    }

    BayOutputLabelsWidget* text = createWidget<BayOutputLabelsWidget>(Vec(0.f,0.f));
    text->mod = module;
    text->box.size = box.size;
    text->modName = "BAY OUTPUT";
    addChild(text);

    for (int i=0; i<BayOutput::OUTPUTS_LEN; i++) {
      addOutput(createOutputCentered<PolyPort>(Vec(37.5f,48.5f+i*42), module, BayOutput::POLY_OUTPUT + i));
    }
  }

};

Model* modelBayOutput = createModel<BayOutput, BayOutputWidget>("BayOutput");

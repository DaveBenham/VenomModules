// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "BayModule.hpp"

struct BayNorm : BayOutputModule {
  
  BayNorm() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i < OUTPUTS_LEN; i++) {
      configInput(POLY_INPUT+i, defaultNormalName[i]);
      configOutput(POLY_OUTPUT+i, defaultPortName[i]);
      modName = "Bay Norm";
    }
    clockDivider.setDivision(32);
  }

  void process(const ProcessArgs& args) override {
    BayOutputModule::process(args);
    if (srcMod && !srcMod->isBypassed()) {
      for (int i=0; i<OUTPUTS_LEN; i++) {
        int cnt = 0;
        if (srcMod->inputs[i].isConnected()) {
          cnt = srcMod->inputs[i].getChannels();
          outputs[i].channels = std::max(cnt,1);
          for (int c=0; c<cnt; c++)
            outputs[i].setVoltage(srcMod->inputs[i].getVoltage(c), c);
        }
        else {
          cnt = inputs[i].getChannels();
          outputs[i].channels = std::max(cnt,1);
          for (int c=0; c<cnt; c++)
            outputs[i].setVoltage(inputs[i].getVoltage(c), c);
        }
        if (zeroChannel==true && cnt==0)
          outputs[i].channels = 0;
        else
          outputs[i].setChannels(cnt);
      }
    }
    else {
      for (int i=0; i<OUTPUTS_LEN; i++) {
        int cnt = inputs[i].getChannels();
        outputs[i].channels = std::max(cnt,1);
        for (int c=0; c<cnt; c++)
          outputs[i].setVoltage(inputs[i].getVoltage(c), c);
        if (zeroChannel==true && cnt==0)
          outputs[i].channels = 0;
        else
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

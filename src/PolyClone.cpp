// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "CloneModule.hpp"

struct PolyClone : CloneModuleBase {

  enum ParamId {
    CLONE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    POLY_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    POLY_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(CHANNEL_LIGHTS, 32),
    LIGHTS_LEN
  };

  int clones = 1;

  dsp::ClockDivider lightDivider;

  PolyClone() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(CLONE_PARAM, 1.f, 16.f, 1.f, "Clone count");
    configInput(POLY_INPUT, "Poly");
    configOutput(POLY_OUTPUT, "Poly");
    configBypass(POLY_INPUT, POLY_OUTPUT);
    for (int i=0; i<16; i++){
      configLight(CHANNEL_LIGHTS+i*2, string::f("Channel %d clone indicator", i+1))->description = "yellow = OK, red = Error";
    }  
    lightDivider.setDivision(44);
    lights[CHANNEL_LIGHTS].setBrightness(1);
    lights[CHANNEL_LIGHTS+1].setBrightness(0);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    clones = static_cast<int>(params[CLONE_PARAM].getValue());
    int maxCh = 16/clones;

    // Get number of channels
    int ch = std::max({1,inputs[POLY_INPUT].getChannels()});
    int goodCh = ch > maxCh ? maxCh : ch;

    int c=0;
    for (int i=0; i<goodCh; i++) {
      float val = inputs[POLY_INPUT].getVoltage(i);
      for (int j=0; j<clones; j++)
        outputs[POLY_OUTPUT].setVoltage(val, c++);
    }
    int outCnt = goodCh * clones;
    outputs[POLY_OUTPUT].setChannels(outCnt);
    processExpander(clones, goodCh);

    if (lightDivider.process()) {
      for (int i=1; i<16; i++) {
        lights[CHANNEL_LIGHTS+i*2].setBrightness(i<goodCh);
        lights[CHANNEL_LIGHTS+i*2+1].setBrightness(i>=goodCh && i<ch);
      }
      setExpanderLights(goodCh);
    }
  }
  
};


struct PolyCloneWidget : VenomWidget {

  struct PCCountDisplay : DigitalDisplay18 {
    void step() override {
      if (module) {
        PolyClone* mod = static_cast<PolyClone*>(module);
        text = string::f("%d", mod->clones);
        fgColor = SCHEME_YELLOW;
      } else {
        text = "16";
        fgColor = SCHEME_YELLOW;
      }
    }
  };

  PolyCloneWidget(PolyClone* module) {
    setModule(module);
    setVenomPanel("PolyClone");

    float x = 22.5f;
    float y = RACK_GRID_WIDTH * 5.5f + 3;
    float dy = RACK_GRID_WIDTH * 2.f;

    PCCountDisplay* countDisplay = createWidget<PCCountDisplay>(Vec(x-12, y-23));
    countDisplay->module = module;
    addChild(countDisplay);

    y+=dy*1.f;
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(x,y), module, PolyClone::CLONE_PARAM));

    y += dy*4.75f;
    int ly = y-2;
    for(int li = 0; li < 8; li++, ly-=dy/2.f){
      addChild(createLightCentered<MediumLight<YellowRedLight<>>>(Vec(x-dy/5.f,ly), module, (PolyClone::CHANNEL_LIGHTS+li)*2));
      addChild(createLightCentered<MediumLight<YellowRedLight<>>>(Vec(x+dy/5.f,ly), module, (PolyClone::CHANNEL_LIGHTS+li+8)*2));
    }

    y+=dy;
    addInput(createInputCentered<PolyPort>(Vec(x,y), module, PolyClone::POLY_INPUT));

    y+=dy*1.75f;
    addOutput(createOutputCentered<PolyPort>(Vec(x,y), module, PolyClone::POLY_OUTPUT));
  }

};

Model* modelPolyClone = createModel<PolyClone, PolyCloneWidget>("PolyClone");

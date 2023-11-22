// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

struct CloneMerge : VenomModule {
  enum ParamId {
    CLONE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(MONO_INPUTS, 8),
    INPUTS_LEN
  };
  enum OutputId {
    POLY_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(MONO_LIGHTS, 16),
    LIGHTS_LEN
  };

  dsp::ClockDivider lightDivider;

  CloneMerge() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(CLONE_PARAM, 1.f, 16.f, 1.f, "Clone count");
    for (int i=0; i < 8; i++) {
      configInput(MONO_INPUTS+i, string::f("Mono %d", i + 1));
      configLight(MONO_LIGHTS+i*2, string::f("Input %d cloned indicator (yellow OK, red Error)", i + 1));
    }
    configOutput(POLY_OUTPUT, "Poly");
    lightDivider.setDivision(44);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    int clones = static_cast<int>(params[CLONE_PARAM].getValue());
    int maxIns = 16/clones;

    // Get number of inputs
    int ins = 1;
    for (int i=7; i>0; i--) {
      if (inputs[MONO_INPUTS+i].isConnected()) {
        ins = i+1;
        break;
      }
    }
    int goodIns = ins > maxIns ? maxIns : ins;

    int channel=0;
    for (int i=0; i<goodIns; i+=1) {
      float val = inputs[MONO_INPUTS + i].getVoltage();
      for (int c=0; c<clones; c++)
        outputs[POLY_OUTPUT].setVoltage(val, channel++);
    }
    outputs[POLY_OUTPUT].setChannels(channel);
    
    if (lightDivider.process()) {
      for (int i=0; i<8; i++) {
        lights[MONO_LIGHTS+i*2].setBrightness(i<goodIns);
        lights[MONO_LIGHTS+i*2+1].setBrightness(i>=goodIns && i<ins);
      }
    }
  }

};

struct CloneMergeWidget : VenomWidget {
  CloneMergeWidget(CloneMerge* module) {
    setModule(module);
    setVenomPanel("CloneMerge");

    float x = 22.5f;
    float y = RACK_GRID_WIDTH * 3.55f;
    float dy = RACK_GRID_WIDTH * 2.f;

    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(Vec(x,y), module, CloneMerge::CLONE_PARAM));

    y+=dy*1.25f;
    for (int i=0; i<8; i++, y+=dy) {
      addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, CloneMerge::MONO_INPUTS + i));
      addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(x+dy*0.5f, y-dy*0.3f), module, CloneMerge::MONO_LIGHTS + i*2));
    }
    y+=dy*0.33f;
    addOutput(createOutputCentered<PolyPJ301MPort>(Vec(x,y), module, CloneMerge::POLY_OUTPUT));
  }

};

Model* modelCloneMerge = createModel<CloneMerge, CloneMergeWidget>("CloneMerge");

// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "CloneModule.hpp"

struct CloneMerge : CloneModuleBase {

  enum ParamId {
    CLONE_PARAM,
    GROUP_PARAM,
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
      configLight(MONO_LIGHTS+i*2, string::f("Input %d cloned indicator", i + 1))->description = "yellow = OK, red = Error";
    }
    configSwitch<FixedSwitchQuantity>(GROUP_PARAM, 0.f, 1.f, 0.f, "Output grouping", {"Input channel", "Input set"});
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

    if (params[GROUP_PARAM].getValue()){ //group by input set
      for (int i=0; i<goodIns; i++) {
        float val = inputs[MONO_INPUTS+i].getVoltage();
        for (int c=0, o=i; c<clones; c++, o+=goodIns)
          outputs[POLY_OUTPUT].setVoltage(val, o);
      }
    }
    else{ // group by input channel
      for (int i=0, o=0; i<goodIns; i+=1) {
        float val = inputs[MONO_INPUTS + i].getVoltage();
        for (int c=0; c<clones; c++)
          outputs[POLY_OUTPUT].setVoltage(val, o++);
      }
    }
    outputs[POLY_OUTPUT].setChannels(goodIns * clones);
    
    processExpander(clones, goodIns);
    
    if (lightDivider.process()) {
      for (int i=0; i<8; i++) {
        lights[MONO_LIGHTS+i*2].setBrightness(i<goodIns);
        lights[MONO_LIGHTS+i*2+1].setBrightness(i>=goodIns && i<ins);
      }
      setExpanderLights(goodIns);
    }
  }

};

struct CloneMergeWidget : CloneModuleWidget {
  CloneMergeWidget(CloneMerge* module) {
    setModule(module);
    setVenomPanel("CloneMerge");

    float x = 22.5f;
    float y = RACK_GRID_WIDTH * 3.55f;
    float dy = RACK_GRID_WIDTH * 2.f;

    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(Vec(x,y), module, CloneMerge::CLONE_PARAM));

    y+=dy*1.25f;
    for (int i=0; i<8; i++, y+=dy) {
      addInput(createInputCentered<MonoPort>(Vec(x,y), module, CloneMerge::MONO_INPUTS + i));
      addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(x+dy*0.5f, y-dy*0.3f), module, CloneMerge::MONO_LIGHTS + i*2));
    }

    addParam(createLockableParamCentered<GroupSwitch>(Vec(6.f,311.5f), module, CloneMerge::GROUP_PARAM));

    y+=dy*0.33f;
    addOutput(createOutputCentered<PolyPort>(Vec(x,y), module, CloneMerge::POLY_OUTPUT));
  }

};

Model* modelVenomCloneMerge = createModel<CloneMerge, CloneMergeWidget>("CloneMerge");

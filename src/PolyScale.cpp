// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

struct PolyScale : VenomModule {
  enum ParamId {
    ENUMS(LEVEL_PARAM,16),
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
    LIGHTS_LEN
  };
  
  int channels = 0;
  
  PolyScale() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<16; i++) {
      std::string nm = "Level " + std::to_string(i+1);
      configParam(LEVEL_PARAM+i, 0.f, 1.f, 0.f, nm, " V");
    }
    configInput(POLY_INPUT,"Poly");
    configOutput(POLY_OUTPUT,"Poly");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
  
};

struct PolyScaleWidget : VenomWidget {
  
  struct PCCountDisplay : DigitalDisplay18 {
    void step() override {
      if (module) {
        PolyScale* mod = dynamic_cast<PolyScale*>(module);
        text = string::f("%d", mod->channels);
        fgColor = SCHEME_YELLOW;
      } else {
        text = "16";
        fgColor = SCHEME_YELLOW;
      }
    }
  };

  PolyScaleWidget(PolyScale* module) {
    setModule(module);
    setVenomPanel("PolyScale");
    float y=64.5f;
    for (int i=0; i<8; i++, y+=24.f){
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.f, y), module, PolyScale::LEVEL_PARAM+i));
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(33.f, y), module, PolyScale::LEVEL_PARAM+i+8));
    }

    PCCountDisplay* countDisplay = createWidget<PCCountDisplay>(Vec(10.316, 252.431));
    countDisplay->module = module;
    addChild(countDisplay);

    addInput(createInputCentered<PolyPort>(Vec(22.5,300.5), module, PolyScale::POLY_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5,339.5), module, PolyScale::POLY_OUTPUT));
  }

};

Model* modelPolyScale = createModel<PolyScale, PolyScaleWidget>("PolyScale");

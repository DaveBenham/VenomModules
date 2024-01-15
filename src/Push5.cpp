// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

struct Push5 : VenomModule {
  enum ParamId {
    ENUMS(BUTTON_PARAM,5),
    PARAMS_LEN
  };
  enum InputId {
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(OUTPUT,5),
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(BUTTON_LIGHT,5*3),
    LIGHTS_LEN
  };
  
  Push5() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<5; i++) {
      std::string nm = "Button " + std::to_string(i+1);
      configSwitch<FixedSwitchQuantity>(BUTTON_PARAM+i, 0.f, 1.f, 0.f, nm, {"Off", "On"});
      configOutput(OUTPUT+i, nm);
    }  
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
  
};

struct Push5Widget : VenomWidget {

  Push5Widget(Push5* module) {
    setModule(module);
    setVenomPanel("Push5");
    float y=42.5f;
    for (int i=0; i<5; i++, y+=31.f){
      addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedGreenBlueLight>>>(Vec(22.5f, y), module, Push5::BUTTON_PARAM+i, Push5::BUTTON_LIGHT+i*3));
    }
    y = 209.5f;
    for (int i=0; i<5; i++, y+=32.f){
      addOutput(createOutputCentered<PolyPort>(Vec(22.5f, y), module, Push5::OUTPUT+i));
    }
  }
};

Model* modelPush5 = createModel<Push5, Push5Widget>("Push5");

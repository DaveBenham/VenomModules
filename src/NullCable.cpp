// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

#define LIGHT_OFF 0.02f

namespace Venom {

struct NullCable : VenomModule {

  enum ParamId {
    ENUMS(GATE_PARAM,3),
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(GATE_INPUT,3),
    ENUMS(SIGNAL_INPUT,3),
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(SIGNAL_OUTPUT,3),
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(GATE_LIGHT,6),
    LIGHTS_LEN
  };
  
  NullCable() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    
    for (int i=0; i<3; i++) {
      std::string istr = std::to_string(i+1);
      std::string nm = "Gate " + istr;
      configParam(GATE_PARAM+i, 0.f, 1.f, 0.f, nm);
      configInput(GATE_INPUT+i, nm);
      nm = "Signal " + istr;
      configInput(SIGNAL_INPUT+i, nm);
      configOutput(SIGNAL_OUTPUT+i, nm);
      configBypass(SIGNAL_INPUT+i, SIGNAL_OUTPUT+i);
    }
  }
  

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
  
};

struct NullCableWidget : VenomWidget {

  NullCableWidget(NullCable* module) {
    setModule(module);
    setVenomPanel("NullCable");

    for (int i=0; i<3; i++) {
      float delta = i * 111.f;
      addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteRedLight<>>>>(Vec(22.5f, 38.f+delta), module, NullCable::GATE_PARAM+i, NullCable::GATE_LIGHT+i*2));
      addInput(createInputCentered<MonoPort>(Vec(22.5f, 65.f+delta), module, NullCable::GATE_INPUT+i));
      addInput(createInputCentered<PolyPort>(Vec(22.5f, 93.f+delta), module, NullCable::SIGNAL_INPUT+i));
      addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 121.5f+delta), module, NullCable::SIGNAL_OUTPUT+i));
    }
  }
  
};

}

Model* modelVenomNullCable = createModel<Venom::NullCable, Venom::NullCableWidget>("NullCable");

// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct Blocker : VenomModule {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    INPUTS_LEN
  };
  enum OutputId {
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  Blocker() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
  
  void onExpanderChange(const ExpanderChangeEvent& e) override {
    if (e.side) {
      getRightExpander().module->getLeftExpander().module = NULL;
      getRightExpander().module->getLeftExpander().moduleId = -1;
      getRightExpander().module = NULL;
      getRightExpander().moduleId = -1;
    }
    else {
      getLeftExpander().module->getRightExpander().module = NULL;
      getLeftExpander().module->getRightExpander().moduleId = -1;
      getLeftExpander().module = NULL;
      getLeftExpander().moduleId = -1;
    }
  }

};

struct BlockerWidget : VenomWidget {
  
  BlockerWidget(Blocker* module) {
    setModule(module);
    setVenomPanel("Blocker");
  }

};

}

Model* modelVenomBlocker = createModel<Venom::Blocker, Venom::BlockerWidget>("Blocker");

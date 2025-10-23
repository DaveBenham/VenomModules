// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct VenomBlank : VenomModule {
  VenomBlank() {
    venomConfig(0, 0, 0, 0);
  }
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
};

struct VenomBlankWidget : VenomWidget {
  VenomBlankWidget(VenomBlank* module) {
    setModule(module);
    setVenomPanel("VenomBlank");
  }
};

}

Model* modelVenomBlank = createModel<Venom::VenomBlank, Venom::VenomBlankWidget>("VenomBlank");

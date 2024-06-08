// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "BayModule.hpp"

// BayInput module defined in BayModule.hpp

struct BayInputWidget : VenomWidget {

  BayInputWidget(BayInput* module) {
    setModule(module);
    setVenomPanel("BayInput");

    for (int i=0; i<8; i++) {
      addInput(createInputCentered<PolyPort>(Vec(30.f,48.5f+i*42), module, BayInput::POLY_INPUT + i));
    }
  }

};

std::map<int64_t, BayInput*> BayModule::sources{};

Model* modelBayInput = createModel<BayInput, BayInputWidget>("BayInput");

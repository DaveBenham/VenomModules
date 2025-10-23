// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "MixModule.hpp"

namespace Venom {

struct MixPan : MixExpanderModule {

  MixPan() {
    venomConfig(PAN_PARAMS_LEN, PAN_INPUTS_LEN, PAN_OUTPUTS_LEN, EXP_LIGHTS_LEN);
    mixType = MIXPAN_TYPE;
    configLight(EXP_LIGHT, "Left connection indicator");
    for (int i=0; i<4; i++) {
      std::string i_s = std::to_string(i+1);
      configParam(PAN_PARAM+i, -1.f, 1.f, 0.f, "Pan "+i_s);
      configParam(PAN_CV_PARAM+i, -1.f, 1.f, 0.f, "Pan CV "+i_s);
      configInput(PAN_INPUT+i, "Pan CV " + i_s);
    }  
  }

  void process(const ProcessArgs& args) override {
    MixExpanderModule::process(args);
  }
  
};

struct MixPanWidget : MixExpanderWidget {
  MixPanWidget(MixPan* module) {
    setModule(module);
    setVenomPanel("MixPan");

    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(5.f, 22.f), module, MixModule::EXP_LIGHT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f,  42.295f), module, MixModule::PAN_PARAM+0));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f,  73.035f), module, MixModule::PAN_PARAM+1));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 103.775f), module, MixModule::PAN_PARAM+2));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 134.514f), module, MixModule::PAN_PARAM+3));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(13.5f, 173.278f), module, MixModule::PAN_CV_PARAM+0));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(13.5f, 222.179f), module, MixModule::PAN_CV_PARAM+1));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(13.5f, 271.081f), module, MixModule::PAN_CV_PARAM+2));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(13.5f, 319.983f), module, MixModule::PAN_CV_PARAM+3));
    addInput(createInputCentered<PolyPort>(Vec(28.5f, 195.82f), module, MixModule::PAN_INPUT+0));
    addInput(createInputCentered<PolyPort>(Vec(28.5f, 244.721f), module, MixModule::PAN_INPUT+1));
    addInput(createInputCentered<PolyPort>(Vec(28.5f, 293.623f), module, MixModule::PAN_INPUT+2));
    addInput(createInputCentered<PolyPort>(Vec(28.5f, 342.525f), module, MixModule::PAN_INPUT+3));
  }
};

}

Model* modelVenomMixPan = createModel<Venom::MixPan, Venom::MixPanWidget>("MixPan");

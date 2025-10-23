// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "MixModule.hpp"

namespace Venom {

struct MixOffset : MixExpanderModule {

  MixOffset() {
    venomConfig(OFFSET_PARAMS_LEN, OFFSET_INPUTS_LEN, OFFSET_OUTPUTS_LEN, EXP_LIGHTS_LEN);
    mixType = MIXOFFSET_TYPE;
    configLight(EXP_LIGHT, "Left connection indicator");
    for (int i=0; i<4; i++) {
      std::string i_s = std::to_string(i+1);
      configParam(PRE_OFFSET_PARAM+i, -10.f, 10.f, 0.f, "Pre level "+i_s+" offset", " V");
      configParam(POST_OFFSET_PARAM+i, -10.f, 10.f, 0.f, "Post level "+i_s+" offset", " V");
    }  
    configParam(PRE_MIX_OFFSET_PARAM, -10.f, 10.f, 0.f, "Pre mix level offset", " V");
    configParam(POST_MIX_OFFSET_PARAM, -10.f, 10.f, 0.f, "Post mix level offset", " V");
  }

  void process(const ProcessArgs& args) override {
    MixExpanderModule::process(args);
  }
  
};

struct MixOffsetWidget : MixExpanderWidget {
  MixOffsetWidget(MixOffset* module) {
    setModule(module);
    setVenomPanel("MixOffset");

    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(5.f, 22.f), module, MixModule::EXP_LIGHT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f,  42.295f), module, MixModule::PRE_OFFSET_PARAM+0));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f,  73.035f), module, MixModule::PRE_OFFSET_PARAM+1));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 103.775f), module, MixModule::PRE_OFFSET_PARAM+2));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 134.514f), module, MixModule::PRE_OFFSET_PARAM+3));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5f, 168.254f), module, MixModule::PRE_MIX_OFFSET_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 209.778f), module, MixModule::POST_OFFSET_PARAM+0));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 241.320f), module, MixModule::POST_OFFSET_PARAM+1));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 273.239f), module, MixModule::POST_OFFSET_PARAM+2));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 305.158f), module, MixModule::POST_OFFSET_PARAM+3));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5f, 340.434f), module, MixModule::POST_MIX_OFFSET_PARAM));
  }
};

}

Model* modelVenomMixOffset = createModel<Venom::MixOffset, Venom::MixOffsetWidget>("MixOffset");

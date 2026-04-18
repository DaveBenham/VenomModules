// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "MixModule.hpp"

namespace Venom {

struct MixFade : MixExpanderModule {
  
  MixFade() {
    venomConfig(FADE_PARAMS_LEN, FADE_INPUTS_LEN, FADE_OUTPUTS_LEN, EXP_LIGHTS_LEN);
    mixType = MIXFADE_TYPE;
    configLight(EXP_LIGHT, "Left connection indicator");
    for (int i=0; i<4; i++) {
      std::string i_s = std::to_string(i+1);
      configParam(FADE_TIME_PARAM+i, 0.f, 30.f, 0.f, "Fade " + i_s + " time");
      configParam<ShapeQuantity>(FADE_SHAPE_PARAM+i, -1.f, 1.f, 0.f, "Fade " + i_s + " shape", "%", 0.f, 100.f);
      configOutput(FADE_OUTPUT+i, "Fade " + i_s + " level");
    }  
    configParam(FADE_MIX_TIME_PARAM, 0.f, 30.f, 0.f, "Fade mix time");
    configParam<ShapeQuantity>(FADE_MIX_SHAPE_PARAM, -1.f, 1.f, 0.f, "Fade mix shape");
    configOutput(FADE_MIX_OUTPUT, "Fade mix level");
  }

  void process(const ProcessArgs& args) override {
    MixExpanderModule::process(args);
  }
  
};

struct MixFadeWidget : MixExpanderWidget {

  MixFadeWidget(MixFade* module) {
    setModule(module);
    setVenomPanel("MixFade");

    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(5.f, 22.f), module, MixModule::EXP_LIGHT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.23f,  42.295f), module, MixModule::FADE_TIME_PARAM+0));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.23f,  73.035f), module, MixModule::FADE_TIME_PARAM+1));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.23f, 103.775f), module, MixModule::FADE_TIME_PARAM+2));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.23f, 134.514f), module, MixModule::FADE_TIME_PARAM+3));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.23f, 168.254f), module, MixModule::FADE_MIX_TIME_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(32.77f,  42.295f), module, MixModule::FADE_SHAPE_PARAM+0));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(32.77f,  73.035f), module, MixModule::FADE_SHAPE_PARAM+1));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(32.77f, 103.775f), module, MixModule::FADE_SHAPE_PARAM+2));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(32.77f, 134.514f), module, MixModule::FADE_SHAPE_PARAM+3));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(32.77f, 168.254f), module, MixModule::FADE_MIX_SHAPE_PARAM));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f, 209.778f), module, MixModule::FADE_OUTPUT+0));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f, 241.320f), module, MixModule::FADE_OUTPUT+1));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f, 273.239f), module, MixModule::FADE_OUTPUT+2));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f, 305.158f), module, MixModule::FADE_OUTPUT+3));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f, 340.434f), module, MixModule::FADE_MIX_OUTPUT));
  }

};

}

Model* modelVenomMixFade = createModel<Venom::MixFade, Venom::MixFadeWidget>("MixFade");

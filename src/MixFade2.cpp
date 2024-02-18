// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "MixModule.hpp"

struct MixFade2 : MixExpanderModule {
  
  MixFade2() {
    venomConfig(FADE2_PARAMS_LEN, FADE2_INPUTS_LEN, FADE2_OUTPUTS_LEN, EXP_LIGHTS_LEN);
    mixType = MIXFADE2_TYPE;
    configLight(EXP_LIGHT, "Left connection indicator");
    for (int i=0; i<4; i++) {
      std::string i_s = std::to_string(i+1);
      configParam(RISE_TIME_PARAM+i, 0.f, 30.f, 0.f, "Fade " + i_s + " rise time", " sec");
      configParam(FALL_TIME_PARAM+i, 0.f, 30.f, 0.f, "Fade " + i_s + " fall time", " sec");
      configParam<ShapeQuantity>(FADE2_SHAPE_PARAM+i, -1.f, 1.f, 0.f, "Fade " + i_s + " shape", "%", 0.f, 100.f);
      configOutput(FADE2_OUTPUT+i, "Fade " + i_s + " level");
    }  
    configParam(MIX_RISE_TIME_PARAM, 0.f, 30.f, 0.f, "Fade mix rise time", " sec");
    configParam(MIX_FALL_TIME_PARAM, 0.f, 30.f, 0.f, "Fade mix fall time", " sec");
    configParam<ShapeQuantity>(FADE2_MIX_SHAPE_PARAM, -1.f, 1.f, 0.f, "Fade mix shape", "%", 0.f, 100.f);
    configOutput(FADE2_MIX_OUTPUT, "Fade mix level");
  }

  void process(const ProcessArgs& args) override {
    MixExpanderModule::process(args);
  }
  
};

struct MixFade2Widget : MixExpanderWidget {

  MixFade2Widget(MixFade2* module) {
    setModule(module);
    setVenomPanel("MixFade2");

    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(5.f, 22.f), module, MixModule::EXP_LIGHT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(10.73f,  47.045f), module, MixModule::RISE_TIME_PARAM+0));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(10.73f,  77.785f), module, MixModule::RISE_TIME_PARAM+1));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(10.73f, 108.525f), module, MixModule::RISE_TIME_PARAM+2));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(10.73f, 139.264f), module, MixModule::RISE_TIME_PARAM+3));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(10.73f, 173.004f), module, MixModule::MIX_RISE_TIME_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(49.27,   47.045f), module, MixModule::FALL_TIME_PARAM+0));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(49.27,   77.785f), module, MixModule::FALL_TIME_PARAM+1));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(49.27,  108.525f), module, MixModule::FALL_TIME_PARAM+2));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(49.27,  139.264f), module, MixModule::FALL_TIME_PARAM+3));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(49.27,  173.004f), module, MixModule::MIX_FALL_TIME_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(30.f,    37.545f), module, MixModule::FADE2_SHAPE_PARAM+0));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(30.f,    68.285f), module, MixModule::FADE2_SHAPE_PARAM+1));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(30.f,    99.025f), module, MixModule::FADE2_SHAPE_PARAM+2));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(30.f,   129.764f), module, MixModule::FADE2_SHAPE_PARAM+3));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(30.f,   163.504f), module, MixModule::FADE2_MIX_SHAPE_PARAM));
    addOutput(createOutputCentered<MonoPort>(Vec(30.f, 209.778f), module, MixModule::FADE2_OUTPUT+0));
    addOutput(createOutputCentered<MonoPort>(Vec(30.f, 241.320f), module, MixModule::FADE2_OUTPUT+1));
    addOutput(createOutputCentered<MonoPort>(Vec(30.f, 273.239f), module, MixModule::FADE2_OUTPUT+2));
    addOutput(createOutputCentered<MonoPort>(Vec(30.f, 305.158f), module, MixModule::FADE2_OUTPUT+3));
    addOutput(createOutputCentered<MonoPort>(Vec(30.f, 340.434f), module, MixModule::FADE2_MIX_OUTPUT));
  }

};

Model* modelMixFade2 = createModel<MixFade2, MixFade2Widget>("MixFade2");

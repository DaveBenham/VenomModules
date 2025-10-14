// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "MixModule.hpp"

#define LIGHT_OFF 0.02f
#define LIGHT_ON  1.f

struct MixMute : MixExpanderModule {
  
  MixMute() {
    venomConfig(MUTE_PARAMS_LEN, MUTE_INPUTS_LEN, MUTE_OUTPUTS_LEN, MUTE_LIGHTS_LEN);
    mixType = MIXMUTE_TYPE;
    configLight(EXP_LIGHT, "Left connection indicator");
    for (int i=0; i<4; i++) {
      std::string i_s = std::to_string(i+1);
      configSwitch<FixedSwitchQuantity>(MUTE_PARAM+i, 0.f, 1.f, 0.f, "Mute " + i_s, {"Unmuted", "Muted"});
      configInput(MUTE_INPUT+i, "Mute " + i_s);
    }  
    configSwitch<FixedSwitchQuantity>(MUTE_MIX_PARAM, 0.f, 1.f, 0.f, "Mute Mix", {"Unmuted", "Muted"});
    configInput(MUTE_MIX_INPUT, "Mute Mix");
  }

  void process(const ProcessArgs& args) override {
    MixExpanderModule::process(args);
  }
  
};

struct MixMuteWidget : MixExpanderWidget {

  MixMuteWidget(MixMute* module) {
    setModule(module);
    setVenomPanel("MixMute");

    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(5.f, 22.f), module, MixModule::EXP_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(22.5f,  42.295f), module, MixModule::MUTE_PARAM+0, MixModule::MUTE_LIGHT+0));
    addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(22.5f,  73.035f), module, MixModule::MUTE_PARAM+1, MixModule::MUTE_LIGHT+1));
    addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(22.5f, 103.775f), module, MixModule::MUTE_PARAM+2, MixModule::MUTE_LIGHT+2));
    addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(22.5f, 134.514f), module, MixModule::MUTE_PARAM+3, MixModule::MUTE_LIGHT+3));
    addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(22.5f, 168.254f), module, MixModule::MUTE_MIX_PARAM, MixModule::MUTE_MIX_LIGHT));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 209.778f), module, MixModule::MUTE_INPUT+0));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 241.320f), module, MixModule::MUTE_INPUT+1));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 273.239f), module, MixModule::MUTE_INPUT+2));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 305.158f), module, MixModule::MUTE_INPUT+3));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 340.434f), module, MixModule::MUTE_MIX_INPUT));
  }

  void step() override {
    MixExpanderWidget::step();
    if(this->module) {
      for (int i=0; i<4; i++){
        this->module->lights[MixModule::MUTE_LIGHT+i].setBrightness(this->module->params[MixModule::MUTE_PARAM+i].getValue() ? LIGHT_ON : LIGHT_OFF);
      }
      this->module->lights[MixModule::MUTE_MIX_LIGHT].setBrightness(this->module->params[MixModule::MUTE_MIX_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }  
  }

};

Model* modelVenomMixMute = createModel<MixMute, MixMuteWidget>("MixMute");

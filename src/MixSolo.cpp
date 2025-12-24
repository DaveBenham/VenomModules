// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "MixModule.hpp"

#define LIGHT_OFF 0.02f
#define LIGHT_ON  1.f

namespace Venom {

struct MixSolo : MixExpanderModule {

  MixSolo() {
    venomConfig(MUTE_PARAMS_LEN, MUTE_INPUTS_LEN, MUTE_OUTPUTS_LEN, MUTE_LIGHTS_LEN);
    mixType = MIXSOLO_TYPE;
    configLight(EXP_LIGHT, "Left connection indicator");
    for (int i=0; i<4; i++) {
      std::string i_s = std::to_string(i+1);
      configSwitch<FixedSwitchQuantity>(SOLO_PARAM+i, 0.f, 1.f, 0.f, "Solo " + i_s, {"Off", "On"});
      configInput(SOLO_INPUT+i, "Solo " + i_s);
    }  
  }

  void process(const ProcessArgs& args) override {
    MixExpanderModule::process(args);
  }
  
};

struct MixSoloWidget : MixExpanderWidget {

  MixSoloWidget(MixSolo* module) {
    setModule(module);
    setVenomPanel("MixSolo");

    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(5.f, 22.f), module, MixModule::EXP_LIGHT));
    addParam(createLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<GreenLight>>>(Vec(22.5f,  42.295f), module, MixModule::SOLO_PARAM+0, MixModule::SOLO_LIGHT+0));
    addParam(createLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<GreenLight>>>(Vec(22.5f,  73.035f), module, MixModule::SOLO_PARAM+1, MixModule::SOLO_LIGHT+1));
    addParam(createLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<GreenLight>>>(Vec(22.5f, 103.775f), module, MixModule::SOLO_PARAM+2, MixModule::SOLO_LIGHT+2));
    addParam(createLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<GreenLight>>>(Vec(22.5f, 134.514f), module, MixModule::SOLO_PARAM+3, MixModule::SOLO_LIGHT+3));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 209.778f), module, MixModule::SOLO_INPUT+0));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 241.320f), module, MixModule::SOLO_INPUT+1));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 273.239f), module, MixModule::SOLO_INPUT+2));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 305.158f), module, MixModule::SOLO_INPUT+3));
  }

  void step() override {
    MixExpanderWidget::step();
    if(this->module) {
      for (int i=0; i<4; i++){
        this->module->lights[MixModule::SOLO_LIGHT+i].setBrightness(this->module->params[MixModule::SOLO_PARAM+i].getValue() ? LIGHT_ON : LIGHT_OFF);
      }  
    }  
  }

};

}

Model* modelVenomMixSolo = createModel<Venom::MixSolo, Venom::MixSoloWidget>("MixSolo");

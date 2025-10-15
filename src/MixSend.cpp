// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "MixModule.hpp"

#define LIGHT_OFF 0.02f
#define LIGHT_ON  1.f

namespace Venom {

struct MixSend : MixExpanderModule {

  MixSend() {
    venomConfig(SEND_PARAMS_LEN, SEND_INPUTS_LEN, SEND_OUTPUTS_LEN, SEND_LIGHTS_LEN);
    mixType = MIXSEND_TYPE;
    configLight(EXP_LIGHT, "Left connection indicator");
    for (int i=0; i<4; i++) {
      std::string i_s = std::to_string(i+1);
      configParam(SEND_PARAM+i, 0.f, 1.f, 0.f, "Send level "+i_s);
    }
    configParam(RETURN_PARAM, 0.f, 1.f, 1.f, "Return level");
    configSwitch<FixedSwitchQuantity>(SEND_MUTE_PARAM, 0.f, 1.f, 0.f, "Send Mute", {"Unmuted", "Muted"});
    configOutput(LEFT_SEND_OUTPUT, "Left send");
    configOutput(RIGHT_SEND_OUTPUT, "Right send");
    configInput(LEFT_RETURN_INPUT, "Left return");
    configInput(RIGHT_RETURN_INPUT, "Right return");
    configSwitch<FixedSwitchQuantity>(SEND_CHAIN_PARAM, 0.f, 1.f, 0.f, "Chain mode", {"Off", "On"});
    fade[0].rise = fade[0].fall = 40.f;
}

  void process(const ProcessArgs& args) override {
    MixExpanderModule::process(args);
  }
  
};

struct MixSendWidget : MixExpanderWidget {

  struct ChainSwitch : GlowingSvgSwitchLockable {
    ChainSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
    }
  };

  MixSendWidget(MixSend* module) {
    setModule(module);
    setVenomPanel("MixSend");

    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(5.f, 22.f), module, MixModule::EXP_LIGHT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f,  42.295f), module, MixModule::SEND_PARAM+0));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f,  73.035f), module, MixModule::SEND_PARAM+1));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 103.775f), module, MixModule::SEND_PARAM+2));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 134.514f), module, MixModule::SEND_PARAM+3));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 169.926f), module, MixModule::RETURN_PARAM));
    addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(22.5f, 203.278f), module, MixModule::SEND_MUTE_PARAM, MixModule::RETURN_MUTE_LIGHT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 241.5f), module, MixModule::LEFT_SEND_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 269.5f), module, MixModule::RIGHT_SEND_OUTPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 315.5f), module, MixModule::LEFT_RETURN_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 343.5f), module, MixModule::RIGHT_RETURN_INPUT));
    addParam(createLockableParamCentered<ChainSwitch>(Vec(37.5,300.5), module, MixModule::SEND_CHAIN_PARAM));
  }

  void step() override {
    MixExpanderWidget::step();
    if(this->module) {
      this->module->lights[MixModule::RETURN_MUTE_LIGHT].setBrightness(this->module->params[MixModule::SEND_MUTE_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }  
  }

};

}

Model* modelVenomMixSend = createModel<Venom::MixSend, Venom::MixSendWidget>("MixSend");

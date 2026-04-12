// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

#define LIGHT_OFF 0.02f

namespace Venom {

struct PolyMute : VenomModule {

  enum ParamId {
    ENUMS(MUTE_PARAM,16),
    SOFT_PARAM,
    MODE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    GATES_INPUT,
    POLY1_INPUT,
    POLY2_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    POLY1_OUTPUT,
    POLY2_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(MUTE_LIGHT,32),
    LIGHTS_LEN
  };
  
  PolyMute() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    
    for (int i=0; i<16; i++) {
      std::string nm = "Channel " + std::to_string(i+1);
      configParam(MUTE_PARAM+i, 0.f, 1.f, 0.f, nm);
    }
    configSwitch<FixedSwitchQuantity>(SOFT_PARAM, 0.f, 1.f, 0.f, "Soft switching", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 1.f, 0.f, "Gate mode", {"Mute", "Pass"});
    configInput(GATES_INPUT, "Channel gates");
    configInput(POLY1_INPUT, "Poly 1");
    configInput(POLY2_INPUT, "Poly 2");
    configOutput(POLY1_OUTPUT, "Poly 1");
    configOutput(POLY2_OUTPUT, "Poly 2");
    configBypass(POLY1_INPUT, POLY1_OUTPUT);
    configBypass(POLY2_INPUT, POLY2_OUTPUT);
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
  
};

struct PolyMuteWidget : VenomWidget {

  struct SoftSwitch : GlowingSvgSwitchLockable {
    SoftSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
    }
  };

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };

  PolyMuteWidget(PolyMute* module) {
    setModule(module);
    setVenomPanel("PolyMute");

    for (int i=0; i<8; i++) {
      addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteRedLight<>>>>(Vec(20.5f, 44.5f+i*27.f), module, PolyMute::MUTE_PARAM+i, PolyMute::MUTE_LIGHT+i*2));
      addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteRedLight<>>>>(Vec(54.5f, 44.5f+i*27.f), module, PolyMute::MUTE_PARAM+i+8, PolyMute::MUTE_LIGHT+(i+8)*2));
    }
    addParam(createLockableParamCentered<SoftSwitch>(Vec(12.5f, 269.5f), module, PolyMute::SOFT_PARAM));
    addParam(createLockableParamCentered<ModeSwitch>(Vec(62.5f, 269.5f), module, PolyMute::MODE_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(37.5f, 269.5f), module, PolyMute::GATES_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(20.5f, 303.5f), module, PolyMute::POLY1_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(54.5f, 303.5f), module, PolyMute::POLY2_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(20.5f, 341.f), module, PolyMute::POLY1_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(54.5f, 341.f), module, PolyMute::POLY2_OUTPUT));
  }
  
};

}

Model* modelVenomPolyMute = createModel<Venom::PolyMute, Venom::PolyMuteWidget>("PolyMute");

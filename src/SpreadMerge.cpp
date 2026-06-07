// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct SpreadMerge : VenomModule {

  enum ParamId {
    MODE_PARAM,
    COUNT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(CV_INPUT,8),
    ENUMS(GATE_INPUT,8),
    INPUTS_LEN
  };
  enum OutputId {
    CV_OUTPUT,
    GATE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ERR_LIGHT,
    LIGHTS_LEN
  };
  
  SpreadMerge() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(COUNT_PARAM, 0.f, 6.f, 0.f, "Spread channel count", {"2", "3", "4", "5", "6", "7", "8"});
    configLight(ERR_LIGHT, "Error indicator");
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 1.f, 0.f, "CV mode", {"Sample & Hold", "Track & Hold"});
    for (int i=0; i<8; i++) {
      std::string istr = std::to_string(i+1);
      std::string nm = "Gate " + istr;
      configInput(GATE_INPUT+i, nm);
      nm = "CV " + istr;
      configInput(CV_INPUT+i, nm);
    }
    configOutput(GATE_OUTPUT, "Gate");
    configOutput(CV_OUTPUT, "CV");
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }

};

struct SpreadMergeWidget : VenomWidget {

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_SH.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_TH.svg")));
    }
  };

  struct CountSwitch : GlowingSvgSwitchLockable {
    CountSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_2.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_3.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_4.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_5.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_6.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_7.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_8.svg")));
    }
  };

  SpreadMergeWidget(SpreadMerge* module) {
    setModule(module);
    setVenomPanel("SpreadMerge");

    addParam(createLockableParamCentered<CountSwitch>(Vec(20.f, 46.5f), module, SpreadMerge::COUNT_PARAM));
    addChild(createLightCentered<SmallLight<RedLight>>(Vec(20.f,65.f), module, SpreadMerge::ERR_LIGHT));
    addParam(createLockableParamCentered<ModeSwitch>(Vec(55.f, 76.5f), module, SpreadMerge::MODE_PARAM));
    for (int i=0; i<8; i++){
      addInput(createInputCentered<MonoPort>(Vec(20.f, 118.5f + 27.f*i), module, SpreadMerge::GATE_INPUT+i));
      addInput(createInputCentered<MonoPort>(Vec(55.f, 118.5f + 27.f*i), module, SpreadMerge::CV_INPUT+i));
    }
    addOutput(createOutputCentered<PolyPort>(Vec(20.f, 338.5f), module, SpreadMerge::GATE_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(55.f, 338.5f), module, SpreadMerge::CV_OUTPUT));
  }

/*
  void step() override {
    VenomWidget::step();
    if(this->module) {
      SpreadMerge* mod = static_cast<SpreadMerge*>(this->module);
    }
  }
*/
};

}

Model* modelVenomSpreadMerge = createModel<Venom::SpreadMerge, Venom::SpreadMergeWidget>("SpreadMerge");

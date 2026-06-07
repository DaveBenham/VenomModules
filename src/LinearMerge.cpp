// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct LinearMerge : VenomModule {

  enum ParamId {
    MODE_PARAM,
    SELECT_PARAM,
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
    LIGHTS_LEN
  };
  
  LinearMerge() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 1.f, 0.f, "CV mode", {"Sample & Hold", "Track & Hold"});
    configSwitch<FixedSwitchQuantity>(SELECT_PARAM, 0.f, 5.f, 0.f, "CV selection", {"First", "Last", "Minimum", "Maximum", "Average", "Sum"});
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

struct LinearMergeWidget : VenomWidget {

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_SH.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_TH.svg")));
    }
  };

  struct SelectSwitch : GlowingSvgSwitchLockable {
    SelectSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/select_First.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/select_Last.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/select_Min.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/select_Max.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/select_Avg.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/select_Sum.svg")));
    }
  };

  LinearMergeWidget(LinearMerge* module) {
    setModule(module);
    setVenomPanel("LinearMerge");

    addParam(createLockableParamCentered<ModeSwitch>(Vec(55.f, 46.5f), module, LinearMerge::MODE_PARAM));
    addParam(createLockableParamCentered<SelectSwitch>(Vec(55.f, 76.5f), module, LinearMerge::SELECT_PARAM));
    for (int i=0; i<8; i++){
      addInput(createInputCentered<MonoPort>(Vec(20.f, 118.5f + 27.f*i), module, LinearMerge::GATE_INPUT+i));
      addInput(createInputCentered<MonoPort>(Vec(55.f, 118.5f + 27.f*i), module, LinearMerge::CV_INPUT+i));
    }
    addOutput(createOutputCentered<MonoPort>(Vec(20.f, 338.5f), module, LinearMerge::GATE_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(55.f, 338.5f), module, LinearMerge::CV_OUTPUT));
  }

/*
  void step() override {
    VenomWidget::step();
    if(this->module) {
      LinearMerge* mod = static_cast<LinearMerge*>(this->module);
    }
  }
*/
};

}

Model* modelVenomLinearMerge = createModel<Venom::LinearMerge, Venom::LinearMergeWidget>("LinearMerge");

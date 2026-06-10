// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct SpreadMergeExpander : VenomModule {

  enum ParamId {
    MODE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(CV_INPUT,8),
    INPUTS_LEN
  };
  enum OutputId {
    CV_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    EXPAND_LIGHT,
    LIGHTS_LEN
  };
  
  float cv[8][8]{};
  
  SpreadMergeExpander() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configLight(EXPAND_LIGHT, "Left connection indicator");
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 1.f, 0.f, "CV mode", {"Sample & Hold", "Track & Hold"});
    for (int i=0; i<8; i++) {
      std::string istr = std::to_string(i+1);
      std::string nm = "CV " + istr;
      configInput(CV_INPUT+i, nm);
    }
    configOutput(CV_OUTPUT, "CV");
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }

};

struct SpreadMergeExpanderWidget : VenomWidget {

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

  SpreadMergeExpanderWidget(SpreadMergeExpander* module) {
    setModule(module);
    setVenomPanel("SpreadMergeExpander");

    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(5.5f,17.f), module, SpreadMergeExpander::EXPAND_LIGHT));
    addParam(createLockableParamCentered<ModeSwitch>(Vec(22.5f, 76.5f), module, SpreadMergeExpander::MODE_PARAM));
    for (int i=0; i<8; i++){
      addInput(createInputCentered<MonoPort>(Vec(22.5f, 118.5f + 27.f*i), module, SpreadMergeExpander::CV_INPUT+i));
    }
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 338.5f), module, SpreadMergeExpander::CV_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    if (this->module) {
      bool connected = false;
      for (Module *leftMod = this->module->getLeftExpander().module; leftMod; leftMod = leftMod->getLeftExpander().module) {
        if (leftMod->model == modelVenomSpreadMerge){
          connected = true;
          break;
        }
        if (leftMod->model != modelVenomSpreadMergeExpander)
          break;
      }
      this->module->lights[0].setBrightness(connected);  
    }
  }
};

}

Model* modelVenomSpreadMergeExpander = createModel<Venom::SpreadMergeExpander, Venom::SpreadMergeExpanderWidget>("SpreadMergeExpander");

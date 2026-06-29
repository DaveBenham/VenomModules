// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct LinearMergeExpander : VenomModule {

  enum ParamId {
    MODE_PARAM,
    SELECT_PARAM,
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
  
  LinearMergeExpander() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configLight(EXPAND_LIGHT, "Left connection indicator");
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 1.f, 0.f, "CV mode", {"Sample & Hold", "Track & Hold"});
    configSwitch<FixedSwitchQuantity>(SELECT_PARAM, 0.f, 5.f, 0.f, "CV selection", {"First channel", "Last channel", "Minimum CV", "Maximum CV", "Average CV", "Sum CV"});
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

struct LinearMergeExpanderWidget : VenomWidget {

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

  LinearMergeExpanderWidget(LinearMergeExpander* module) {
    setModule(module);
    setVenomPanel("LinearMergeExpander");

    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(5.5f,17.f), module, LinearMergeExpander::EXPAND_LIGHT));
    addParam(createLockableParamCentered<ModeSwitch>(Vec(22.5f, 46.5f), module, LinearMergeExpander::MODE_PARAM));
    addParam(createLockableParamCentered<SelectSwitch>(Vec(22.5f, 76.5f), module, LinearMergeExpander::SELECT_PARAM));
    for (int i=0; i<8; i++){
      addInput(createInputCentered<MonoPort>(Vec(22.5f, 118.5f + 27.f*i), module, LinearMergeExpander::CV_INPUT+i));
    }
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f, 338.5f), module, LinearMergeExpander::CV_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    if (this->module) {
      bool connected = false;
      for (Module *leftMod = this->module->getLeftExpander().module; leftMod; leftMod = leftMod->getLeftExpander().module) {
        if (leftMod->model == modelVenomLinearMerge){
          connected = true;
          break;
        }
        if (leftMod->model != modelVenomLinearMergeExpander)
          break;
      }
      this->module->lights[0].setBrightness(connected);  
    }
  }

};

}

Model* modelVenomLinearMergeExpander = createModel<Venom::LinearMergeExpander, Venom::LinearMergeExpanderWidget>("LinearMergeExpander");

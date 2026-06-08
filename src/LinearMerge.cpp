// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include <limits>

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

  enum Selectid {
    FIRST,
    LAST,
    MIN,
    MAX,
    AVG,
    SUM
  };
  
  dsp::TSchmittTrigger<float> trigger[8];
  bool oldGate = false;
  int cnt = 0;
  
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
  
  float getCV(std::vector<Input>& inputs, int select) {
    float cv = (select==MIN) ? std::numeric_limits<float>::max() : (select==MAX ? std::numeric_limits<float>::lowest() : 0.f);
    for (int i=0; i<8; i++) {
      if (trigger[i].isHigh()) {
        float val = inputs[CV_INPUT+i].getVoltage();
        switch(select) {
          case 0: //FIRST:
            return val;
            break;
          case 1: //LAST:
            cv = val;
            break;
          case 2: //MIN:
            if (val < cv)
              cv = val;
            break;
          case 3: //MAX:
            if (val > cv)
              cv = val;
            break;
          default:
            cv += val;
        }
      }
    }
    return select==AVG ? cv/cnt : cv;
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    bool gate = false;
    cnt = 0;
    for (int i=0; i<8; i++) {
      trigger[i].process(inputs[GATE_INPUT+i].getVoltage(), 0.2f, 2.f);
      if (trigger[i].isHigh()) {
        gate = true;
        cnt += 1;
      }
    }
    outputs[GATE_OUTPUT].setVoltage(gate ? 10.f : 0.f);
    if (gate) {
      if (params[MODE_PARAM].getValue() || gate != oldGate)
        outputs[CV_OUTPUT].setVoltage(getCV(inputs, params[SELECT_PARAM].getValue()));
      Module *exp = getRightExpander().module;
      while (exp && exp->model == modelVenomLinearMergeExpander) {
        if (!exp->isBypassed() && (exp->params[MODE_PARAM].getValue() || gate != oldGate))
          exp->outputs[CV_OUTPUT].setVoltage(getCV(exp->inputs, exp->params[SELECT_PARAM].getValue()));
        exp = exp->getRightExpander().module;
      }
    }
    oldGate = gate;
  }

  void onBypass	(const BypassEvent &e) override {
    oldGate = false;
    for (int i=0; i<8; i++)
      trigger[i].process(0.f);
    Module *exp = getRightExpander().module;
    while (exp && exp->model == modelVenomLinearMergeExpander) {
      exp->outputs[CV_OUTPUT].setVoltage(0.f);
      exp = exp->getRightExpander().module;
    }
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

  void appendContextMenu(Menu* menu) override {
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Add CV Expander", "", [this](){addExpander(modelVenomLinearMergeExpander,this);}));
    VenomWidget::appendContextMenu(menu);
  }

};

}

Model* modelVenomLinearMerge = createModel<Venom::LinearMerge, Venom::LinearMergeWidget>("LinearMerge");

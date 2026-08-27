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
    CLOCK_INPUT,
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
    SUM,
    PREV
  };
  
  dsp::TSchmittTrigger<float> trigger[8],
                              clockTrig;
  bool oldGate = false;
  int cnt = 0,
      prev = 0;
  
  LinearMerge() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(CLOCK_INPUT, "Clock");
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 1.f, 0.f, "CV mode", {"Sample & Hold", "Track & Hold"});
    configSwitch<FixedSwitchQuantity>(SELECT_PARAM, 0.f, 5.f, 0.f, "CV selection", {"First port", "Last port", "Minimum CV", "Maximum CV", "Average CV", "Sum CV"});
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
    float cv = 0.f;
    switch(select) {
      case PREV:
        if (prev >= 0)
          return inputs[CV_INPUT+prev].getVoltage();
        else
          select = -prev;
        break;
      case MIN:
        cv = std::numeric_limits<float>::max();
        break;
      case MAX:
        cv = std::numeric_limits<float>::lowest();
        break;
      case AVG:
      case SUM:
        prev = -select;
        break;
    }
    for (int i=0; i<8; i++) {
      if (trigger[i].isHigh()) {
        float val = inputs[CV_INPUT+i].getVoltage();
        switch(select) {
          case FIRST:
            prev = i;
            return val;
            break;
          case LAST:
            prev = i;
            cv = val;
            break;
          case MIN:
            if (val < cv) {
              prev = i;
              cv = val;
            }
            break;
          case MAX:
            if (val > cv) {
              prev = i;
              cv = val;
            }
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
    bool clock = clockTrig.process(inputs[CLOCK_INPUT].getVoltage(), 0.2f, 2.f) || !inputs[CLOCK_INPUT].isConnected(),
         gate = oldGate;
    if (clock) {
      gate = false;
      cnt = 0;
      for (int i=0; i<8; i++) {
        trigger[i].process(inputs[GATE_INPUT+i].getVoltage(), 0.2f, 2.f);
        if (trigger[i].isHigh()) {
          gate = true;
          cnt += 1;
        }
      }
      outputs[GATE_OUTPUT].setVoltage(gate ? 10.f : 0.f);
    }
    if (gate && (params[MODE_PARAM].getValue() || gate != oldGate))
      outputs[CV_OUTPUT].setVoltage(getCV(inputs, params[SELECT_PARAM].getValue()));
    Module *exp = getRightExpander().module;
    while (exp && exp->model == modelVenomLinearMergeExpander) {
      if (gate && !exp->isBypassed() && (exp->params[MODE_PARAM].getValue() || gate != oldGate))
        exp->outputs[CV_OUTPUT].setVoltage(getCV(exp->inputs, exp->params[SELECT_PARAM].getValue()));
      exp = exp->getRightExpander().module;
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

    addInput(createInputCentered<MonoPort>(Vec(20.f, 62.5f), module, LinearMerge::CLOCK_INPUT));
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

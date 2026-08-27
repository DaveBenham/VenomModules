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
  
  int spread = 0,
      ins = 0,
      channels = 0,
      cur[8]{};
  bool gate[8]{},
       trig[8]{};
  dsp::TSchmittTrigger<float> trigger[8];
  
  
  SpreadMerge() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch<FixedSwitchQuantity>(COUNT_PARAM, 0.f, 7.f, 0.f, "Spread channel count", {"Maximize", "2", "3", "4", "5", "6", "7", "8"});
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
  
  void resetEverything() {
    for (int i=0; i<8; i++) {
      gate[i] = trig[i] = trigger[i].process(0.f);
      cur[i] = 0;
    }
    outputs[GATE_OUTPUT].setChannels(0);
    outputs[CV_OUTPUT].setChannels(0);
    Module *exp = getRightExpander().module;
    while (exp && exp->model == modelVenomSpreadMergeExpander) {
      if (!exp->isBypassed())
        exp->outputs[CV_OUTPUT].setChannels(0);
      exp = exp->getRightExpander().module;
    }
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    ins = 7;
    while (ins>=0 && !inputs[GATE_INPUT+ins].isConnected())
      ins--;
    ins++;
    int cnt = params[COUNT_PARAM].getValue()+1;
    if (cnt == 1) // use Maximum available
      cnt = ins ? 16 / ins : 0;
    channels = cnt * ins;
    lights[ERR_LIGHT].setBrightness(channels>16);
    if (channels > 16)
      channels = ins = cnt = 0;
    if (cnt != spread)
      resetEverything();
    spread = cnt;
    for (int i=0; i<ins; i++) {
      trig[i] = trigger[i].process(inputs[GATE_INPUT+i].getVoltage(), 0.2f, 2.f);
      outputs[GATE_OUTPUT].setVoltage(trigger[i].isHigh() ? 10.f : 0.f, i*spread + cur[i]);
      if (gate[i] && !trigger[i].isHigh())
        cur[i] = (cur[i]+1)%spread;
      gate[i] = trigger[i].isHigh();
      if (gate[i] && (trig[i] || params[MODE_PARAM].getValue()))
        outputs[CV_OUTPUT].setVoltage(inputs[CV_INPUT+i].getVoltage(), i*spread + cur[i]);
    }
    outputs[GATE_OUTPUT].setChannels(channels);
    outputs[CV_OUTPUT].setChannels(channels);
    Module *exp = getRightExpander().module;
    while (exp && exp->model == modelVenomSpreadMergeExpander) {
      if (!exp->isBypassed()) {
        for (int i=0; i<ins; i++)
          if (gate[i] && (trig[i] || exp->params[MODE_PARAM].getValue()))
            exp->outputs[CV_OUTPUT].setVoltage(exp->inputs[CV_INPUT+i].getVoltage(), i*spread + cur[i]);
        exp->outputs[CV_OUTPUT].setChannels(channels);
      }
      exp = exp->getRightExpander().module;
    }
  }

  void onBypass	(const BypassEvent &e) override {
    resetEverything();
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
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/select_Max.svg")));
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

    addParam(createLockableParamCentered<CountSwitch>(Vec(20.f, 69.f), module, SpreadMerge::COUNT_PARAM));
    addChild(createLightCentered<SmallLight<RedLight>>(Vec(20.f,86.5f), module, SpreadMerge::ERR_LIGHT));
    addParam(createLockableParamCentered<ModeSwitch>(Vec(55.f, 69.f), module, SpreadMerge::MODE_PARAM));
    for (int i=0; i<8; i++){
      addInput(createInputCentered<MonoPort>(Vec(20.f, 118.5f + 27.f*i), module, SpreadMerge::GATE_INPUT+i));
      addInput(createInputCentered<MonoPort>(Vec(55.f, 118.5f + 27.f*i), module, SpreadMerge::CV_INPUT+i));
    }
    addOutput(createOutputCentered<PolyPort>(Vec(20.f, 338.5f), module, SpreadMerge::GATE_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(55.f, 338.5f), module, SpreadMerge::CV_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Add CV Expander", "", [this](){addExpander(modelVenomSpreadMergeExpander,this);}));
    VenomWidget::appendContextMenu(menu);
  }

};

}

Model* modelVenomSpreadMerge = createModel<Venom::SpreadMerge, Venom::SpreadMergeWidget>("SpreadMerge");

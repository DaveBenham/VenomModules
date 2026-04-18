// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include <algorithm>

#define LIGHT_OFF 0.02f

namespace Venom {

struct PolyPrune : VenomModule {

  enum ParamId {
    SORT_PARAM,
    START_PARAM,
    COUNT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    SELECT_INPUT,
    START_INPUT,
    COUNT_INPUT,
    POLY_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    POLY_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  bool selState[16]{};
  int selected = 1,
      start = 1,
      effCnt = 1;
  
  #define PREASC   1
  #define PREDESC  2
  #define MIDASC   3
  #define MIDDESC  4
  #define POSTASC  5
  #define POSTDESC 6
  
  struct CountQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
      int val = static_cast<int>(getValue());
      return val ? ParamQuantity::getDisplayValueString() : "All";
    }
    void setDisplayValueString(std::string str) override {
      std::transform(str.begin(), str.end(), str.begin(),
                     [](unsigned char c){ return std::tolower(c); });
      if (str == "all")
        ParamQuantity::setDisplayValueString("0");
      else
        ParamQuantity::setDisplayValueString(str);
    }
  };

  PolyPrune() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    
    configInput(SELECT_INPUT, "Select gates");
    configSwitch<FixedSwitchQuantity>(SORT_PARAM, 0.f, 6.f, 0.f, "Sort", {"Off", "Input ascending", "Input descending", "Selection ascending", "Selection descending", "Output ascending", "Output descending"});
    configParam(START_PARAM, 1.f, 16.f, 1.f, "Start");
    configInput(START_INPUT, "Start");
    configParam<CountQuantity>(COUNT_PARAM, -16.f, 16.f, 0.f, "Count");
    configInput(COUNT_INPUT, "Count");
    configInput(POLY_INPUT, "Poly");
    configOutput(POLY_OUTPUT, "Poly");
    configBypass(POLY_INPUT, POLY_OUTPUT);
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    float in[16]{},
          mid[16]{};
    int sort = params[SORT_PARAM].getValue(),
        inCnt = inputs[POLY_INPUT].getChannels(),
        midCnt = 0;
    
    inputs[POLY_INPUT].readVoltages(in);
    if (sort == PREASC)
      std::sort(in, in+inCnt);
    if (sort == PREDESC)
      std::sort(in, in+inCnt, std::greater<float>());

    for (int i=0; i<16; i++) {
      float sel = inputs[SELECT_INPUT].getNormalPolyVoltage(10.f, i);
      if (sel >= 2.f)
        selState[i] = true;
      if (sel <= 0.2f)
        selState[i] = false;
    }

    for (int i=0; i<inCnt; i++) {
      if (selState[i])
        mid[midCnt++] = in[i];
    }
    if (sort == MIDASC)
      std::sort(mid, mid+midCnt);
    if (sort == MIDDESC)
      std::sort(mid, mid+midCnt, std::greater<float>());
    if (!midCnt)
      midCnt = 1;
    selected = midCnt;

    start = clamp(params[START_PARAM].getValue() + std::round(inputs[START_INPUT].getVoltage()*2.f), 1.f, 16.f);
    if (start > midCnt)
      start = midCnt;
    int cnt = clamp(params[COUNT_PARAM].getValue() + std::round(inputs[COUNT_INPUT].getVoltage()*2.f), -16.f, 16.f);
    effCnt = cnt;
    if (!cnt)
      cnt = midCnt;
    int dir = cnt<0 ? -1 : 1;
    cnt = cnt * dir;
    if (cnt > midCnt) {
      cnt = midCnt;
      effCnt = cnt * dir;
    }
    float *out = outputs[POLY_OUTPUT].getVoltages();
    for (int i=0, pos=start-1; i<cnt; i++, pos+=dir) {
      if (pos < 0)
        pos = midCnt-1;
      if (pos >= midCnt)
        pos = 0;
      out[i] = mid[pos];
    }

    if (sort == POSTASC)
      std::sort(out, out+cnt);
    if (sort == POSTDESC)
      std::sort(out, out+cnt, std::greater<float>());
    outputs[POLY_OUTPUT].setChannels(cnt);
  }
  
};

struct PolyPruneWidget : VenomWidget {

  struct SortSwitch : GlowingSvgSwitchLockable {
    SortSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/sort_off.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/sort_preAsc.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/sort_preDesc.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/sort_midAsc.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/sort_midDesc.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/sort_postAsc.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/sort_postDesc.svg")));
    }
  };

  PolyPruneWidget(PolyPrune* module) {
    setModule(module);
    setVenomPanel("PolyPrune");

    addInput(createInputCentered<PolyPort>(Vec(22.5f, 62.f), module, PolyPrune::SELECT_INPUT));
    addParam(createLockableParamCentered<SortSwitch>(Vec(22.5f, 102.f), module, PolyPrune::SORT_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(22.5f, 144.5f), module, PolyPrune::START_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 182.f), module, PolyPrune::START_INPUT));
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(22.5f, 224.f), module, PolyPrune::COUNT_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(22.5f, 261.5f), module, PolyPrune::COUNT_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 301.f), module, PolyPrune::POLY_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 340.5f), module, PolyPrune::POLY_OUTPUT));

  }
  
  void appendContextMenu(Menu *menu) override {
    PolyPrune *module = static_cast<PolyPrune*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Selected", std::to_string(module->selected)));
    menu->addChild(createMenuItem("Effective start", std::to_string(module->start)));
    menu->addChild(createMenuItem("Effective count", module->effCnt ? std::to_string(module->effCnt) : "All"));
    VenomWidget::appendContextMenu(menu);
  }  

};

}

Model* modelVenomPolyPrune = createModel<Venom::PolyPrune, Venom::PolyPruneWidget>("PolyPrune");

// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

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
  
  PolyPrune() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    
    configInput(SELECT_INPUT, "Select gates");
    configSwitch<FixedSwitchQuantity>(SORT_PARAM, 0.f, 6.f, 0.f, "Sort", {"Off", "Pre ascending", "Pre descending", "Mid ascending", "Mid descending", "Post ascending", "Post descending"});
    configParam(START_PARAM, 1.f, 16.f, 1.f, "Start");
    configInput(START_INPUT, "Start");
    configParam(COUNT_PARAM, -16.f, 16.f, 0.f, "Count");
    configInput(COUNT_INPUT, "Count");
    configInput(POLY_INPUT, "Poly");
    configOutput(POLY_OUTPUT, "Poly");
    configBypass(POLY_INPUT, POLY_OUTPUT);
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
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
  
};

}

Model* modelVenomPolyPrune = createModel<Venom::PolyPrune, Venom::PolyPruneWidget>("PolyPrune");

// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "CloneModule.hpp"

namespace Venom {

struct PolyUnison : CloneModuleBase {

  enum ParamId {
    CLONE_PARAM,
    DETUNE_PARAM,
    DIRECTION_PARAM,
    RANGE_PARAM,
    GROUP_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    CLONE_INPUT,
    DETUNE_INPUT,
    POLY_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    POLY_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(CHANNEL_LIGHTS, 32),
    LIGHTS_LEN
  };

  int clones = 1;
  float range[3] = {1.f/12.f, 1.f, 5.f};
  bool vOctDetuneCV = false;

  dsp::ClockDivider lightDivider;

  float detuneParamGetValue() {
    return params[DETUNE_PARAM].getValue() * range[static_cast<int>(params[RANGE_PARAM].getValue())];
  }

  struct DetuneQuantity : ParamQuantity {
    float getDisplayValue() override {
      PolyUnison* module = reinterpret_cast<PolyUnison*>(this->module);
      return module->detuneParamGetValue() / module->range[0]/*semitone*/;
    }
    void setDisplayValue(float val) override {
      PolyUnison* module = reinterpret_cast<PolyUnison*>(this->module);
      setValue(clamp(
       val * module->range[0]/*semitone*/ / module->range[static_cast<int>(module->params[PolyUnison::RANGE_PARAM].getValue())],
       0.f, 1.f
      ));
    }
  };

  PolyUnison() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(CLONE_PARAM, 1.f, 16.f, 1.f, "Unison count");
    configInput(CLONE_INPUT, "Unison count");

    configSwitch<FixedSwitchQuantity>(DIRECTION_PARAM, 0.f, 2.f, 0.f, "Detune direction", {"Center", "Up", "Down"});
    configSwitch<FixedSwitchQuantity>(RANGE_PARAM, 0.f, 2.f, 0.f, "Detune range", {"1 semitone", "1 octave", "5 octaves"});
    configParam<DetuneQuantity>(DETUNE_PARAM, 0.f, 1.f, 0.f, "Detune spread", " semitones");
    configInput(DETUNE_INPUT, "Detune spread");

    configInput(POLY_INPUT, "Poly");
    configSwitch<FixedSwitchQuantity>(GROUP_PARAM, 0.f, 1.f, 0.f, "Output grouping", {"Input channel", "Input set"});
    configOutput(POLY_OUTPUT, "Poly");
    configBypass(POLY_INPUT, POLY_OUTPUT);
    for (int i=0; i<16; i++){
      configLight(CHANNEL_LIGHTS+i*2, string::f("Channel %d clone indicator", i+1))->description = "yellow = OK, red = Error";
    }  
    lightDivider.setDivision(44);
    lights[CHANNEL_LIGHTS].setBrightness(1);
    lights[CHANNEL_LIGHTS+1].setBrightness(0);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    // Get number of clones
    clones = clamp(static_cast<int>(params[CLONE_PARAM].getValue() + round(inputs[CLONE_INPUT].getVoltage()*3.f)), 1, 16);

    // Get number of channels
    int maxCh = 16/clones;
    int ch = std::max({1,inputs[POLY_INPUT].getChannels()});
    int goodCh = ch > maxCh ? maxCh : ch;
    
    // Get detune
    float spread = 0.f;
    float delta = 0.f;
    if (clones>1) {
      spread = detuneParamGetValue() + inputs[DETUNE_INPUT].getVoltage() * (vOctDetuneCV ? 1.f : range[static_cast<int>(params[RANGE_PARAM].getValue())] / 10.f);
      delta = spread / (clones-1);
    }
    float start = 0.f;
    switch (static_cast<int>(params[DIRECTION_PARAM].getValue())) {
      case 0: // Center
        start = -spread / 2;
        break;
      case 1: // Up
        start = 0.f;
        break;
      case 2: // Down
        start = -spread;
        break;
    }

    if (params[GROUP_PARAM].getValue()){ //group by input channel set
      for (int i=0; i<goodCh; i++) {
        float val = inputs[POLY_INPUT].getVoltage(i) + start;
        for (int c=0, o=i; c<clones; c++, o+=goodCh, val+=delta)
          outputs[POLY_OUTPUT].setVoltage(val, o);
      }
    }
    else{ //group by individual input channel
      for (int i=0, o=0; i<goodCh; i++) {
        float val = inputs[POLY_INPUT].getVoltage(i) + start;
        for (int j=0; j<clones; j++, val+=delta)
          outputs[POLY_OUTPUT].setVoltage(val, o++);
      }
    }
    outputs[POLY_OUTPUT].setChannels(goodCh * clones);
    processExpander(clones, goodCh);

    if (lightDivider.process()) {
      for (int i=1; i<16; i++) {
        lights[CHANNEL_LIGHTS+i*2].setBrightness(i<goodCh);
        lights[CHANNEL_LIGHTS+i*2+1].setBrightness(i>=goodCh && i<ch);
      }
      setExpanderLights(goodCh);
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "vOctDetuneCV", json_boolean(vOctDetuneCV));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "vOctDetuneCV")))
      vOctDetuneCV = json_boolean_value(val);
    else
      vOctDetuneCV = true;
  }

};


struct PolyUnisonWidget : CloneModuleWidget {

  struct PCCountDisplay : DigitalDisplay18 {
    void step() override {
      if (module) {
        PolyUnison* mod = dynamic_cast<PolyUnison*>(module);
        text = string::f("%d", mod->clones);
        fgColor = SCHEME_YELLOW;
      } else {
        text = "16";
        fgColor = SCHEME_YELLOW;
      }
    }
  };

  struct DirectionSwitch : GlowingSvgSwitchLockable {
    DirectionSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
    }
  };

  struct RangeSwitch : GlowingSvgSwitchLockable {
    RangeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };

  PolyUnisonWidget(PolyUnison* module) {
    setModule(module);
    setVenomPanel("PolyUnison");

    PCCountDisplay* countDisplay = createWidget<PCCountDisplay>(Vec(10.316, 38.431));
    countDisplay->module = module;
    addChild(countDisplay);
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(22.5,91.941), module, PolyUnison::CLONE_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(22.5,124.974), module, PolyUnison::CLONE_INPUT));

    addParam(createLockableParamCentered<DirectionSwitch>(Vec(13.012f,153.106f), module, PolyUnison::DIRECTION_PARAM));
    addParam(createLockableParamCentered<RangeSwitch>(Vec(31.989f,153.106), module, PolyUnison::RANGE_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5,184.026), module, PolyUnison::DETUNE_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(22.5,217.079), module, PolyUnison::DETUNE_INPUT));

    {
      int li, end;
      float x, y, delta=7.557f;
      for (li=0, end=8, x=11.160f; li<32; end+=8, x+=delta){
        for(y=261.593f; li < end; li+=2, y-=delta){
          addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(x,y), module, PolyUnison::CHANNEL_LIGHTS+li));
        }
      }
    }

    addInput(createInputCentered<PolyPort>(Vec(22.5,287.712), module, PolyUnison::POLY_INPUT));
    addParam(createLockableParamCentered<GroupSwitch>(Vec(22.5f,308.f), module, PolyUnison::GROUP_PARAM));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5,340.434), module, PolyUnison::POLY_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    PolyUnison* module = static_cast<PolyUnison*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("V/Oct Detune CV", "", &module->vOctDetuneCV));
    CloneModuleWidget::appendContextMenu(menu);
  }
};

}

Model* modelVenomPolyUnison = createModel<Venom::PolyUnison, Venom::PolyUnisonWidget>("PolyUnison");

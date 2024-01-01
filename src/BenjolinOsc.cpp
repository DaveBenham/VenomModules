// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

struct BenjolinOsc : VenomModule {
  
  enum ParamId {
    OVER_PARAM,
    FREQ1_PARAM,
    FREQ2_PARAM,
    RUNG1_PARAM,
    RUNG2_PARAM,
    CV1_PARAM,
    CV2_PARAM,
    PATTERN_PARAM,
    CHAOS_PARAM,
    DOUBLE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    CV1_INPUT,
    CV2_INPUT,
    CHAOS_INPUT,
    DOUBLE_INPUT,
    CLOCK_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    TRI1_OUTPUT,
    TRI2_OUTPUT,
    PULSE1_OUTPUT,
    PULSE2_OUTPUT,
    XOR_OUTPUT,
    PWM_OUTPUT,
    RUNG_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    CHAOS_LIGHT,
    DOUBLE_LIGHT,
    LIGHTS_LEN
  };
 
  BenjolinOsc() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    configParam(FREQ1_PARAM, -1.f, 1.f, 0.f, "Oscillator 1 frequency");
    configParam(FREQ2_PARAM, -1.f, 1.f, 0.f, "Oscillator 2 frequency");
    configParam(RUNG1_PARAM, -1.f, 1.f, 0.f, "Oscillator 1 rungler modulation amount");
    configParam(RUNG2_PARAM, -1.f, 1.f, 0.f, "Oscillator 2 rungler modulation amount");
    configParam(CV1_PARAM, -1.f, 1.f, 0.f, "Oscillator 1 CV modulation amount");
    configParam(CV2_PARAM, -1.f, 1.f, 0.f, "Oscillator 2 CV modulation amount");
    configInput(CV1_INPUT, "Oscillator 1 CV modulation");
    configInput(CV2_INPUT, "Oscillator 2 CV modulation");
    configOutput(TRI1_OUTPUT, "Oscillator 1 triangle");
    configOutput(TRI2_OUTPUT, "Oscillator 2 triangle");
    configOutput(PULSE1_OUTPUT, "Oscillator 1 pulse");
    configOutput(PULSE2_OUTPUT, "Oscillator 2 pulse");
    configParam(PATTERN_PARAM, -1.f, 1.f, 0.f, "Rungler cycle chance");
    configSwitch<FixedSwitchQuantity>(CHAOS_PARAM, 0.f, 1.f, 0.f, "Maximum chaos", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(DOUBLE_PARAM, 0.f, 1.f, 0.f, "Double clock", {"Off", "On"});
    configInput(CHAOS_INPUT,"Maximum chaos trigger");
    configInput(DOUBLE_INPUT,"Double clock trigger");
    configInput(CLOCK_INPUT,"Clock");
    configOutput(XOR_OUTPUT,"XOR");
    configOutput(PWM_OUTPUT,"PWM");
    configOutput(RUNG_OUTPUT,"Rungler");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
};

struct BenjolinOscWidget : VenomWidget {

  struct OverSwitch : GlowingSvgSwitchLockable {
    OverSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
    }
  };

  BenjolinOscWidget(BenjolinOsc* module) {
    setModule(module);
    setVenomPanel("BenjolinOsc");
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(24.865,96.097), module, BenjolinOsc::FREQ1_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(63.288,96.097), module, BenjolinOsc::FREQ2_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(120.923,76.204), module, BenjolinOsc::OVER_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(24.865,148.33), module, BenjolinOsc::RUNG1_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(63.288,148.33), module, BenjolinOsc::RUNG2_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(120.923,148.33), module, BenjolinOsc::PATTERN_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(24.865,202.602), module, BenjolinOsc::CV1_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(63.288,202.602), module, BenjolinOsc::CV2_PARAM));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(101.712,202.602), module, BenjolinOsc::CHAOS_PARAM, BenjolinOsc::CHAOS_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(140.135,202.602), module, BenjolinOsc::DOUBLE_PARAM, BenjolinOsc::DOUBLE_LIGHT));
    addInput(createInputCentered<PJ301MPort>(Vec(24.865,235.665), module, BenjolinOsc::CV1_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(63.288,235.665), module, BenjolinOsc::CV2_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(101.712,235.665), module, BenjolinOsc::CHAOS_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(140.135,235.665), module, BenjolinOsc::DOUBLE_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(24.865,288.931), module, BenjolinOsc::TRI1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(63.288,288.931), module, BenjolinOsc::TRI2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(101.712,288.931), module, BenjolinOsc::XOR_OUTPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(140.135,288.931), module, BenjolinOsc::CLOCK_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(24.865,330.368), module, BenjolinOsc::PULSE1_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(63.288,330.368), module, BenjolinOsc::PULSE2_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(101.712,330.368), module, BenjolinOsc::PWM_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(140.135,330.368), module, BenjolinOsc::RUNG_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    BenjolinOsc* mod = dynamic_cast<BenjolinOsc*>(this->module);
    if(mod) {
      mod->lights[BenjolinOsc::CHAOS_LIGHT].setBrightness(mod->params[BenjolinOsc::CHAOS_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      mod->lights[BenjolinOsc::DOUBLE_LIGHT].setBrightness(mod->params[BenjolinOsc::DOUBLE_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }
  }
};

Model* modelBenjolinOsc = createModel<BenjolinOsc, BenjolinOscWidget>("BenjolinOsc");

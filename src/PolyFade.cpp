// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"
#include <float.h>

struct PolyFade : VenomModule {

  enum ParamId {
    RISE_PARAM,
    RISE_AMT_PARAM,
    FALL_PARAM,
    FALL_AMT_PARAM,
    WIDTH_PARAM,
    WIDTH_AMT_PARAM,
    HOLD_PARAM,
    HOLD_AMT_PARAM,
    SKEW_PARAM,
    SKEW_AMT_PARAM,
    RATE_PARAM,
    DIR_PARAM,
    CHAN_PARAM,
    START_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RISE_INPUT,
    FALL_INPUT,
    WIDTH_INPUT,
    HOLD_INPUT,
    SKEW_INPUT,
    RATE_INPUT,
    DIR_INPUT,
    CHAN_INPUT,
    START_INPUT,
    PHASOR_INPUT,
    POLY_INPUT,
    RESET_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    GATES_OUTPUT,
    POLY_OUTPUT,
    SUM_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(CHAN_LIGHT,16),
    ENUMS(CHAN_ACTIVE_LIGHT,16),
    LIGHTS_LEN
  };
  
  float phasor = 0.f;
  float prevPhasor = 0.f;
  float baseFreq = dsp::FREQ_C4/128.f;
  dsp::SchmittTrigger resetTrig;
  
  struct ChannelQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
      if (getImmediateValue())
        return ParamQuantity::getDisplayValueString();
      else
        return "Auto";
    }
    void setDisplayValueString(std::string val) override {
      if (rack::string::lowercase(val) == "auto")
        setValue(0.f);
      else
        ParamQuantity::setDisplayValueString(val);
    }
  };

  PolyFade() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    
    configParam(RISE_PARAM, -1.f, 1.f, 0.f, "Rise shape", "%", 0, 100, 0);
    configParam(RISE_AMT_PARAM, -1.f, 1.f, 0.f, "Rise CV amount", "%", 0, 100, 0);
    configInput(RISE_INPUT, "Rise CV");
    
    configParam(FALL_PARAM, -1.f, 1.f, 0.f, "Fall shape", "%", 0, 100, 0);
    configParam(FALL_AMT_PARAM, -1.f, 1.f, 0.f, "Fall CV amount", "%", 0, 100, 0);
    configInput(FALL_INPUT, "Fall CV");
    
    configParam(WIDTH_PARAM, -4.f, 4.f, 1.f, "Width", "", 2, 1, 0);
    configParam(WIDTH_AMT_PARAM, -1.f, 1.f, 0.f, "Width CV amount", "%", 0, 100, 0);
    configInput(WIDTH_INPUT, "Width CV");
    
    configParam(HOLD_PARAM, 0.f, 1.f, 0.0f, "Hold", "%", 0, 100, 0);
    configParam(HOLD_AMT_PARAM, -1.f, 1.f, 0.f, "Hold CV amount", "%", 0, 100, 0);
    configInput(HOLD_INPUT, "Hold CV");
    
    configParam(SKEW_PARAM, 0.f, 1.f, 0.5f, "Skew", "%", 0, 100, 0);
    configParam(SKEW_AMT_PARAM, -1.f, 1.f, 0.f, "Skew CV amount", "%", 0, 100, 0);
    configInput(SKEW_INPUT, "Skew CV");
    
    for (int i=0; i<16; i++){
      std::string nm = "Channel " + std::to_string(i+1);
      configLight( CHAN_LIGHT+i, nm + " level");
      configLight( CHAN_ACTIVE_LIGHT+i, nm + " active");
    }

    configParam(RATE_PARAM, -5.f, 5.f, 0.f, "Rate", " Hz", 2.f, baseFreq);
    configInput(RATE_INPUT, "Rate CV");
    
    configSwitch<FixedSwitchQuantity>(DIR_PARAM, 0.f, 3.f, 0.f, "Direction", {"Forward", "Backward", "Ping Pong", "Off"});
    configInput(DIR_INPUT, "Direction CV");

    configParam<ChannelQuantity>(CHAN_PARAM, 0.f, 16.f, 0.f, "Channels");
    configInput(CHAN_INPUT, "Channels CV");

    configParam(START_PARAM, 0.f, 15.f, 0.f, "Start channel", "", 0.f, 1.f, 1.f);
    configInput(START_INPUT, "Start channel CV");
    
    configInput(PHASOR_INPUT, "Phasor");
    configInput(POLY_INPUT, "Poly");
    configInput(RESET_INPUT, "Reset");
    
    configOutput(GATES_OUTPUT, "Gates");
    configOutput(POLY_OUTPUT, "Poly");
    configOutput(SUM_OUTPUT, "Sum");

  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    float k =  args.sampleTime;
    float freq = math::clamp(dsp::exp2_taylor5(params[RATE_PARAM].getValue() + inputs[RATE_INPUT].getVoltage())*baseFreq, 0.001f, 12000.f);
    int dir = static_cast<int>(params[DIR_PARAM].getValue());
    phasor = fmod(phasor + freq*k , 1.f);
    if (resetTrig.process(inputs[RESET_INPUT].getVoltage(), 0.2f, 2.f))
      phasor = 0.f;
    float tempPhasor = 0.f;
    switch (dir){
      case 0: // forward = rising ramp
        tempPhasor = phasor;
        break;
      case 1: // backward = falling ramp
        tempPhasor = 1.f - phasor;
        break;
      case 2: // ping pong = triangle
        tempPhasor = ((phasor <= 0.5f) ? phasor : 0.5f - phasor) * 2.f;
        break;
      default: // off, reset phasor
        phasor = 0.f;
    }
    tempPhasor = fmod(tempPhasor + inputs[PHASOR_INPUT].getVoltage()/10.f, 1.f);
    if (tempPhasor<0.f)
      tempPhasor += 1.f;

    int channels = static_cast<int>(params[CHAN_PARAM].getValue());
    if (!channels)
      channels = std::max(inputs[POLY_INPUT].getChannels(), 1);
    int startChannel = static_cast<int>(params[START_PARAM].getValue());
    float chanWidth = 1.f / channels;
    float width = math::clamp(dsp::exp2_taylor5(params[WIDTH_PARAM].getValue()), 0.f, static_cast<float>(channels)) * chanWidth;
    float start = (chanWidth - width) / 2.f;
    float holdWidth = params[HOLD_PARAM].getValue() * width;
    float riseWidth = params[SKEW_PARAM].getValue() * (width - holdWidth);
    float fallWidth = width - holdWidth - riseWidth;
    float endRise = start + riseWidth;
    float endHold = endRise + holdWidth;
    float endFall = endHold + fallWidth;
    float riseShape = params[RISE_PARAM].getValue() * 0.9f;
    float fallShape = params[FALL_PARAM].getValue() * 0.9f;
    bool activeLight[16]{};
    
    float out = 0.f;
    float sum = 0.f;
    float gate = 0.f;
    if (tempPhasor > 1 + start)
      tempPhasor -= 1.f;
/*      
    outputs[GATES_OUTPUT].setVoltage(channels,0);
    outputs[GATES_OUTPUT].setVoltage(startChannel,1);
    outputs[GATES_OUTPUT].setVoltage(chanWidth,2);
    outputs[GATES_OUTPUT].setVoltage(width,3);
    outputs[GATES_OUTPUT].setVoltage(riseWidth,4);
    outputs[GATES_OUTPUT].setVoltage(holdWidth,5);
    outputs[GATES_OUTPUT].setVoltage(fallWidth,6);
    outputs[GATES_OUTPUT].setVoltage(start,7);
    outputs[GATES_OUTPUT].setVoltage(endRise,8);
    outputs[GATES_OUTPUT].setVoltage(endHold,9);
    outputs[GATES_OUTPUT].setVoltage(endFall,10);
    outputs[GATES_OUTPUT].setVoltage(tempPhasor,11);
    outputs[GATES_OUTPUT].setChannels(12);
    return;
*/    
    for (int i=0; i<channels; i++) {
      if (tempPhasor < start){            // pre-start
        out = 0.f;
        gate = 0.f;
      }
      else if (tempPhasor < endRise) {   // rise
        out = normSigmoid((tempPhasor - start) / riseWidth, riseShape);
        gate = 10.f;
      }
      else if (tempPhasor <= endHold) {  // hold
        out = 1.f;
        gate = 10.f;
      }
      else if (tempPhasor < endFall) {   // fall
        out = normSigmoid(1.f - (tempPhasor - endHold) / fallWidth, fallShape);
        gate = 10.f;
      }
      else {                             // post end
        out = 0.f;
        gate = 0.f;
      }
      int c = (i+startChannel)%16;
      lights[CHAN_LIGHT+c].setBrightnessSmooth(out, args.sampleTime);
      lights[CHAN_ACTIVE_LIGHT+c].setBrightness(i==0 ? 1.f : 0.2f);
      out *= inputs[POLY_INPUT].getNormalVoltage(10.f, c);
      outputs[POLY_OUTPUT].setVoltage(out, i);
      outputs[GATES_OUTPUT].setVoltage(gate, i);
      sum += out;
      tempPhasor -= chanWidth;
      if (tempPhasor < start)
        tempPhasor += 1.f;
    }
    for (int c=0; c<16; c++) {
      if (!activeLight[c]) {
        lights[CHAN_LIGHT+c].setBrightnessSmooth(0.f, args.sampleTime);
        lights[CHAN_ACTIVE_LIGHT+c].setBrightnessSmooth(0.f, args.sampleTime);
      }
    }
    outputs[SUM_OUTPUT].setVoltage(sum);
    outputs[POLY_OUTPUT].setChannels(channels);
    outputs[GATES_OUTPUT].setChannels(channels);
  }

};

struct PolyFadeWidget : VenomWidget {

  struct DirSwitch : GlowingSvgSwitchLockable {
    DirSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/dir_right.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/dir_left.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/dir_left_right.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/dir_off.svg")));
    }
  };

  PolyFadeWidget(PolyFade* module) {
    setModule(module);
    setVenomPanel("PolyFade");

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(41.f, 47.5f), module, PolyFade::RISE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(62.f, 47.5f), module, PolyFade::RISE_AMT_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(51.5f, 73.5f), module, PolyFade::RISE_INPUT));
    
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(88.f, 47.5f), module, PolyFade::FALL_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(109.f, 47.5f), module, PolyFade::FALL_AMT_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(98.5f, 73.5f), module, PolyFade::FALL_INPUT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(17.5f, 117.f), module, PolyFade::WIDTH_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(38.5f, 117.f), module, PolyFade::WIDTH_AMT_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(28.f, 143.f), module, PolyFade::WIDTH_INPUT));
    
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(64.5f, 117.f), module, PolyFade::HOLD_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(85.5f, 117.f), module, PolyFade::HOLD_AMT_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(75.f, 143.f), module, PolyFade::HOLD_INPUT));
    
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(111.5f, 117.f), module, PolyFade::SKEW_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(132.5f, 117.f), module, PolyFade::SKEW_AMT_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(122.f, 143.f), module, PolyFade::SKEW_INPUT));
    
    for (int i=0; i<16; i++){
      addChild(createLightCentered<TinyLight<GreenLight>>(Vec(6.5f+9.1333f*i, 168.f), module, PolyFade::CHAN_ACTIVE_LIGHT+i));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(6.5f+9.1333f*i, 174.5f), module, PolyFade::CHAN_LIGHT+i));
    }
    
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(23.5f, 211.5f), module, PolyFade::RATE_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(23.5f, 242.f), module, PolyFade::RATE_INPUT));

    addParam(createLockableParamCentered<DirSwitch>(Vec(57.8333f, 212.5f), module, PolyFade::DIR_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(57.8333f, 242.f), module, PolyFade::DIR_INPUT));

    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(Vec(92.1667f, 211.5f), module, PolyFade::CHAN_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(92.1667f, 242.f), module, PolyFade::CHAN_INPUT));

    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(Vec(126.5f, 211.5f), module, PolyFade::START_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(126.5f, 242.f), module, PolyFade::START_INPUT));

    addInput(createInputCentered<MonoPort>(Vec(32.f, 288.5f), module, PolyFade::PHASOR_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(75.f, 288.5f), module, PolyFade::POLY_INPUT));
    addInput(createInputCentered<MonoPort>(Vec(118.f, 288.5f), module, PolyFade::RESET_INPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(32.f, 335.5f), module, PolyFade::GATES_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(75.f, 335.5f), module, PolyFade::POLY_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(118.f, 335.5f), module, PolyFade::SUM_OUTPUT));
    
  }

};

Model* modelPolyFade = createModel<PolyFade, PolyFadeWidget>("PolyFade");

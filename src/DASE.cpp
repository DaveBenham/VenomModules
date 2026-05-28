// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "Filter.hpp"
#include "math.hpp"
#include <dsp/digital.hpp>

namespace Venom {

struct DASE : VenomModule {

  enum ParamId {
    LEN_PARAM,
    ATK_PARAM,
    LEVEL_PARAM,
    RESP_PARAM,
    LEN_CV_PARAM,
    ATK_CV_PARAM,
    LEVEL_CV_PARAM,
    RESP_CV_PARAM,
    SYNC_PARAM,
    OVER_PARAM,
    RATE_PARAM,
    DEPTH_PARAM,
    SHAPE_PARAM,
    RATE_CV_PARAM,
    DEPTH_CV_PARAM,
    SHAPE_CV_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    LEN_CV_INPUT,
    ATK_CV_INPUT,
    LEVEL_CV_INPUT,
    RESP_CV_INPUT,
    RATE_CV_INPUT,
    DEPTH_CV_INPUT,
    SHAPE_CV_INPUT,
    TRIG_INPUT,
    MAIN_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    MAIN_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  using float_4 = simd::float_4;
  int oversample = 0;
  float sampleRate = 0;
  int oversampleValues[6]{1,2,4,8,16,32};
  bool sync = false;
  OversampleFilter_4 upSample[4]{}, 
                     downSample[4]{};
  DCBlockFilter_4 dcBlockFilter[4]{};
  dsp::TSchmittTrigger<float_4> trigger[4]{};
  float_4 envPhase[4]{},
          envActive[4]{},
          rptPhase[4]{};

  DASE() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam(LEN_PARAM, -5.f, 5.f, 0.f, "Envelope length", " sec", 2.f, 1.f, 0.f);
    configParam(LEN_CV_PARAM, -1.f, 1.f, 0.f, "Envelope length CV amount", "%", 0, 100, 0);
    configInput(LEN_CV_INPUT, "Envelope length CV");

    configParam(ATK_PARAM, 0.f, 1.f, 0.f, "Attack ratio", "%", 0, 100, 0);
    configParam(ATK_CV_PARAM, -0.1f, 0.1f, 0.f, "Attack ratio CV amount", "%", 0, 1000, 0);
    configInput(ATK_CV_INPUT, "Attack ratio CV");

    configParam(LEVEL_PARAM, 0.f, 0.1f, 0.1f, "Input level", "%", 0, 1000, 0);
    configParam(LEVEL_CV_PARAM, -0.01f, 0.01f, 0.f, "Input level CV amount", "%", 0, 10000, 0);
    configInput(LEVEL_CV_INPUT, "Input level CV");

    configParam(RESP_PARAM, -1.f, 1.f, 0.f, "Output response", "");
    configParam(RESP_CV_PARAM, -0.1f, 0.1f, 0.f, "Output response CV amount", "%", 0, 1000, 0);
    configInput(RESP_CV_INPUT, "Output response CV");

    configSwitch<FixedSwitchQuantity>(SYNC_PARAM, 0.f, 1.f, 0.f, "Repeat sync", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});

    configParam(RATE_PARAM, -4.f, 4.f, 0.f, "Repeat rate", " BPM", 2.f, 120.f, 0.f);
    configParam(RATE_CV_PARAM, -1.f, 1.f, 0.f, "Repeate rate CV amount", "%", 0, 100, 0);
    configInput(RATE_CV_INPUT, "Repeat rate CV");

    configParam(DEPTH_PARAM, -1.f, 1.f, 0.f, "Repeat level", "");
    configParam(DEPTH_CV_PARAM, -0.1f, 0.1f, 0.f, "Repeat level CV amount", "%", 0, 1000, 0);
    configInput(DEPTH_CV_INPUT, "Repeat level CV");

    configParam(SHAPE_PARAM, -1.f, 1.f, 0.f, "Repeat shape", "");
    configParam(SHAPE_CV_PARAM, -1.f, 1.f, 0.f, "Repeat shape CV amount", "%", 0, 100, 0);
    configInput(SHAPE_CV_INPUT, "Repeat shape CV");

    configInput(TRIG_INPUT, "Trigger");
    configInput(MAIN_INPUT, "Main");
    configOutput(MAIN_OUTPUT, "Main");

    oversampleStages = 5;
  }
  
  void setOversample() override {
    if (oversample > 1) {
      for (int s=0; s<4; s++){
        upSample[s].setOversample(oversample, oversampleStages);
        downSample[s].setOversample(oversample, oversampleStages);
      }
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    // update oversample configuration
    if (oversample != oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())]) {
      oversample = oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())];
      setOversample();
      sampleRate = 0.f;
    }
    // update DC Block configuration
    if (sampleRate != args.sampleRate){
      sampleRate = args.sampleRate;
      for (int i=0; i<4; i++){
        dcBlockFilter[i].init(oversample, sampleRate);
      }
    }
    // get channel count
    int channels = std::max({1, inputs[TRIG_INPUT].getChannels(), inputs[MAIN_INPUT].getChannels()});
    for (int i=0; i<4; i++)
      channels = std::max({channels, inputs[i].getChannels()});

    if (sync != static_cast<bool>(params[SYNC_PARAM].getValue())) {
      sync = !sync;
      if (sync)
        rptPhase[0] = 0.f;
    }
    float lenParam = params[LEN_PARAM].getValue(),
          lenAmt = params[LEN_CV_PARAM].getValue(),
          atkParam = params[ATK_PARAM].getValue(),
          atkAmt = params[ATK_CV_PARAM].getValue(),
          lvlParam = params[LEVEL_PARAM].getValue(),
          lvlAmt = params[LEVEL_CV_PARAM].getValue(),
          respParam = params[RESP_PARAM].getValue(),
          respAmt = params[RESP_CV_PARAM].getValue(),
          rptDelta = std::max(pow(2.f, params[RATE_PARAM].getValue() + inputs[RATE_CV_PARAM].getVoltage() * params[RATE_CV_PARAM].getValue()) * 2.f / sampleRate, 1e-6),
          rptAtk = clamp(params[SHAPE_PARAM].getValue() + inputs[SHAPE_CV_INPUT].getVoltage() * params[SHAPE_CV_PARAM].getValue()),
          depth = clamp(params[DEPTH_PARAM].getValue() + inputs[DEPTH_CV_INPUT].getVoltage() * params[DEPTH_CV_PARAM].getValue());
    float_4 rpt{};
    if (!sync) {
       rptPhase[0] = fmin(rptPhase[0] + rptDelta, 1.f);
       rpt = (rptPhase[0][0]<rptAtk ? rptPhase[0]/rptAtk : ((1.f-rptAtk)<=1e-6f ? float_4::zero() : (1.f-rptPhase[0])/(1.f-rptAtk))) * depth;
    }
    // channel loop
    for (int s=0, c=0; c<channels; s++, c+=4){
      float_4 envDelta = fmax( 1.f / (pow(2.f, lenParam + inputs[LEN_CV_INPUT].getPolyVoltageSimd<float_4>(c) * lenAmt) * sampleRate), 1e-6f),
              newTrig = trigger[s].process(inputs[TRIG_INPUT].getPolyVoltageSimd<float_4>(c), 0.2f, 2.f),
              baseAtk = clamp(atkParam + inputs[ATK_CV_INPUT].getPolyVoltageSimd<float_4>(c) * atkAmt);
      envActive[s] = ifelse(newTrig, 1.f, envActive[s]);
      envPhase[s] = ifelse(newTrig, 0.f, envPhase[s]);
      envPhase[s] = fmin(envPhase[s] + envDelta * envActive[s], 1.f);
      if (sync) {
        rptPhase[s] = ifelse(newTrig, 0.f, rptPhase[s]);
        rptPhase[s] = fmin(rptPhase[s] + rptDelta, 1.f);
        rpt = ifelse(rptPhase[s]<rptAtk, rptPhase[s]/rptAtk, (1.f-rptPhase[s])/(1.f-rptAtk)) * depth;
      }
      float_4 mainIn = inputs[MAIN_INPUT].getPolyVoltageSimd<float_4>(c),
              envOut = 0.f,
              lvl = lvlParam + inputs[LEVEL_CV_INPUT].getPolyVoltageSimd<float_4>(c) * lvlAmt,
              shape = clamp(respParam + inputs[RESP_CV_INPUT].getPolyVoltageSimd<float_4>(c) * respAmt) * 0.9f;
      // oversample loop
      for (int o=0; o<oversample; o++) {
        // upsample inputs
        if (oversample > 1){
          mainIn = upSample[s].process(o ? float_4::zero() : mainIn*oversample);
        }
        float_4 atk = clamp(baseAtk + mainIn*lvl + rpt[sync ? s : 0]);
        envOut = ifelse(envPhase[s]<atk, envPhase[s]/atk, ifelse((1.f-atk)<=1e-6f, 0.f, (1.f-envPhase[s])/(1.f-atk)))*envActive[s];
        envOut = normSigmoid(envOut, -shape); 
        envOut = dcBlockFilter[s].process(envOut);
        // downsample output
        if (oversample > 1)
          envOut = downSample[s].process(envOut);
      } // end oversample loop
      // write output
      outputs[MAIN_OUTPUT].setVoltageSimd(envOut*5.f, c);
      envPhase[s] = ifelse(envPhase[s]>=1.f, 0.f, envPhase[s]);
      envActive[s] = ifelse(envPhase[s]>0.f, 1.f, 0.f);
      if (sync || s==0)
        rptPhase[s] = ifelse(rptPhase[s]>=1.f, 0.f, rptPhase[s]);
    } // end channel loop
    // set output channel count
    outputs[MAIN_OUTPUT].setChannels(channels);
  }

};

struct DASEWidget : VenomWidget {

  struct SyncSwitch : GlowingSvgSwitchLockable {
    SyncSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
    }
  };

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

  DASEWidget(DASE* module) {
    setModule(module);
    setVenomPanel("DASE");
    for (int i=0; i<4; i++) {
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(20.5f, 49.5f+i*38.f), module, DASE::LEN_PARAM+i));
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 49.5f+i*38.f), module, DASE::LEN_CV_PARAM+i));
      addInput(createInputCentered<PolyPort>(Vec(84.5f, 49.5f+i*38.f), module, DASE::LEN_CV_INPUT+i));
      if (i<3) {
        addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(20.5f, 222.5f+i*38.f), module, DASE::RATE_PARAM+i));
        addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 222.5f+i*38.f), module, DASE::RATE_CV_PARAM+i));
        addInput(createInputCentered<MonoPort>(Vec(84.5f, 222.5f+i*38.f), module, DASE::RATE_CV_INPUT+i));
      }
    }
    addParam(createLockableParamCentered<SyncSwitch>(Vec(14.5f, 187.5f), module, DASE::SYNC_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(62.5f, 187.5f), module, DASE::OVER_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(20.5f, 341.5f), module, DASE::TRIG_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(52.5f, 341.5f), module, DASE::MAIN_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(84.5f, 341.5f), module, DASE::MAIN_OUTPUT));
  }

};

}

Model* modelVenomDASE = createModel<Venom::DASE, Venom::DASEWidget>("DASE");

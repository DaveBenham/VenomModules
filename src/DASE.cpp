// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "Filter.hpp"
#include "math.hpp"

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
  
  int oversample = 0;
  float sampleRate = 0;
  int oversampleValues[6]{1,2,4,8,16,32};
  OversampleFilter_4 upSample[8][4]{}, downSample[4]{};
  DCBlockFilter_4 dcBlockInFilter[4]{}, dcBlockOutFilter[4]{};

  DASE() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam(LEN_PARAM, -5.f, 5.f, 0.f, "Envelope length", " sec");
    configParam(LEN_CV_PARAM, -1.f, 1.f, 0.f, "Envelope length CV amount", "%", 0, 100, 0);
    configInput(LEN_CV_INPUT, "Envelope length CV");

    configParam(ATK_PARAM, 0.f, 1.f, 0.f, "Attack ratio", "%", 0, 100, 0);
    configParam(ATK_CV_PARAM, -1.f, 1.f, 0.f, "Attack ratio CV amount", "%", 0, 100, 0);
    configInput(ATK_CV_INPUT, "Attack ratio CV");

    configParam(LEVEL_PARAM, 0.f, 1.f, 1.f, "Input level", "%", 0, 100, 0);
    configParam(LEVEL_CV_PARAM, -1.f, 1.f, 0.f, "Input level CV amount", "%", 0, 100, 0);
    configInput(LEVEL_CV_INPUT, "Input level CV");

    configParam(RESP_PARAM, -1.f, 1.f, 0.f, "Output response", "");
    configParam(RESP_CV_PARAM, -1.f, 1.f, 0.f, "Output response CV amount", "%", 0, 100, 0);
    configInput(RESP_CV_INPUT, "Output response CV");

    configSwitch<FixedSwitchQuantity>(SYNC_PARAM, 0.f, 1.f, 0.f, "Repeat sync", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});

    configParam(RATE_PARAM, -1.f, 1.f, 0.f, "Repeat rate", "BPM");
    configParam(RATE_CV_PARAM, -1.f, 1.f, 0.f, "Repeate rate CV amount", "%", 0, 100, 0);
    configInput(RATE_CV_INPUT, "Repeat rate CV");

    configParam(DEPTH_PARAM, -1.f, 1.f, 0.f, "Repeat level", "");
    configParam(DEPTH_CV_PARAM, -1.f, 1.f, 0.f, "Repeat level CV amount", "%", 0, 100, 0);
    configInput(DEPTH_CV_INPUT, "Repeat level CV");

    configParam(SHAPE_PARAM, -1.f, 1.f, 0.f, "Repeat shape", "");
    configParam(SHAPE_CV_PARAM, -1.f, 1.f, 0.f, "Repeat shape CV amount", "%", 0, 100, 0);
    configInput(SHAPE_CV_INPUT, "Repeat shape CV");

    configInput(TRIG_INPUT, "Trigger");
    configInput(MAIN_INPUT, "Main");
    configOutput(MAIN_OUTPUT, "Main");

//    oversampleStages = 5;
  }
  
  void setOversample() override {
/*
    if (oversample > 1) {
      for (int s=0; s<4; s++){
        for (int i=0; i<INPUTS_LEN; i++){
          upSample[i][s].setOversample(oversample, oversampleStages);
        }
        downSample[s].setOversample(oversample, oversampleStages);
      }
    }
*/    
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
/*
    using float_4 = simd::float_4;
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
        dcBlockInFilter[i].init(oversample, sampleRate);
        dcBlockOutFilter[i].init(oversample, sampleRate);
      }
    }
    // get channel count
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++)
      channels = std::max({channels, inputs[i].getChannels()});
    
    float_4 in[INPUTS_LEN]{}, out{}, a, b, hiThresh, loThresh, hiAmp, midAmp, loAmp, inOff, outOff;

    // channel loop
    for (int s=0, c=0; c<channels; s++, c+=4){
      // oversample loop
      for (int o=0; o<oversample; o++) {
        // read inputs
        if (!o) {
          for (int i=0; i<INPUTS_LEN; i++)
            in[i] = inputs[i].getPolyVoltageSimd<float_4>(c);
        }
        // upsample inputs
        if (oversample > 1){
          for (int i=0; i<INPUTS_LEN; i++) {
            if (inputs[i].isConnected())
              in[i] = upSample[i][s].process(o ? float_4::zero() : in[i]*oversample);
          }
        }
        // DC block input
        if (params[DC_IN_PARAM].getValue())
          in[WAVE_INPUT] = dcBlockInFilter[s].process(in[WAVE_INPUT]);
        // compute offsets
        inOff = params[IN_OFFSET_PARAM].getValue() + in[IN_OFFSET_INPUT] * params[IN_OFFSET_AMT_PARAM].getValue();
        outOff = params[OUT_OFFSET_PARAM].getValue() + in[OUT_OFFSET_INPUT] * params[OUT_OFFSET_AMT_PARAM].getValue();
        // compute window
        a = params[HI_THRESH_PARAM].getValue() + in[HI_THRESH_INPUT] * params[HI_THRESH_AMT_PARAM].getValue();
        b = params[LO_THRESH_PARAM].getValue() + in[LO_THRESH_INPUT] * params[LO_THRESH_AMT_PARAM].getValue();
        hiThresh = ifelse(a>b, a, b);
        loThresh = ifelse(a>b, b, a);
        // compute amps
        hiAmp = params[HI_AMP_PARAM].getValue() + in[HI_AMP_INPUT] * params[HI_AMP_AMT_PARAM].getValue();
        midAmp = params[MID_AMP_PARAM].getValue() + in[MID_AMP_INPUT] * params[MID_AMP_AMT_PARAM].getValue();
        loAmp = params[LO_AMP_PARAM].getValue() + in[LO_AMP_INPUT] * params[LO_AMP_AMT_PARAM].getValue();
        // offset input
        in[WAVE_INPUT] += inOff;
        // compute output middle
        switch (static_cast<int>(params[MID_CLIP_PARAM].getValue())) {
          case 0: // clamp off
            out = in[WAVE_INPUT] * midAmp;
            break;
          case 1: // clamp pre amp
            out = clamp(in[WAVE_INPUT], loThresh, hiThresh) * midAmp;
            break;
          case 2: // clamp post amp
            out = clamp(in[WAVE_INPUT] * midAmp, loThresh, hiThresh);
            break;
          default: // 3 clamp pre & post amp
            out = simd::clamp(simd::clamp(in[WAVE_INPUT], loThresh, hiThresh) * midAmp, loThresh, hiThresh);
        }
        // add high and low output
        out += ifelse(in[WAVE_INPUT]>hiThresh, (in[WAVE_INPUT]-hiThresh) * hiAmp, ifelse(in[WAVE_INPUT]<loThresh, (in[WAVE_INPUT]-loThresh) * loAmp, float_4::zero()));
        // clamp output
        switch (static_cast<int>(params[CLIP_PARAM].getValue())) {
          case 1: // hard clip 5V
            out = clamp(out, -5.f, 5.f);
            break;
          case 2: // soft clip 5V
            out = softClip(out*2.f) / 2.f;
            break;
          case 3: // soft clip 6V
            out = softClip(out*1.6667f) / 1.6667f;
            break;
        }
        // offset output
        out += outOff;
        // DC block output
        if (params[DC_OUT_PARAM].getValue())
          out = dcBlockOutFilter[s].process(out);
        // downsample output
        if (oversample > 1)
          out = downSample[s].process(out);
      } // end oversample loop
      // write output
      outputs[WAVE_OUTPUT].setVoltageSimd(out, c);
    } // end channel loop
    // set output channel count
    outputs[WAVE_OUTPUT].setChannels(channels);
*/
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

// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"

struct WaveMangler : VenomModule {

  enum ParamId {
    DC_IN_PARAM,
    OVER_PARAM,
    CLIP_PARAM,
    DC_OUT_PARAM,
    IN_OFFSET_AMT_PARAM,
    IN_OFFSET_PARAM,
    OUT_OFFSET_AMT_PARAM,
    OUT_OFFSET_PARAM,
    HI_AMP_AMT_PARAM,
    HI_AMP_PARAM,
    HI_THRESH_AMT_PARAM,
    HI_THRESH_PARAM,
    MID_AMP_AMT_PARAM,
    MID_AMP_PARAM,
    LO_THRESH_AMT_PARAM,
    LO_THRESH_PARAM,
    LO_AMP_AMT_PARAM,
    LO_AMP_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    IN_OFFSET_INPUT,
    OUT_OFFSET_INPUT,
    HI_AMP_INPUT,
    HI_THRESH_INPUT,
    MID_AMP_INPUT,
    LO_THRESH_INPUT,
    LO_AMP_INPUT,
    WAVE_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    WAVE_OUTPUT,
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

  WaveMangler() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(DC_IN_PARAM, 0.f, 1.f, 0.f, "Input DC block", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 3.f, 1.f, "Output clipping", {"Off", "Hard +/- 5V", "Soft +/- 5V", "Soft +/- 6V"});
    configSwitch<FixedSwitchQuantity>(DC_IN_PARAM, 0.f, 1.f, 0.f, "Output DC block", {"Off", "On"});

    configInput(IN_OFFSET_INPUT, "Input offset CV");
    configParam(IN_OFFSET_AMT_PARAM, -1.f, 1.f, 0.f, "Input offset CV amount", "%", 0, 100, 0);
    configParam(IN_OFFSET_PARAM, -5.f, 5.f, 0.f, "Input offset", " V");

    configInput(OUT_OFFSET_INPUT, "Output offset CV");
    configParam(OUT_OFFSET_AMT_PARAM, -1.f, 1.f, 0.f, "Output offset CV amount", "%", 0, 100, 0);
    configParam(OUT_OFFSET_PARAM, -5.f, 5.f, 0.f, "Output offset", " V");

    configInput(HI_AMP_INPUT, "High amplitude CV");
    configParam(HI_AMP_AMT_PARAM, -1.f, 1.f, 0.f, "High amplitude CV amount", "%", 0, 100, 0);
    configParam(HI_AMP_PARAM, -10.f, 10.f, 0.f, "High amplitude");

    configInput(HI_THRESH_INPUT, "High threshold CV");
    configParam(HI_THRESH_AMT_PARAM, -1.f, 1.f, 0.f, "High threshold CV amount", "%", 0, 100, 0);
    configParam(HI_THRESH_PARAM, -5.f, 5.f, 0.f, "High threshold", " V");

    configInput(MID_AMP_INPUT, "Middle amplitude CV");
    configParam(MID_AMP_AMT_PARAM, -1.f, 1.f, 0.f, "Middle amplitude CV amount", "%", 0, 100, 0);
    configParam(MID_AMP_PARAM, -10.f, 10.f, 0.f, "Middle amplitude");

    configInput(LO_THRESH_INPUT, "Low threshold CV");
    configParam(LO_THRESH_AMT_PARAM, -1.f, 1.f, 0.f, "Low threshold CV amount", "%", 0, 100, 0);
    configParam(LO_THRESH_PARAM, -5.f, 5.f, 0.f, "Low threshold", " V");

    configInput(LO_AMP_INPUT, "Low amplitude CV");
    configParam(LO_AMP_AMT_PARAM, -1.f, 1.f, 0.f, "Low amplitude CV amount", "%", 0, 100, 0);
    configParam(LO_AMP_PARAM, -10.f, 10.f, 0.f, "Low amplitude");

    configInput(WAVE_INPUT, "Wave");
    configOutput(WAVE_OUTPUT, "Wave");

    configBypass(WAVE_INPUT, WAVE_OUTPUT);
    
    oversampleStages = 5;
  }
  
  void setOversample() override {
    if (oversample > 1) {
      for (int s=0; s<4; s++){
        for (int i=0; i<INPUTS_LEN; i++){
          upSample[i][s].setOversample(oversample, oversampleStages);
        }
        downSample[s].setOversample(oversample, oversampleStages);
      }
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
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
        // compute output
        out = simd::clamp(in[WAVE_INPUT] * midAmp, loThresh, hiThresh);
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
  }

};

struct WaveManglerWidget : VenomWidget {

  struct DCSwitch : GlowingSvgSwitchLockable {
    DCSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
    }
  };

  struct ClipSwitch : GlowingSvgSwitchLockable {
    ClipSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
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

  WaveManglerWidget(WaveMangler* module) {
    setModule(module);
    setVenomPanel("WaveMangler");

    addParam(createLockableParam<DCSwitch>(Vec(11.25f, 39.636f), module, WaveMangler::DC_IN_PARAM));
    addParam(createLockableParam<OverSwitch>(Vec(36.23f, 39.636f), module, WaveMangler::OVER_PARAM));
    addParam(createLockableParam<ClipSwitch>(Vec(62.211f, 39.636f), module, WaveMangler::CLIP_PARAM));
    addParam(createLockableParam<DCSwitch>(Vec(86.191f, 39.636f), module, WaveMangler::DC_OUT_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 72.5f), module, WaveMangler::IN_OFFSET_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 72.5f), module, WaveMangler::IN_OFFSET_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 72.5f), module, WaveMangler::IN_OFFSET_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 109.5f), module, WaveMangler::OUT_OFFSET_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 109.5f), module, WaveMangler::OUT_OFFSET_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 109.5f), module, WaveMangler::OUT_OFFSET_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 148.5f), module, WaveMangler::HI_AMP_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 148.5f), module, WaveMangler::HI_AMP_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 148.5f), module, WaveMangler::HI_AMP_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 185.5f), module, WaveMangler::HI_THRESH_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 185.5f), module, WaveMangler::HI_THRESH_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 185.5f), module, WaveMangler::HI_THRESH_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 222.5f), module, WaveMangler::MID_AMP_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 222.5f), module, WaveMangler::MID_AMP_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 222.5f), module, WaveMangler::MID_AMP_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 259.5f), module, WaveMangler::LO_THRESH_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 259.5f), module, WaveMangler::LO_THRESH_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 259.5f), module, WaveMangler::LO_THRESH_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 296.5f), module, WaveMangler::LO_AMP_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 296.5f), module, WaveMangler::LO_AMP_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 296.5f), module, WaveMangler::LO_AMP_PARAM));
    
    addInput(createInputCentered<PolyPort>(Vec(32.5f, 341.5f), module, WaveMangler::WAVE_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(72.5f, 341.5f), module, WaveMangler::WAVE_OUTPUT));
  }

};

Model* modelWaveMangler = createModel<WaveMangler, WaveManglerWidget>("WaveMangler");

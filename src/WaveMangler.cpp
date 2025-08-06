// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"
//#include <float.h>

struct WaveMangler : VenomModule {

  enum ParamId {
    OVER_PARAM,
    CLIP_PARAM,
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
  int oversampleValues[6]{1,2,4,8,16,32};
/*
  OversampleFilter_4 preUpSample[4]{}, stageUpSample[4]{}, biasUpSample[4]{}, upSample[4]{}, downSample[4]{};
  float stageRaw = -1.f;
  simd::float_4 stageParm{};
  bool disableOver[3]{}, bipolar[2]{};
*/


  WaveMangler() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 2.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 2.f, 0.f, "Clipping", {"Off", "Hard +/- 5V", "Soft +/- 6V"});

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

    configInput(WAVE_INPUT, "Poly");
    configOutput(WAVE_OUTPUT, "Poly");

    configBypass(WAVE_INPUT, WAVE_OUTPUT);
    
//    oversampleStages = 5;
  }
  
/*
  void setOversample() override {
    if (oversample > 1) {
      for (int i=0; i<4; i++){
        preUpSample[i].setOversample(oversample, oversampleStages);
        stageUpSample[i].setOversample(oversample, oversampleStages);
        biasUpSample[i].setOversample(oversample, oversampleStages);
        upSample[i].setOversample(oversample, oversampleStages);
        downSample[i].setOversample(oversample, oversampleStages);
      }
    }
  }
*/        

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
 /*   
    using float_4 = simd::float_4;
    float limit = 10.f / 6.f;
    if (oversample != oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())]) {
      oversample = oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())];
      setOversample();
    }
    
    if (stageRaw != params[STAGE_PARAM].getValue()) {
      stageRaw = params[STAGE_PARAM].getValue();
      stageParm = pow(10.f, stageRaw);
    }
    float preParm = params[PRE_PARAM].getValue(),
          preAmt = params[PRE_AMT_PARAM].getValue(),
          stageAmt = params[STAGE_AMT_PARAM].getValue(),
          biasParm = params[BIAS_PARAM].getValue(),
          biasAmt = params[BIAS_AMT_PARAM].getValue();
    bool preOver = inputs[PRE_INPUT].isConnected() && !disableOver[PRE_INPUT] && oversample>1,
         stageOver = inputs[STAGE_INPUT].isConnected() && !disableOver[STAGE_INPUT] && oversample>1,
         biasOver = inputs[BIAS_INPUT].isConnected() && !disableOver[BIAS_INPUT] && oversample>1;
    
    int stages = static_cast<int>(params[STAGES_PARAM].getValue())+2;
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++)
      channels = std::max({channels, inputs[i].getChannels()});
    
    float_4 in[4]{}, out[4]{}, pre[4]{}, stage[4]{}, bias[4]{};
    for (int o=0; o<oversample; o++) {
      for (int i=0, c=0; c<channels; i++, c+=4){
        if (!o) {
          pre[i] = inputs[PRE_INPUT].getPolyVoltageSimd<float_4>(c);
          stage[i] = inputs[STAGE_INPUT].getPolyVoltageSimd<float_4>(c);
          bias[i] = inputs[BIAS_INPUT].getPolyVoltageSimd<float_4>(c);
          in[i] = inputs[POLY_INPUT].getPolyVoltageSimd<float_4>(c) * oversample;
        }
        if (oversample > 1) {
          in[i] = upSample[i].process(o ? float_4::zero() : in[i]);
        if (preOver)
            pre[i] = preUpSample[i].process(o ? float_4::zero() : pre[i]*oversample);
        if (stageOver)
          stage[i] = stageUpSample[i].process(o ? float_4::zero() : stage[i]*oversample);
        if (biasOver)
          bias[i] = biasUpSample[i].process(o ? float_4::zero() : bias[i]*oversample);
        }
        if (!o || preOver) {
          pre[i] = pre[i] * preAmt + preParm;
          if (!bipolar[PRE_INPUT])
            pre[i] = ifelse(pre[i]<0.f, 0.f, pre[i]);
        }
        if (!o || stageOver) {
          stage[i] = stage[i] * stageAmt + stageParm;
          if (!bipolar[STAGE_INPUT])
            stage[i] = ifelse(stage[i]<0.f, 0.f, stage[i]);
        }
        if (!o || biasOver)
          bias[i] = bias[i] * biasAmt + biasParm;
        out[i] = (in[i] + bias[i]) * pre[i];
        for (int s=0; s<stages; s++)
          out[i] = simd::clamp( out[i] * stage[i], -5.f, 5.f) * 2.f - out[i];
        out[i] = softClip(out[i]*limit) / limit;
        if (oversample > 1)
          out[i] = downSample[i].process(out[i]);
      }
    }
    for (int i=0, c=0; c<channels; i++, c+=4)
      outputs[POLY_OUTPUT].setVoltageSimd(out[i], c);
    outputs[POLY_OUTPUT].setChannels(channels);
*/
  }

};

struct WaveManglerWidget : VenomWidget {

  struct ClipSwitch : GlowingSvgSwitchLockable {
    ClipSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_off.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/clip_hard_pm5.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/clip_soft_pm6.svg")));
    }
  };

  struct OverSwitch : GlowingSvgSwitchLockable {
    OverSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_off.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_2.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_4.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_8.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_16.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/over_32.svg")));
    }
  };

  WaveManglerWidget(WaveMangler* module) {
    setModule(module);
    setVenomPanel("WaveMangler");

    addParam(createLockableParam<OverSwitch>(Vec(18.5f, 28.743f), module, WaveMangler::OVER_PARAM));
    addParam(createLockableParam<ClipSwitch>(Vec(62.5f, 28.743f), module, WaveMangler::CLIP_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 77.5f), module, WaveMangler::IN_OFFSET_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 77.5f), module, WaveMangler::IN_OFFSET_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 77.5f), module, WaveMangler::IN_OFFSET_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 114.5f), module, WaveMangler::OUT_OFFSET_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 114.5f), module, WaveMangler::OUT_OFFSET_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 114.5f), module, WaveMangler::OUT_OFFSET_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 154.391f), module, WaveMangler::HI_AMP_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 154.391f), module, WaveMangler::HI_AMP_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 154.391f), module, WaveMangler::HI_AMP_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 191.391f), module, WaveMangler::HI_THRESH_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 191.391f), module, WaveMangler::HI_THRESH_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 191.391f), module, WaveMangler::HI_THRESH_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 228.391f), module, WaveMangler::MID_AMP_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 228.391f), module, WaveMangler::MID_AMP_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 228.391f), module, WaveMangler::MID_AMP_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 265.391f), module, WaveMangler::LO_THRESH_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 265.391f), module, WaveMangler::LO_THRESH_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 265.391f), module, WaveMangler::LO_THRESH_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 302.391f), module, WaveMangler::LO_AMP_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 302.391f), module, WaveMangler::LO_AMP_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(84.5f, 302.391f), module, WaveMangler::LO_AMP_PARAM));
    
    addInput(createInputCentered<PolyPort>(Vec(32.5f, 345.28f), module, WaveMangler::WAVE_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(72.5f, 345.28f), module, WaveMangler::WAVE_OUTPUT));
  }

};

Model* modelWaveMangler = createModel<WaveMangler, WaveManglerWidget>("WaveMangler");

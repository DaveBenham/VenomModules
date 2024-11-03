// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"
//#include <float.h>

struct WaveFolder : VenomModule {

  enum ParamId {
    STAGES_PARAM,
    OVER_PARAM,
    PRE_PARAM,
    STAGE_PARAM,
    BIAS_PARAM,
    PRE_AMT_PARAM,
    STAGE_AMT_PARAM,
    BIAS_AMT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    PRE_INPUT,
    STAGE_INPUT,
    BIAS_INPUT,
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
  
  int oversample = 0;
  int oversampleValues[6]{1,2,4,8,16,32};
  OversampleFilter_4 upSample[4]{}, downSample[4]{};
  float stageRaw = -1.f;
  simd::float_4 stageParm{};

  WaveFolder() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(STAGES_PARAM, 0.f, 4.f, 1.f, "Stages", {"2", "3", "4", "5", "6"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 2.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});

    configParam(PRE_PARAM, 1.f, 10.f, 1.f, "Pre-amp");
    configParam(STAGE_PARAM, 0.f, 1.f, 0.f, "Stage amp", "", 10, 1, 0);
    configParam(BIAS_PARAM, -5.f, 5.f, 0.f, "Bias", "V");
    
    configParam(PRE_AMT_PARAM, -1.f, 1.f, 0.f, "Pre-amp CV amount", "%", 0, 100, 0);
    configParam(STAGE_AMT_PARAM, -1.f, 1.f, 0.f, "Stage amp CV amount", "%", 0, 100, 0);
    configParam(BIAS_AMT_PARAM, -1.f, 1.f, 0.f, "Bias CV amount", "%", 0, 100, 0);

    configInput(PRE_INPUT, "Pre-amp CV");
    configInput(STAGE_INPUT, "Stage amp CV");
    configInput(BIAS_INPUT, "Bias CV");

    configInput(POLY_INPUT, "Poly");
    configOutput(POLY_OUTPUT, "Poly");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    
    using float_4 = simd::float_4;
    float limit = 10.f / 6.f;
    if (oversample != oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())]) {
      oversample = oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())];
      if (oversample > 1) {
        for (int i=0; i<4; i++){
          upSample[i].setOversample(oversample);
          downSample[i].setOversample(oversample);
        }
      }
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
    
    int stages = static_cast<int>(params[STAGES_PARAM].getValue())+2;
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++)
      channels = std::max({channels, inputs[i].getChannels()});
    
    float_4 in[4]{}, out[4]{}, pre[4]{}, stage[4]{}, bias[4]{};
    for (int o=0; o<oversample; o++) {
      for (int i=0, c=0; c<channels; i++, c+=4){
        if (!o) {
          pre[i] = inputs[PRE_INPUT].getPolyVoltageSimd<float_4>(c) * preAmt + preParm;
          stage[i] = inputs[STAGE_INPUT].getPolyVoltageSimd<float_4>(c) * stageAmt + stageParm;
          bias[i] = inputs[BIAS_INPUT].getPolyVoltageSimd<float_4>(c) * biasAmt + biasParm;
          in[i] = inputs[POLY_INPUT].getPolyVoltageSimd<float_4>(c) * oversample;
        }
        if (oversample > 1)
          in[i] = upSample[i].process(o ? float_4::zero() : in[i]);
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
  }

};

struct WaveFolderWidget : VenomWidget {

  struct StagesSwitch : GlowingSvgSwitchLockable {
    StagesSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_2.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_3.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_4.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_5.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_6.svg")));
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

  WaveFolderWidget(WaveFolder* module) {
    setModule(module);
    setVenomPanel("WaveFolder");

    addParam(createLockableParamCentered<StagesSwitch>(Vec(24.f, 72.f), module, WaveFolder::STAGES_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(66.f, 72.f), module, WaveFolder::OVER_PARAM));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(19.f, 132.f), module, WaveFolder::PRE_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(71.f, 132.f), module, WaveFolder::STAGE_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(45.f, 166.f), module, WaveFolder::BIAS_PARAM));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(19.f, 198.f), module, WaveFolder::PRE_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(71.f, 198.f), module, WaveFolder::STAGE_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(45.f, 228.f), module, WaveFolder::BIAS_AMT_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(19.f, 257.5f), module, WaveFolder::PRE_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(71.f, 257.5f), module, WaveFolder::STAGE_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(45.f, 287.5f), module, WaveFolder::BIAS_INPUT));
    
    addInput(createInputCentered<PolyPort>(Vec(24.f, 335.5f), module, WaveFolder::POLY_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(66.f, 335.5f), module, WaveFolder::POLY_OUTPUT));
  }

};

Model* modelWaveFolder = createModel<WaveFolder, WaveFolderWidget>("WaveFolder");

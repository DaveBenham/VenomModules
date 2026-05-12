// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "Filter.hpp"
#include "math.hpp"

namespace Venom {

struct BoundedVCO : VenomModule {

  enum ParamId {
    SLOW_PARAM,
    OVER_PARAM,
    SWAP_PARAM,
    FREQ_PARAM,
    SKEW_PARAM,
    FREQ_CV_PARAM,
    SKEW_CV_PARAM,
    FLOOR_PARAM,
    CEILING_PARAM,
    FLOOR_CV_PARAM,
    CEILING_CV_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    FREQ_CV_INPUT,
    SKEW_CV_INPUT,
    FLOOR_CV_INPUT,
    CEILING_CV_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    PULSE_OUTPUT,
    TRI_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  using float_4 = simd::float_4;
  float_4 tri[4]{},
          followMin[4]{},
          oldLo[4]{},
          oldHi[4]{};
  
  bool slow = false,
       swap = false;
       
  float deltaK = 0;

  OversampleFilter_4 upSample[INPUTS_LEN][4]{},
                     downSample[OUTPUTS_LEN][4]{};
  
  int oversample = 0;
  int overVals[6]{1,2,4,8,16,32};
  float sampleTime = 0.f;

  BoundedVCO() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(SLOW_PARAM, 0.f, 1.f, 0.f, "Slow mode", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    configSwitch<FixedSwitchQuantity>(SWAP_PARAM, 0.f, 1.f, 0.f, "Auto swap bounds", {"Off", "On"});

    configParam(FREQ_PARAM, -5.f, 5.f, 0.f, "Frequency @10Vpp", " Hz", 2.f, dsp::FREQ_C4);
    configParam(FREQ_CV_PARAM, -1.f, 1.f, 0.f, "Frequency CV amount", "%", 0.f, 100.f);
    configInput(FREQ_CV_INPUT, "Frequency CV");

    configParam(SKEW_PARAM, 0.01f, 0.99f, 0.5f, "Skew", "%", 0.f, 100.f);
    configParam(SKEW_CV_PARAM, -0.1f, 0.1f, 0.f, "Skew CV amount", "%", 0.f, 1000.f);
    configInput(SKEW_CV_INPUT, "Skew CV");

    configParam(FLOOR_PARAM, -10.f, 10.f, -5.f, "Floor", " V");
    configParam(FLOOR_CV_PARAM, -1.f, 1.f, 0.f, "Floor CV amount", "%", 0.f, 100.f);
    configInput(FLOOR_CV_INPUT, "Floor CV");

    configParam(CEILING_PARAM, -10.f, 10.f, 5.f, "Ceiling", " V");
    configParam(CEILING_CV_PARAM, -1.f, 1.f, 0.f, "Ceiling CV amount", "%", 0.f, 100.f);
    configInput(CEILING_CV_INPUT, "Ceiling CV");
    
    configOutput(PULSE_OUTPUT, "Pulse");
    configOutput(TRI_OUTPUT, "Triangle");

    oversampleStages = 5;
  }
  
  void setOversample() override {
    for (int i=0; i<4; i++){
      for (int j=0; j<INPUTS_LEN; j++)
        upSample[j][i].setOversample(oversample, oversampleStages);
      for (int j=0; j<OUTPUTS_LEN; j++)
        downSample[j][i].setOversample(oversample, oversampleStages);
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    float_4 triOut{},
            pulseOut{};
    swap = static_cast<bool>(params[SWAP_PARAM].getValue());
    if (oversample != overVals[static_cast<int>(params[OVER_PARAM].getValue())]){
      oversample = overVals[static_cast<int>(params[OVER_PARAM].getValue())];
      setOversample();
      deltaK = 0.f;
    }
    if (sampleTime != args.sampleTime){
      sampleTime = args.sampleTime;
      deltaK = 0.f;
    }
    if (slow != static_cast<bool>(params[SLOW_PARAM].getValue())) {
      slow = !slow;
      deltaK = 0.f;
    }
    if (!deltaK) {
      float freqMult = slow ? 2.f : dsp::FREQ_C4;
      paramQuantities[FREQ_PARAM]->displayMultiplier = freqMult;
      deltaK = freqMult * 10.f * sampleTime / oversample;
    }
    int channels = 1;
    float_4 in[INPUTS_LEN]{};
    for (int i=0; i<INPUTS_LEN; i++)
      channels = std::max(channels, inputs[i].getChannels());
    for (int s=0, c=0; c < channels; s++, c+=4) {
      for (int i=0; i<INPUTS_LEN; i++)
        in[i] = inputs[i].getPolyVoltageSimd<float_4>(c);
      for (int o=0; o<oversample; o++) {
        if (oversample>1) {
          for (int i=0; i<INPUTS_LEN; i++) {
            if (inputs[i].isConnected()) {
              in[i] = upSample[i][s].process(o ? 0.f : in[i]*oversample);
            }
          }
        }
        float_4 lo = simd::clamp(params[FLOOR_PARAM].getValue() + in[FLOOR_CV_INPUT] * params[FLOOR_CV_PARAM].getValue(), -12.f, 12.f),
                hi = simd::clamp(params[CEILING_PARAM].getValue() + in[CEILING_CV_INPUT] * params[CEILING_CV_PARAM].getValue(), -12.f, 12.f),
                truLo = ifelse(lo<=hi, lo, hi),
                truHi = ifelse(hi>=lo, hi, lo);
        if (swap) {
          lo = truLo;
          hi = truHi;
        }

        float_4 riseRatio = clamp(params[SKEW_PARAM].getValue() + in[SKEW_CV_INPUT] * params[SKEW_CV_PARAM].getValue(), 0.01f, 0.99f),
                fallRatio = 1.f - riseRatio,
                delta = dsp::exp2_taylor5(params[FREQ_PARAM].getValue() + in[FREQ_CV_INPUT] * params[FREQ_CV_PARAM].getValue()) * deltaK;
        delta = ifelse(delta<1e-6f, 1e-6f, delta);
        float_4 riseDelta = delta / riseRatio,
                fallDelta = delta / fallRatio;
        followMin[s] = ifelse(tri[s]>=hi, 1.f, followMin[s]);
        followMin[s] = ifelse(lo>=hi, 1.f, followMin[s]);
        followMin[s] = ifelse((lo<hi)&(tri[s]<=lo), 0.f, followMin[s]);
        float_4 target = ifelse(followMin[s]>0, lo, hi),
                target2 = ifelse(followMin[s]>0, hi, lo),
                oldTarget = ifelse(followMin[s]>0, oldLo[s], oldHi[s]),
                diff = target - tri[s],
                slope = ifelse(diff>0.f, 1.f, -1.f),
                delta2 = ifelse(slope>0.f, fallDelta, riseDelta);
        diff = abs(diff);
        delta = ifelse(slope>0.f, riseDelta, fallDelta);

        int end = std::min(channels-c, 4);
        for (int i=0; i<end; i++) {
          if (delta[i] <= diff[i])
            tri[s][i] += delta[i]*slope[i];
          else if (followMin[s][i]==1.f && lo[i] >= hi[i])
              tri[s][i] = lo[i];
          else {
            float t = (oldTarget[i] - tri[s][i])/(delta[i]*slope[i] + oldTarget[i] - target[i]);
            tri[s][i] += (t*delta[i] - (1.f-t)*delta2[i])*slope[i];
            if ((followMin[s][i]==0.f && tri[s][i]<lo[i]) || (followMin[s][i] && tri[s][i]>hi[i]))
              tri[s][i] = target2[i];
            else
              followMin[s][i] = followMin[s][i] ? 0.f : 1.f;
          }
        }
        tri[s] = clamp(tri[s], -12.f, 12.f);

        followMin[s] = ifelse(tri[s]>=hi, 1.f, followMin[s]);
        followMin[s] = ifelse(lo>=hi, 1.f, followMin[s]);
        followMin[s] = ifelse((lo<hi)&(tri[s]<=lo), 0.f, followMin[s]);

        for (int i=0; i<4; i++) {
          if (std::isnan(tri[s][i])) {
            tri[s][i] = 0.f;
            followMin[s][i] = 0.f;
            slope[i] = 0.f;
          }
        }

        if (oversample>1) {
          triOut = downSample[TRI_OUTPUT][s].process(tri[s]);
          pulseOut = downSample[PULSE_OUTPUT][s].process(slope*5.f);
        }
        else {
          triOut = tri[s];
          pulseOut = slope*5.f;
        }
        oldLo[s] = lo;
        oldHi[s] = hi;
      }
      outputs[TRI_OUTPUT].setVoltageSimd(triOut, c);
      outputs[PULSE_OUTPUT].setVoltageSimd(pulseOut, c);
    }
    outputs[TRI_OUTPUT].setChannels(channels);
    outputs[PULSE_OUTPUT].setChannels(channels);
  }
  
};

struct BoundedVCOWidget : VenomWidget {

  struct OnOffSwitch : GlowingSvgSwitchLockable {
    OnOffSwitch() {
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

  BoundedVCOWidget(BoundedVCO* module) {
    setModule(module);
    setVenomPanel("BoundedVCO");

    addParam(createLockableParamCentered<OnOffSwitch>(Vec(14.5f,56.5f), module, BoundedVCO::SLOW_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(37.5f,56.5f), module, BoundedVCO::OVER_PARAM));
    addParam(createLockableParamCentered<OnOffSwitch>(Vec(59.5f,56.5f), module, BoundedVCO::SWAP_PARAM));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(19.5f, 94.f), module, BoundedVCO::FREQ_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(19.5f, 133.5f), module, BoundedVCO::FREQ_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(19.5f, 168.5f), module, BoundedVCO::FREQ_CV_INPUT));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(55.5f, 94.f), module, BoundedVCO::SKEW_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(55.5f, 133.5f), module, BoundedVCO::SKEW_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(55.5f, 168.5f), module, BoundedVCO::SKEW_CV_INPUT));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(19.5f, 214.f), module, BoundedVCO::FLOOR_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(19.5f, 253.5f), module, BoundedVCO::FLOOR_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(19.5f, 288.5f), module, BoundedVCO::FLOOR_CV_INPUT));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(55.5f, 214.f), module, BoundedVCO::CEILING_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(55.5f, 253.5f), module, BoundedVCO::CEILING_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(55.5f, 288.5f), module, BoundedVCO::CEILING_CV_INPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(19.5f, 335.5f), module, BoundedVCO::PULSE_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(55.5f, 335.5f), module, BoundedVCO::TRI_OUTPUT));
  }
  
};

}

Model* modelVenomBoundedVCO = createModel<Venom::BoundedVCO, Venom::BoundedVCOWidget>("BoundedVCO");

// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "Filter.hpp"
#include "math.hpp"

namespace Venom {

struct Octaver : VenomModule {

  enum ParamId {
    UP1_PARAM,
    DRY_PARAM,
    DOWN1_PARAM,
    DOWN2_PARAM,
    DRIVE_PARAM,
    UP1_CV_PARAM,
    DRY_CV_PARAM,
    DOWN1_CV_PARAM,
    DOWN2_CV_PARAM,
    DRIVE_CV_PARAM,
    MODE_PARAM,
    OVER_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    UP1_CV_INPUT,
    DRY_CV_INPUT,
    DOWN1_CV_INPUT,
    DOWN2_CV_INPUT,
    DRIVE_CV_INPUT,
    SIGNAL_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    SIGNAL_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  using float_4 = simd::float_4;
  float_4 inState[4]{},
          down1State[4]{-1.f,-1.f,-1.f,-1.f},
          down2State[4]{-1.f,-1.f,-1.f,-1.f},
          level[4]{},
          down1[4]{},
          down2[4]{};
  

  OversampleFilter_4 upSample[4]{},
                     downSample[4]{};
  
  DCBlockFilter_4 inDcBlock[4]{},
                  up1DcBlock[4]{};
  
  int oversample = 0;
  int overVals[3]{2,4,8};
  float sampleRate = 0.f,
        maxRise = 0.f,
        maxFall = 0.f,
        maxSqrRise = 0.f,
        maxSqrFall = 0.f;

  Octaver() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 1.f, 0.f, "Sub-octave mode", {"Inversion (Pearl)", "Square (Boss)"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 2.f, 0.f, "Oversample", {"x2", "x4", "x8"});

    configParam(UP1_PARAM, 0.f, 1.f, 0.f, "Octave +1 Mix", "%", 0.f, 100.f);
    configParam(UP1_CV_PARAM, -0.1f, 0.1f, 0.f, "Octave +1 Mix CV", "%", 0.f, 1000.f);
    configInput(UP1_CV_INPUT, "Octave +1 Mix CV");

    configParam(DRY_PARAM, 0.f, 1.f, 0.f, "Dry Mix", "%", 0.f, 100.f);
    configParam(DRY_CV_PARAM, -0.1f, 0.1f, 0.f, "Dry Mix CV", "%", 0.f, 1000.f);
    configInput(DRY_CV_INPUT, "Dry Mix CV");

    configParam(DOWN1_PARAM, 0.f, 1.f, 0.f, "Octave -1 Mix", "%", 0.f, 100.f);
    configParam(DOWN1_CV_PARAM, -0.1f, 0.1f, 0.f, "Octave -1 Mix CV", "%", 0.f, 1000.f);
    configInput(DOWN1_CV_INPUT, "Octave -1 Mix CV");

    configParam(DOWN2_PARAM, 0.f, 1.f, 0.f, "Octave -2 Mix", "%", 0.f, 100.f);
    configParam(DOWN2_CV_PARAM, -0.1f, 0.1f, 0.f, "Octave -2 Mix CV", "%", 0.f, 1000.f);
    configInput(DOWN2_CV_INPUT, "Octave -2 Mix CV");
    
    configParam(DRIVE_PARAM, 0.f, 5.f, 1.f, "Drive", "");
    configParam(DRIVE_CV_PARAM, -1.f, 1.f, 0.f, "Drive CV", "%", 0.f, 100.f);
    configInput(DRIVE_CV_INPUT, "Drive CV");
    
    configInput(SIGNAL_INPUT, "Signal");
    configOutput(SIGNAL_OUTPUT, "Signal");

    configBypass(SIGNAL_INPUT, SIGNAL_OUTPUT);

  }
  
  void setOversample() override {
    for (int i=0; i<4; i++){
      upSample[i].setOversample(oversample, 5);
      downSample[i].setOversample(oversample, 5);
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (oversample != overVals[static_cast<int>(params[OVER_PARAM].getValue())]){
      oversample = overVals[static_cast<int>(params[OVER_PARAM].getValue())];
      setOversample();
      sampleRate = 0.f;
    }
    if (sampleRate != args.sampleRate){
      sampleRate = args.sampleRate;
      for (int i=0; i<4; i++) {
        inDcBlock[i].init(oversample, sampleRate);
        up1DcBlock[i].init(oversample, sampleRate);
      }
      maxRise = 1280.f / sampleRate / oversample;
      maxFall = -40.f / sampleRate / oversample;
      maxSqrRise = 10000.f / sampleRate / oversample;
      maxSqrFall = -maxSqrRise;
    }
    int mode = params[MODE_PARAM].getValue(),
        channels = inputs[SIGNAL_INPUT].getChannels();
    for (int s=0, c=0; c < channels; s++, c+=4) {
      float_4 in = inputs[SIGNAL_INPUT].getPolyVoltageSimd<float_4>(c) * oversample,
              out{},
              inAmt = clamp(params[DRY_PARAM].getValue() + params[DRY_CV_PARAM].getValue() * inputs[DRY_CV_INPUT].getPolyVoltageSimd<float_4>(c)),
              up1Amt = clamp(params[UP1_PARAM].getValue() + params[UP1_CV_PARAM].getValue() * inputs[UP1_CV_INPUT].getPolyVoltageSimd<float_4>(c)),
              dn1Amt = clamp(params[DOWN1_PARAM].getValue() + params[DOWN1_CV_PARAM].getValue() * inputs[DOWN1_CV_INPUT].getPolyVoltageSimd<float_4>(c)),
              dn2Amt = clamp(params[DOWN2_PARAM].getValue() + params[DOWN2_CV_PARAM].getValue() * inputs[DOWN2_CV_INPUT].getPolyVoltageSimd<float_4>(c)),
              drive = clamp(params[DRIVE_PARAM].getValue() + params[DRIVE_CV_PARAM].getValue() * inputs[DRIVE_CV_INPUT].getPolyVoltageSimd<float_4>(c), 0.f, 10.f);
      for (int o=0; o<oversample; o++) {
        in = upSample[s].process(o ? 0.f : in);
        in = inDcBlock[s].process(in);
        float_4 up1 = abs(in) * 2.f,
                dn1,
                dn2;
        if (mode) {
          float_4 diff = up1 - level[s];
          diff = ifelse( diff > maxRise, maxRise, diff);
          diff = ifelse (diff < maxFall, maxFall, diff);
          level[s] += diff;
        }
        up1 = up1DcBlock[s].process(up1);
        float_4 newInState = ifelse( in>0.f, 1.f, 0.f),
                newDown1State = ifelse((newInState>0.f) & (newInState!=inState[s]), down1State[s]*-1.f, down1State[s]);
        down2State[s] = ifelse((newDown1State>0.f) & (newDown1State!=down1State[s]), down2State[s]*-1.f, down2State[s]);
        down1State[s] = newDown1State;
        inState[s] = newInState;
        if (mode) {
          dn1 = down1State[s] * level[s] * 0.5;
          float_4 diff = dn1 - down1[s];
          diff = ifelse( diff > maxSqrRise, maxSqrRise, diff);
          diff = ifelse( diff < maxSqrFall, maxSqrFall, diff);
          down1[s] += diff;
          dn1 = down1[s];
          dn2 = down2State[s] * level[s] * 0.5f;
          diff = dn2 - down2[s];
          diff = ifelse( diff > maxSqrRise, maxSqrRise, diff);
          diff = ifelse( diff < maxSqrFall, maxSqrFall, diff);
          down2[s] += diff;
          dn2 = down2[s];
        }
        else {
          dn1 = in * down1State[s];
          dn2 = (dn1 + dn1 * down2State[s]) * 0.5f;
        }
        out = in * inAmt
            + up1 * up1Amt
            + dn1 * dn1Amt
            + dn2 * dn2Amt;
        out = softClip(out*10.f/6.f * drive) * 6.f/10.f;
        out = downSample[s].process(out);
      }
      outputs[SIGNAL_OUTPUT].setVoltageSimd(out, c);
    }
    outputs[SIGNAL_OUTPUT].setChannels(channels);
  }
  
};

struct OctaverWidget : VenomWidget {

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
    }
  };

  struct OverSwitch : GlowingSvgSwitchLockable {
    OverSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
    }
  };

  OctaverWidget(Octaver* module) {
    setModule(module);
    setVenomPanel("Octaver");

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(19.5f, 64.f), module, Octaver::UP1_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(19.5f, 99.5f), module, Octaver::UP1_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(19.5f, 130.5f), module, Octaver::UP1_CV_INPUT));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(55.5f, 64.f), module, Octaver::DRY_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(55.5f, 99.5f), module, Octaver::DRY_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(55.5f, 130.5f), module, Octaver::DRY_CV_INPUT));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(19.5f, 172.f), module, Octaver::DOWN1_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(19.5f, 207.5f), module, Octaver::DOWN1_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(19.5f, 238.5f), module, Octaver::DOWN1_CV_INPUT));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(55.5f, 172.f), module, Octaver::DOWN2_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(55.5f, 207.5f), module, Octaver::DOWN2_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(55.5f, 238.5f), module, Octaver::DOWN2_CV_INPUT));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(19.5f, 282.f), module, Octaver::DRIVE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(55.5f, 267.5f), module, Octaver::DRIVE_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(55.5f, 300.5f), module, Octaver::DRIVE_CV_INPUT));
    
    addParam(createLockableParamCentered<ModeSwitch>(Vec(29.f,30.f), module, Octaver::MODE_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(46.f,30.f), module, Octaver::OVER_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(19.5f, 341.5f), module, Octaver::SIGNAL_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(55.5f, 341.5f), module, Octaver::SIGNAL_OUTPUT));
  }
  
};

}

Model* modelVenomOctaver = createModel<Venom::Octaver, Venom::OctaverWidget>("Octaver");

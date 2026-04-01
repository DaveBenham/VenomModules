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
    TONE_PARAM,
    DRIVE_PARAM,
    UP1_CV_PARAM,
    DRY_CV_PARAM,
    DOWN1_CV_PARAM,
    DOWN2_CV_PARAM,
    TONE_CV_PARAM,
    DRIVE_CV_PARAM,
    OVER_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    UP1_CV_INPUT,
    DRY_CV_INPUT,
    DOWN1_CV_INPUT,
    DOWN2_CV_INPUT,
    TONE_CV_INPUT,
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
          down1State[4]{-50.f,-50.f,-50.f,-50.f},
          down2State[4]{-50.f,-50.f,-50.f,-50.f},
          level[4]{},
          freqOct{1.f,0.f,-1.f,-2.f};
  

  OversampleFilter_4 upSample[4]{},
                     downSample[4]{};
  
  DCBlockFilter_4 inDcBlock[4]{},
                  up1DcBlock[4]{};
  
  float_4 vcf[3][4][4]{}; // [octave][mode][simIndex]

  #define UP1 0
  #define DN1 1
  #define DN2 2

  #define LOW   0
  #define HIGH  1
  #define BAND  2
  #define NOTCH 3
  
  const float q = 1.f / pow(2.f, 0.9f);

  int oversample = 0;
  int overVals[3]{2,4,8};
  float sampleRate = 0.f,
        maxFreq = 0.f,
        maxRise = 0.f,
        maxFall = 0.f,
        sampleTimePi = 0.f;

  Octaver() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam(UP1_PARAM, 0.f, 1.f, 0.f, "Octave +1 Mix", "%", 0.f, 100.f);
    configParam(UP1_CV_PARAM, -0.1f, 0.1f, 0.f, "Octave +1 CV", "%", 0.f, 1000.f);
    configInput(UP1_CV_INPUT, "Octave +1 CV");

    configParam(DRY_PARAM, 0.f, 1.f, 0.f, "Dry Mix", "%", 0.f, 100.f);
    configParam(DRY_CV_PARAM, -0.1f, 0.1f, 0.f, "Dry CV", "%", 0.f, 1000.f);
    configInput(DRY_CV_INPUT, "Dry CV");

    configParam(DOWN1_PARAM, 0.f, 1.f, 0.f, "Octave -1 Mix", "%", 0.f, 100.f);
    configParam(DOWN1_CV_PARAM, -0.1f, 0.1f, 0.f, "Octave -1 CV", "%", 0.f, 1000.f);
    configInput(DOWN1_CV_INPUT, "Octave -1 CV");

    configParam(DOWN2_PARAM, 0.f, 1.f, 0.f, "Octave -2 Mix", "%", 0.f, 100.f);
    configParam(DOWN2_CV_PARAM, -0.1f, 0.1f, 0.f, "Octave -2 CV", "%", 0.f, 1000.f);
    configInput(DOWN2_CV_INPUT, "Octave -2 CV");
    
    configParam(TONE_PARAM, -3.f, 5.f, 0.f, "Tone", " Hz", 2.f, dsp::FREQ_C4);
    configParam(TONE_CV_PARAM, -1.f, 1.f, 0., "Tone CV", "%", 0.f, 100.f);
    configInput(TONE_CV_INPUT, "Tone CV");

    configParam(DRIVE_PARAM, 0.f, 5.f, 1.f, "Drive", "");
    configParam(DRIVE_CV_PARAM, -1.f, 1.f, 0.f, "Drive CV", "%", 0.f, 100.f);
    configInput(DRIVE_CV_INPUT, "Drive CV");
    
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 2.f, 1.f, "Oversample", {"x2", "x4", "x8"});

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
      maxRise = 40.f / sampleRate / oversample;
      maxFall = -1280.f / sampleRate / oversample;
      maxFreq = sampleRate / 3.f;
      sampleTimePi = M_PI * args.sampleTime / oversample;
    }
    
    // get channel count
    int channels=1;
    for (int i=0; i<INPUTS_LEN; i++){
      if(inputs[i].getChannels() > channels)
        channels = inputs[i].getChannels();
    }
    
    for (int s=0, c=0; c < channels; s++, c+=4) {
      float_4 in = inputs[SIGNAL_INPUT].getPolyVoltage(c) * oversample,
              out{},
              inAmt = clamp(params[DRY_PARAM].getValue() + params[DRY_CV_PARAM].getValue() * inputs[DRY_CV_INPUT].getPolyVoltageSimd<float_4>(c)),
              drive = clamp(params[DRIVE_PARAM].getValue() + params[DRIVE_CV_PARAM].getValue() * inputs[DRIVE_CV_INPUT].getPolyVoltageSimd<float_4>(c), 0.f, 10.f);
      float_4 amt[3]{};
      amt[UP1] = clamp(params[UP1_PARAM].getValue() + params[UP1_CV_PARAM].getValue() * inputs[UP1_CV_INPUT].getPolyVoltageSimd<float_4>(c))/10.f;
      amt[DN1] = clamp(params[DOWN1_PARAM].getValue() + params[DOWN1_CV_PARAM].getValue() * inputs[DOWN1_CV_INPUT].getPolyVoltageSimd<float_4>(c))/10.f;
      amt[DN2] = clamp(params[DOWN2_PARAM].getValue() + params[DOWN2_CV_PARAM].getValue() * inputs[DOWN2_CV_INPUT].getPolyVoltageSimd<float_4>(c))/10.f;

      float_4 freq[3]{};
      freq[UP1] = params[TONE_PARAM].getValue() + params[TONE_CV_PARAM].getValue() * inputs[TONE_CV_INPUT].getPolyVoltageSimd<float_4>(c);
      freq[DN1] = pow(2.f, freq[UP1]-1.f) * dsp::FREQ_C4;
      freq[DN2] = pow(2.f, freq[UP1]-2.f) * dsp::FREQ_C4;
      freq[UP1] = pow(2.f, freq[UP1]+1.f) * dsp::FREQ_C4;
      for (int i=0; i<3; i++){
        freq[i] = ifelse(freq[i]>maxFreq, maxFreq, freq[i]);
        freq[i] = 2.f * sin(sampleTimePi * freq[i]);
      }
      
      for (int o=0; o<oversample; o++) {
        in = upSample[s].process(o ? 0.f : in);
        in = inDcBlock[s].process(in);
        float_4 oct[3]{};
        oct[UP1] = abs(in) * 2.f;
        float_4 diff = oct[UP1] - level[s];
        diff = ifelse( diff > maxRise, maxRise, diff);
        diff = ifelse (diff < maxFall, maxFall, diff);
        level[s] += diff;
        oct[UP1] = up1DcBlock[s].process(oct[UP1]) * 10.f;
        float_4 newInState = ifelse( in>0.f, 1.f, 0.f),
                newDown1State = ifelse((newInState>0.f) & (newInState!=inState[s]), down1State[s]*-1.f, down1State[s]);
        down2State[s] = ifelse((newDown1State>0.f) & (newDown1State!=down1State[s]), down2State[s]*-1.f, down2State[s]);
        down1State[s] = newDown1State;
        inState[s] = newInState;
        oct[DN1] = down1State[s] * level[s];
        oct[DN2] = down2State[s] * level[s];
        out = in * inAmt;

        for (int i=0; i<3; i++) {
          vcf[i][NOTCH][s] = q * vcf[i][BAND][s] - oct[i];
          vcf[i][HIGH][s] = -(vcf[i][NOTCH][s] + vcf[i][LOW][s]);
          vcf[i][BAND][s] = vcf[i][BAND][s] + freq[i] * vcf[i][HIGH][s];
          vcf[i][LOW][s] = vcf[i][LOW][s] + freq[i] * vcf[i][BAND][s];
          out = out + (vcf[i][LOW][s]*0.7f + vcf[i][BAND][s]*0.3f) * amt[i];
        }
        out = softClip(out*10.f/6.f * drive) * 6.f/10.f;
        out = downSample[s].process(out);
      }
      outputs[SIGNAL_OUTPUT].setVoltageSimd(out, c);
    }
    outputs[SIGNAL_OUTPUT].setChannels(channels);
  }
  
};

struct OctaverWidget : VenomWidget {

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

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(19.5f, 45.f), module, Octaver::UP1_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(19.5f, 73.5f), module, Octaver::UP1_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(19.5f, 102.5f), module, Octaver::UP1_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(55.5f, 45.f), module, Octaver::DRY_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(55.5f, 73.5f), module, Octaver::DRY_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(55.5f, 102.5f), module, Octaver::DRY_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(19.5f, 143.f), module, Octaver::DOWN1_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(19.5f, 171.5f), module, Octaver::DOWN1_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(19.5f, 200.5f), module, Octaver::DOWN1_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(55.5f, 143.f), module, Octaver::DOWN2_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(55.5f, 171.5f), module, Octaver::DOWN2_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(55.5f, 200.5f), module, Octaver::DOWN2_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(19.5f, 241.f), module, Octaver::TONE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(19.5f, 269.5f), module, Octaver::TONE_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(19.5f, 298.5f), module, Octaver::TONE_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(55.5f, 241.f), module, Octaver::DRIVE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(55.5f, 269.5f), module, Octaver::DRIVE_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(55.5f, 298.5f), module, Octaver::DRIVE_CV_INPUT));
    
    addParam(createLockableParamCentered<OverSwitch>(Vec(37.5f,262.f), module, Octaver::OVER_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(19.5f, 341.5f), module, Octaver::SIGNAL_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(55.5f, 341.5f), module, Octaver::SIGNAL_OUTPUT));
  }
  
};

}

Model* modelVenomOctaver = createModel<Venom::Octaver, Venom::OctaverWidget>("Octaver");

// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"

struct SVF : VenomModule {
 
  enum ParamId {
    FREQ_PARAM,
    FREQ_CV_PARAM,
    RES_PARAM,
    RES_CV_PARAM,
    DRIVE_PARAM,
    DRIVE_CV_PARAM,
    RANGE_PARAM,
    SLOPE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    VOCT_INPUT,
    FREQ_CV_INPUT,
    RES_CV_INPUT,
    DRIVE_CV_INPUT,
    L_INPUT,
    R_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    L_LOW_OUTPUT,
    R_LOW_OUTPUT,
    L_HIGH_OUTPUT,
    R_HIGH_OUTPUT,
    L_BAND_OUTPUT,
    R_BAND_OUTPUT,
    L_NOTCH_OUTPUT,
    R_NOTCH_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  using float_4 = simd::float_4;
  float maxFreqLo,
        maxFreqHi;
  float sampleRate;
  float rangeFreq[2] {dsp::FREQ_C4, 2.f};
  int oversample = 2,
      range = 0;
  int rangeOver[2] {2,1};
  float_4 state[4][8]{}, 
          modeState[4][7][4][8]{};
  OversampleFilter_4 stereoUpSample[8]{},
                     lowDownSample[8]{},
                     bandDownSample[8]{},
                     highDownSample[8]{},
                     notchDownSample[8]{};
  #define INPUT 4  

  #define LOW 0
  #define HIGH 1
  #define BAND 2
  #define NOTCH 3
  
  SVF() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam(FREQ_PARAM, -4.f, 6.f, 0.f, "Cutoff frequency", " Hz", 2.f, rangeFreq[range], 0.f);
    configInput(VOCT_INPUT, "V/Oct");
    configParam(FREQ_CV_PARAM, -1.f, 1.f, 0.f, "Cutoff CV amount", "%", 0.f, 100.f, 0.f);
    configInput(FREQ_CV_INPUT, "Cutoff CV");

    configParam(RES_PARAM, 0.f, 1.f, 0.f, "Resonance");
    configParam(RES_CV_PARAM, -1.f, 1.f, 0.f, "Resonance CV amount", "%", 0.f, 100.f, 0.f);
    configInput(RES_CV_INPUT, "Resonance CV");

    configParam(DRIVE_PARAM, 0.f, 2.f, 1.f, "Drive");
    configParam(DRIVE_CV_PARAM, -1.f, 1.f, 0.f, "Drive CV amount", "%", 0.f, 100.f, 0.f);
    configInput(DRIVE_CV_INPUT, "Drive CV");

    configInput(L_INPUT, "Left");
    configInput(R_INPUT, "Right");
    configSwitch<FixedSwitchQuantity>(RANGE_PARAM, 0.f, 1.f, 0.f, "Frequency range", {"Audio rate", "Low frequency"});
    configSwitch<FixedSwitchQuantity>(SLOPE_PARAM, 0.f, 7.f, 0.f, "Slope", {"12dB", "24dB", "36dB", "48dB", "60dB", "72dB", "84dB", "96dB"});

    configOutput(L_LOW_OUTPUT, "Left low pass");
    configOutput(R_LOW_OUTPUT, "Right low pass");
    configOutput(L_HIGH_OUTPUT, "Left high pass");
    configOutput(R_HIGH_OUTPUT, "Right high pass");
   
    configOutput(L_BAND_OUTPUT, "Left band pass");
    configOutput(R_BAND_OUTPUT, "Right band pass");
    configOutput(L_NOTCH_OUTPUT, "Left notch pass");
    configOutput(R_NOTCH_OUTPUT, "Right notch pass");

    setOversample();
  }

  void setOversample() override {
    for (int i=0; i<8; i++){
      stereoUpSample[i].setOversample(oversample, 5);
      lowDownSample[i].setOversample(oversample, 5);
      bandDownSample[i].setOversample(oversample, 5);
      highDownSample[i].setOversample(oversample, 5);
      notchDownSample[i].setOversample(oversample, 5);
    }
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    
    // update range
    if (range != static_cast<int>(params[RANGE_PARAM].getValue())) {
      range = static_cast<int>(params[RANGE_PARAM].getValue());
      oversample = rangeOver[range];
      paramQuantities[FREQ_PARAM]->displayMultiplier = rangeFreq[range];
      sampleRate = 0.f;
    }
    // update sampleRate
    if (sampleRate != args.sampleRate){
      sampleRate = args.sampleRate;
      maxFreqHi = sampleRate<20000 ? 4 : sampleRate<40000 ? 5 : sampleRate<80000 ? 6 : sampleRate<170000 ? 7 : 8;
      maxFreqLo = maxFreqHi + 6;
    }
    float maxFreq = range ? maxFreqLo : maxFreqHi;

    float freqParam = params[FREQ_PARAM].getValue(),
          resParam = params[RES_PARAM].getValue(),
          driveParam = params[DRIVE_PARAM].getValue(),
          freqCVAmt = params[FREQ_CV_PARAM].getValue(),
          resCVAmt = params[RES_CV_PARAM].getValue(),
          driveCVAmt = params[DRIVE_CV_PARAM].getValue(),
          sampleTimePiK = M_PI * args.sampleTime * rangeFreq[range] / oversample;

    float_4 voctIn{},
            freqIn{},
            resIn{},
            driveIn{},
            stereoIn{},
            voct{},
            freq{},
            res{},
            drive{},
            stereo{},
            f{},
            q{},
            low{},
            band{},
            high{},
            notch{};

    int slope = params[SLOPE_PARAM].getValue();
    
    bool stereoConnected = inputs[L_INPUT].isConnected() || inputs[R_INPUT].isConnected(),
         outConnected[4]{ outputs[L_LOW_OUTPUT].isConnected() || outputs[R_LOW_OUTPUT].isConnected(),
                          outputs[L_HIGH_OUTPUT].isConnected() || outputs[R_HIGH_OUTPUT].isConnected(),
                          outputs[L_BAND_OUTPUT].isConnected() || outputs[R_BAND_OUTPUT].isConnected(),
                          outputs[L_NOTCH_OUTPUT].isConnected() || outputs[R_NOTCH_OUTPUT].isConnected() };


    // get channel count
    int channels=1;
    for (int i=0; i<INPUTS_LEN; i++){
      if(inputs[i].getChannels() > channels)
        channels = inputs[i].getChannels();
    }

    for (int s=0, c1=0, c2=1; c1<channels; s++, c1+=2, c2+=2) {
      voctIn[0] = inputs[VOCT_INPUT].getVoltage(c1);
      voctIn[1] = voctIn[0];
      voctIn[2] = inputs[VOCT_INPUT].getVoltage(c2);
      voctIn[3] = voctIn[2];
      freqIn[0] = inputs[FREQ_CV_INPUT].getVoltage(c1);
      freqIn[1] = freqIn[0];
      freqIn[2] = inputs[FREQ_CV_INPUT].getVoltage(c2);
      freqIn[3] = freqIn[2];
      resIn[0] = inputs[RES_CV_INPUT].getVoltage(c1);
      resIn[1] = resIn[0];
      resIn[2] = inputs[RES_CV_INPUT].getVoltage(c2);
      resIn[3] = resIn[2];
      driveIn[0] = inputs[DRIVE_CV_INPUT].getVoltage(c1);
      driveIn[1] = driveIn[0];
      driveIn[2] = inputs[DRIVE_CV_INPUT].getVoltage(c2);
      driveIn[3] = driveIn[2];
      stereoIn[0] = inputs[L_INPUT].getVoltage(c1);
      stereoIn[1] = inputs[R_INPUT].getNormalVoltage(stereoIn[0], c1);
      stereoIn[2] = inputs[L_INPUT].getVoltage(c2);
      stereoIn[3] = inputs[R_INPUT].getNormalVoltage(stereoIn[2], c2);
      freq = freqParam + voctIn + freqIn * freqCVAmt;
      freq = ifelse(freq>maxFreq, maxFreq, freq);
      res = clamp(resParam + resIn/10.f * resCVAmt) * 9.f;
      drive = clamp(driveParam + driveIn/5.f * driveCVAmt, 0.f, 2.f);
      f = 2.f * sin(sampleTimePiK * pow(2.f, freq));
      q = (slope==0) ? 1.f / pow(2.f, res) : 1.f;
      for (int o=0; o<oversample; o++){
        if (oversample>1 && stereoConnected) {
          stereoIn = stereoUpSample[s].process(o ? 0.f : stereoIn*oversample);
        }
        stereo = stereoIn * drive + 1e-6f * (2.f * random::uniform() - 1.f);
        notch = state[NOTCH][s] = q * state[BAND][s] - stereo;
        high = state[HIGH][s] = -(state[NOTCH][s] + state[LOW][s]);
        band = state[BAND][s] = state[BAND][s] + f * state[HIGH][s];
        low = state[LOW][s] = state[LOW][s] + f * state[BAND][s];
        for (int i=0; i<slope; i++){
          if (i==slope-1)
            q = 1.f / pow(2.f, res);
          int b=LOW;
          if (outConnected[b]){
            stereo = low;
            modeState[b][i][NOTCH][s] = q * modeState[b][i][BAND][s] - stereo;
            modeState[b][i][HIGH][s] = -(modeState[b][i][NOTCH][s] + modeState[b][i][LOW][s]);
            modeState[b][i][BAND][s] = modeState[b][i][BAND][s] + f * modeState[b][i][HIGH][s];
            low = modeState[b][i][LOW][s] = modeState[b][i][LOW][s] + f * modeState[b][i][BAND][s];
          }
          b=HIGH;
          if (outConnected[b]){
            stereo = high;
            modeState[b][i][NOTCH][s] = q * modeState[b][i][BAND][s] - stereo;
            high = modeState[b][i][HIGH][s] = -(modeState[b][i][NOTCH][s] + modeState[b][i][LOW][s]);
            modeState[b][i][BAND][s] = modeState[b][i][BAND][s] + f * modeState[b][i][HIGH][s];
            modeState[b][i][LOW][s] = modeState[b][i][LOW][s] + f * modeState[b][i][BAND][s];
          }
          b=BAND;
          if (outConnected[b]){
            stereo = band;
            modeState[b][i][NOTCH][s] = q * modeState[b][i][BAND][s] - stereo;
            modeState[b][i][HIGH][s] = -(modeState[b][i][NOTCH][s] + modeState[b][i][LOW][s]);
            band = modeState[b][i][BAND][s] = modeState[b][i][BAND][s] + f * modeState[b][i][HIGH][s];
            modeState[b][i][LOW][s] = modeState[b][i][LOW][s] + f * modeState[b][i][BAND][s];
          }
          b=NOTCH;
          if (outConnected[b]){
            stereo = notch;
            notch = modeState[b][i][NOTCH][s] = q * modeState[b][i][BAND][s] - stereo;
            modeState[b][i][HIGH][s] = -(modeState[b][i][NOTCH][s] + modeState[b][i][LOW][s]);
            modeState[b][i][BAND][s] = modeState[b][i][BAND][s] + f * modeState[b][i][HIGH][s];
            modeState[b][i][LOW][s] = modeState[b][i][LOW][s] + f * modeState[b][i][BAND][s];
          }
        }
        low = softClip(low);
        high = softClip(high);
        band = softClip(band);
        notch = softClip(notch);
        if (oversample>1) {
          if (outConnected[LOW])
            low = lowDownSample[s].process(low);
          if (outConnected[BAND])
            band = bandDownSample[s].process(band);
          if (outConnected[HIGH])
            high = highDownSample[s].process(high);
          if (outConnected[NOTCH])
            notch = notchDownSample[s].process(notch);
        }
      }
      outputs[L_LOW_OUTPUT].setVoltage(low[0], c1);
      outputs[R_LOW_OUTPUT].setVoltage(low[1], c1);
      outputs[L_LOW_OUTPUT].setVoltage(low[2], c2);
      outputs[R_LOW_OUTPUT].setVoltage(low[3], c2);

      outputs[L_HIGH_OUTPUT].setVoltage(high[0], c1);
      outputs[R_HIGH_OUTPUT].setVoltage(high[1], c1);
      outputs[L_HIGH_OUTPUT].setVoltage(high[2], c2);
      outputs[R_HIGH_OUTPUT].setVoltage(high[3], c2);

      outputs[L_BAND_OUTPUT].setVoltage(band[0], c1);
      outputs[R_BAND_OUTPUT].setVoltage(band[1], c1);
      outputs[L_BAND_OUTPUT].setVoltage(band[2], c2);
      outputs[R_BAND_OUTPUT].setVoltage(band[3], c2);

      outputs[L_NOTCH_OUTPUT].setVoltage(notch[0], c1);
      outputs[R_NOTCH_OUTPUT].setVoltage(notch[1], c1);
      outputs[L_NOTCH_OUTPUT].setVoltage(notch[2], c2);
      outputs[R_NOTCH_OUTPUT].setVoltage(notch[3], c2);
     
    }
    outputs[L_LOW_OUTPUT].setChannels(channels);
    outputs[L_HIGH_OUTPUT].setChannels(channels);
    outputs[L_BAND_OUTPUT].setChannels(channels);
    outputs[L_NOTCH_OUTPUT].setChannels(channels);
    outputs[R_LOW_OUTPUT].setChannels(channels);
    outputs[R_HIGH_OUTPUT].setChannels(channels);
    outputs[R_BAND_OUTPUT].setChannels(channels);
    outputs[R_NOTCH_OUTPUT].setChannels(channels);
  }
        
};

struct SVFWidget : VenomWidget {
  
  struct RangeSwitch : GlowingSvgSwitchLockable {
    RangeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/range_Audio.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/range_LowFreq.svg")));
    }
  };

  struct SlopeSwitch : GlowingSvgSwitchLockable {
    SlopeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/slope_12dB.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/slope_24dB.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/slope_36dB.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/slope_48dB.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/slope_60dB.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/slope_72dB.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/slope_84dB.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/slope_96dB.svg")));
    }
  };

  SVFWidget(SVF* module) {
    setModule(module);
    setVenomPanel("SVF");
    
    addParam(createLockableParamCentered<RoundBigBlackKnobLockable>(Vec(35.5f,77.5f), module, SVF::FREQ_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(81.5f,92.5f), module, SVF::FREQ_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(115.5f, 62.5f), module, SVF::VOCT_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(115.5f, 92.5f), module, SVF::FREQ_CV_INPUT));

    addParam(createLockableParamCentered<RoundLargeBlackKnobLockable>(Vec(35.5f,141.5f), module, SVF::RES_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(81.5f,141.5f), module, SVF::RES_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(115.5f, 141.5f), module, SVF::RES_CV_INPUT));

    addParam(createLockableParamCentered<RoundLargeBlackKnobLockable>(Vec(35.5f,201.f), module, SVF::DRIVE_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(81.5f,201.f), module, SVF::DRIVE_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(115.5f, 201.f), module, SVF::DRIVE_CV_INPUT));

    addInput(createInputCentered<PolyPort>(Vec(19.5f, 253.f), module, SVF::L_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(49.5f, 253.f), module, SVF::R_INPUT));
    addParam(createLockableParamCentered<RangeSwitch>(Vec(81.5f,253.f), module, SVF::RANGE_PARAM));
    addParam(createLockableParamCentered<SlopeSwitch>(Vec(115.f,253.f), module, SVF::SLOPE_PARAM));

    addOutput(createOutputCentered<PolyPort>(Vec(19.5f,295.5f), module, SVF::L_LOW_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(49.5f,295.5f), module, SVF::R_LOW_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(85.5f,295.5f), module, SVF::L_HIGH_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(115.5f,295.5f), module, SVF::R_HIGH_OUTPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(19.5f,339.5f), module, SVF::L_BAND_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(49.5f,339.5f), module, SVF::R_BAND_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(85.5f,339.5f), module, SVF::L_NOTCH_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(115.5f,339.5f), module, SVF::R_NOTCH_OUTPUT));
  }

};

Model* modelSVF = createModel<SVF, SVFWidget>("SVF");

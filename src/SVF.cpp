// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"

struct SVF : VenomModule {
 
  enum ParamId {
    FREQ_PARAM,
    SLOPE_PARAM,
    RANGE_PARAM,
    FREQ_CV_PARAM,
    RES_PARAM,
    RES_CV_PARAM,
    DRIVE_PARAM,
    DRIVE_CV_PARAM,
    SPREAD_PARAM,
    SPREAD_DIR_PARAM,
    SPREAD_MONO_PARAM,
    SPREAD_CV_PARAM,
    FDBK_PARAM,
    FDBK_CV_PARAM,
    MORPH_PARAM,
    MORPH_MODE_PARAM,
    MORPH_CV_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    VOCT_INPUT,
    FREQ_CV_INPUT,
    RES_CV_INPUT,
    DRIVE_CV_INPUT,
    SPREAD_CV_INPUT,
    FDBK_CV_INPUT,
    MORPH_CV_INPUT,
    L_INPUT,
    R_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    L_MORPH_OUTPUT,
    R_MORPH_OUTPUT,
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
  float maxFreq;
  float sampleRate;
  float rangeFreq[2] {dsp::FREQ_C4, 2.f};
  int oversample = 2,
      range = 0;
  int rangeOver[2] {2,1};
  bool disableDCBlock = false;
  float_4 state[4][8]{}, 
          modeState[4][7][4][8]{},
          fdbkOld[8]{};
  OversampleFilter_4 stereoUpSample[8]{},
                     lowDownSample[8]{},
                     morphDownSample[8]{},
                     bandDownSample[8]{},
                     highDownSample[8]{},
                     notchDownSample[8]{};

  DCBlockFilter_4 dcBlockFilter[5][8]{};

  #define LOW 0
  #define HIGH 1
  #define BAND 2
  #define NOTCH 3
  #define MORPH 4

  struct FreqQuantity:ParamQuantity {
    float maxFreq = 0;
    float getDisplayValue() override {
      float rtn = ParamQuantity::getDisplayValue();
      return rtn>maxFreq ? maxFreq : rtn;
    }
  };
  
  struct FdbkQuantity:ParamQuantity {
    float getDisplayValue() override {
      float rtn = ParamQuantity::getDisplayValue();
      return rtn<0.001f ? 0.f : rtn;
    }
  };
  
  SVF() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam<FreqQuantity>(FREQ_PARAM, -4.f, 6.f, 0.f, "Cutoff frequency", " Hz", 2.f, rangeFreq[range], 0.f);
    configSwitch<FixedSwitchQuantity>(SLOPE_PARAM, 0.f, 7.f, 0.f, "Slope", {"12dB", "24dB", "36dB", "48dB", "60dB", "72dB", "84dB", "96dB"});
    configSwitch<FixedSwitchQuantity>(RANGE_PARAM, 0.f, 1.f, 0.f, "Frequency range", {"Audio rate", "Low frequency"});
    configInput(VOCT_INPUT, "V/Oct");
    configParam(FREQ_CV_PARAM, -1.f, 1.f, 0.f, "Cutoff CV amount", "%", 0.f, 100.f, 0.f);
    configInput(FREQ_CV_INPUT, "Cutoff CV");

    configParam(RES_PARAM, 0.f, 1.f, 0.f, "Resonance");
    configParam(RES_CV_PARAM, -1.f, 1.f, 0.f, "Resonance CV amount", "%", 0.f, 100.f, 0.f);
    configInput(RES_CV_INPUT, "Resonance CV");

    configParam(DRIVE_PARAM, 0.f, 2.f, 1.f, "Gain");
    configParam(DRIVE_CV_PARAM, -1.f, 1.f, 0.f, "Gain CV amount", "%", 0.f, 100.f, 0.f);
    configInput(DRIVE_CV_INPUT, "Gain CV");
    
    configParam(SPREAD_PARAM, -2.f, 2.f, 0.f, "Cutoff spread", "");
    configSwitch<FixedSwitchQuantity>(SPREAD_DIR_PARAM, 0.f, 1.f, 0.f, "Spread direction", {"Bipolar (both)", "Unipolar (right)"});
    configSwitch<FixedSwitchQuantity>(SPREAD_MONO_PARAM, 0.f, 1.f, 0.f, "Spread mono mode", {"Additive", "Subtractive"});
    configParam(SPREAD_CV_PARAM, -1.f, 1.f, 0.f, "Cutoff spread CV amount", "%", 0.f, 100.f, 0.f);
    configInput(SPREAD_CV_INPUT, "Cutoff spread CV");

    configParam<FdbkQuantity>(FDBK_PARAM, -10.f, 0.f, -10.f, "Feedback", "", 2.f, 1.f, 0.f);
    configParam(FDBK_CV_PARAM, -1.f, 1.f, 0.f, "Feedback CV amount", "%", 0.f, 100.f, 0.f);
    configInput(FDBK_CV_INPUT, "Feedback CV");

    configParam(MORPH_PARAM, 0.f, 1.f, 0.5f, "Morph", "");
    configSwitch<FixedSwitchQuantity>(MORPH_MODE_PARAM, 0.f, 3.f, 2.f, "Morph mode", {"LP <-> BP", "LP <-> BP <-> HP", "LP <-> HP", "BP <-> HP"});
    configParam(MORPH_CV_PARAM, -1.f, 1.f, 0.f, "Morph CV Amount", "%", 0.f, 100.f, 0.f);
    configInput(MORPH_CV_INPUT, "Morph CV");

    configInput(L_INPUT, "Left");
    configInput(R_INPUT, "Right");
    configOutput(L_MORPH_OUTPUT, "Left morph");
    configOutput(R_MORPH_OUTPUT, "Right morph");

    configOutput(L_LOW_OUTPUT, "Left low pass");
    configOutput(R_LOW_OUTPUT, "Right low pass");
    configOutput(L_HIGH_OUTPUT, "Left high pass");
    configOutput(R_HIGH_OUTPUT, "Right high pass");
   
    configOutput(L_BAND_OUTPUT, "Left band pass");
    configOutput(R_BAND_OUTPUT, "Right band pass");
    configOutput(L_NOTCH_OUTPUT, "Left notch pass");
    configOutput(R_NOTCH_OUTPUT, "Right notch pass");
    
    for (int i=0; i<OUTPUTS_LEN; i+=2){
      configBypass(L_INPUT,i);
      configBypass(R_INPUT,i+1);
    }

    setOversample();
  }

  void setOversample() override {
    for (int i=0; i<8; i++){
      stereoUpSample[i].setOversample(oversample, 5);
      morphDownSample[i].setOversample(oversample, 5);
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
      maxFreq = sampleRate<12000 ? 3500
              : sampleRate<22000 ? 3750
              : sampleRate<24000 ? 7000
              : sampleRate<44000 ? 7500
              : sampleRate<48000 ? 14000
              : sampleRate<88000 ? 15000
              : sampleRate<96000 ? 28000
              : 30000;
      if (range)
        maxFreq /= 2.f;
      static_cast<FreqQuantity*>(paramQuantities[FREQ_PARAM])->maxFreq = maxFreq;
      for (int i=0; i<5; i++)
        for (int j=0; j<8; j++)
          dcBlockFilter[i][j].init(oversample, sampleRate);
    }

    int slope = params[SLOPE_PARAM].getValue(),
        dir = params[SPREAD_DIR_PARAM].getValue(),
        mono = params[SPREAD_MONO_PARAM].getValue(),
        mode = params[MORPH_MODE_PARAM].getValue();
    
    float freqParam = params[FREQ_PARAM].getValue(),
          resParam = params[RES_PARAM].getValue(),
          driveParam = params[DRIVE_PARAM].getValue(),
          fdbkParam = params[FDBK_PARAM].getValue(),
          morphParam = params[MORPH_PARAM].getValue(),
          freqCVAmt = params[FREQ_CV_PARAM].getValue(),
          resCVAmt = params[RES_CV_PARAM].getValue() / 10.f,
          driveCVAmt = params[DRIVE_CV_PARAM].getValue() / 5.f,
          spreadCVAmt = params[SPREAD_CV_PARAM].getValue(),
          fdbkCVAmt = params[FDBK_CV_PARAM].getValue(),
          morphCVAmt = params[MORPH_CV_PARAM].getValue() / 10.f,
          sampleTimePi = M_PI * args.sampleTime / oversample;

    float_4 spreadParam{},
            voctIn{},
            freqIn{},
            resIn{},
            driveIn{},
            spreadIn{},
            fdbkIn{},
            morphIn{},
            stereoIn{},
            voct{},
            freq{},
            res{},
            drive{},
            fdbkAmt{},
            stereo{},
            f{},
            q{},
            low{},
            band{},
            high{},
            notch{},
            morph{},
            morphARatio{},
            morphBRatio{},
            morphBHigh{},
            morphA,
            morphB;

    bool stereoConnected = inputs[L_INPUT].isConnected() || inputs[R_INPUT].isConnected(),
         outConnected[5]{ outputs[L_LOW_OUTPUT].isConnected() || outputs[R_LOW_OUTPUT].isConnected(),
                          outputs[L_HIGH_OUTPUT].isConnected() || outputs[R_HIGH_OUTPUT].isConnected(),
                          outputs[L_BAND_OUTPUT].isConnected() || outputs[R_BAND_OUTPUT].isConnected(),
                          outputs[L_NOTCH_OUTPUT].isConnected() || outputs[R_NOTCH_OUTPUT].isConnected(),
                          outputs[L_MORPH_OUTPUT].isConnected() || outputs[R_MORPH_OUTPUT].isConnected() },
         rightConnected[5]{ outputs[R_LOW_OUTPUT].isConnected(),
                            outputs[R_HIGH_OUTPUT].isConnected(),
                            outputs[R_BAND_OUTPUT].isConnected(),
                            outputs[R_NOTCH_OUTPUT].isConnected(),
                            outputs[R_MORPH_OUTPUT].isConnected() };

    if (dir) {
      spreadParam[1] = spreadParam[3] = params[SPREAD_PARAM].getValue();
    }
    else {
      spreadParam[0] = spreadParam[2] = params[SPREAD_PARAM].getValue()*-0.5f;
      spreadParam[1] = spreadParam[3] = -spreadParam[0];
    }

    // get channel count
    int channels=1;
    for (int i=0; i<INPUTS_LEN; i++){
      if(inputs[i].getChannels() > channels)
        channels = inputs[i].getChannels();
    }

    for (int s=0, c1=0, c2=1; c1<channels; s++, c1+=2, c2+=2) { // poly channel loop
      voctIn[0] = inputs[VOCT_INPUT].getPolyVoltage(c1);
      voctIn[1] = voctIn[0];
      voctIn[2] = inputs[VOCT_INPUT].getPolyVoltage(c2);
      voctIn[3] = voctIn[2];
      freqIn[0] = inputs[FREQ_CV_INPUT].getPolyVoltage(c1);
      freqIn[1] = freqIn[0];
      freqIn[2] = inputs[FREQ_CV_INPUT].getPolyVoltage(c2);
      freqIn[3] = freqIn[2];
      resIn[0] = inputs[RES_CV_INPUT].getPolyVoltage(c1);
      resIn[1] = resIn[0];
      resIn[2] = inputs[RES_CV_INPUT].getPolyVoltage(c2);
      resIn[3] = resIn[2];
      driveIn[0] = inputs[DRIVE_CV_INPUT].getPolyVoltage(c1);
      driveIn[1] = driveIn[0];
      driveIn[2] = inputs[DRIVE_CV_INPUT].getPolyVoltage(c2);
      driveIn[3] = driveIn[2];

      if (dir){
        spreadIn[1] = inputs[SPREAD_CV_INPUT].getPolyVoltage(c1);
        spreadIn[3] = inputs[SPREAD_CV_INPUT].getPolyVoltage(c2);
      }
      else {
        spreadIn[0] = inputs[SPREAD_CV_INPUT].getPolyVoltage(c1)*-0.5f;
        spreadIn[2] = inputs[SPREAD_CV_INPUT].getPolyVoltage(c2)*-0.5f;
        spreadIn[1] = -spreadIn[0];
        spreadIn[3] = -spreadIn[2];
      }

      fdbkIn[0] = inputs[FDBK_CV_INPUT].getPolyVoltage(c1);
      fdbkIn[1] = fdbkIn[0];
      fdbkIn[2] = inputs[FDBK_CV_INPUT].getPolyVoltage(c2);
      fdbkIn[3] = fdbkIn[2];

      morphIn[0] = inputs[MORPH_CV_INPUT].getPolyVoltage(c1);
      morphIn[1] = morphIn[0];
      morphIn[2] = inputs[MORPH_CV_INPUT].getPolyVoltage(c2);
      morphIn[3] = morphIn[2];

      stereoIn[0] = inputs[L_INPUT].getPolyVoltage(c1);
      stereoIn[1] = inputs[R_INPUT].getNormalPolyVoltage(stereoIn[0], c1);
      stereoIn[2] = inputs[L_INPUT].getPolyVoltage(c2);
      stereoIn[3] = inputs[R_INPUT].getNormalPolyVoltage(stereoIn[2], c2);
      freq = pow(2.f, freqParam + voctIn + freqIn*freqCVAmt + spreadParam + spreadIn*spreadCVAmt) * rangeFreq[range];
      freq = ifelse(freq>maxFreq, maxFreq, freq);
      res = clamp(resParam + resIn/10.f * resCVAmt) * 9.f;
      drive = clamp(driveParam + driveIn/5.f * driveCVAmt, 0.f, 2.f);
      fdbkAmt = clamp(pow(2.f, fdbkParam + fdbkIn*fdbkCVAmt));
      fdbkAmt = ifelse(fdbkAmt<0.001f, 0.f, fdbkAmt);
      f = 2.f * sin(sampleTimePi * freq);
      q = (slope==0) ? 1.f / pow(2.f, res) : 1.f;
      if (outConnected[MORPH]){
        morphBRatio = clamp(morphParam + morphIn*morphCVAmt);
        if (mode==1){
          morphBRatio *= 2.f;
          morphBHigh = morphBRatio > 1.f;
          morphBRatio = ifelse(morphBHigh, morphBRatio - 1.f, morphBRatio);
        }
        morphARatio = 1.f - morphBRatio;
      }
      for (int o=0; o<oversample; o++){ // oversample loop
        if (oversample>1 && stereoConnected) {
          stereoIn = stereoUpSample[s].process(o ? 0.f : stereoIn*oversample);
        }
        stereo = (stereoIn * drive) + (1e-4f * (2.f*random::uniform() - 1.f)) + (fdbkOld[s] * fdbkAmt);
        notch = state[NOTCH][s] = q * state[BAND][s] - stereo;
        high = state[HIGH][s] = -(state[NOTCH][s] + state[LOW][s]);
        band = state[BAND][s] = state[BAND][s] + f * state[HIGH][s];
        low = state[LOW][s] = state[LOW][s] + f * state[BAND][s];
        for (int i=0; i<slope; i++){ // slope loop
          if (i==slope-1)
            q = 1.f / pow(2.f, res);
          int b=LOW;
          if (outConnected[b] || outConnected[MORPH]){
            stereo = low;
            modeState[b][i][NOTCH][s] = q * modeState[b][i][BAND][s] - stereo;
            modeState[b][i][HIGH][s] = -(modeState[b][i][NOTCH][s] + modeState[b][i][LOW][s]);
            modeState[b][i][BAND][s] = modeState[b][i][BAND][s] + f * modeState[b][i][HIGH][s];
            low = modeState[b][i][LOW][s] = modeState[b][i][LOW][s] + f * modeState[b][i][BAND][s];
          }
          b=HIGH;
          if (outConnected[b] || outConnected[MORPH]){
            stereo = high;
            modeState[b][i][NOTCH][s] = q * modeState[b][i][BAND][s] - stereo;
            high = modeState[b][i][HIGH][s] = -(modeState[b][i][NOTCH][s] + modeState[b][i][LOW][s]);
            modeState[b][i][BAND][s] = modeState[b][i][BAND][s] + f * modeState[b][i][HIGH][s];
            modeState[b][i][LOW][s] = modeState[b][i][LOW][s] + f * modeState[b][i][BAND][s];
          }
          b=BAND;
          // always compute band states for feedback
            stereo = band;
            modeState[b][i][NOTCH][s] = q * modeState[b][i][BAND][s] - stereo;
            modeState[b][i][HIGH][s] = -(modeState[b][i][NOTCH][s] + modeState[b][i][LOW][s]);
            band = modeState[b][i][BAND][s] = modeState[b][i][BAND][s] + f * modeState[b][i][HIGH][s];
            modeState[b][i][LOW][s] = modeState[b][i][LOW][s] + f * modeState[b][i][BAND][s];
          //
          b=NOTCH;
          if (outConnected[b]){
            stereo = notch;
            notch = modeState[b][i][NOTCH][s] = q * modeState[b][i][BAND][s] - stereo;
            modeState[b][i][HIGH][s] = -(modeState[b][i][NOTCH][s] + modeState[b][i][LOW][s]);
            modeState[b][i][BAND][s] = modeState[b][i][BAND][s] + f * modeState[b][i][HIGH][s];
            modeState[b][i][LOW][s] = modeState[b][i][LOW][s] + f * modeState[b][i][BAND][s];
          }
        } // end slope loop
        fdbkOld[s] = softClip(band/10.f);
        if (outConnected[MORPH]){
          switch (mode){
            case 0:
              morphA = low;
              morphB = band * (slope%4==1?-1.f:1.f);
              break;
            case 1:
              morphA = ifelse(morphBHigh, band * (slope%4==1?-1.f:1.f), low);
              morphB = ifelse(morphBHigh, high, band * (slope%4==1?-1.f:1.f));
              break;
            case 2:
              morphA = low;
              morphB = high * (slope%2?1.f:-1.f);
              break;
            default: // 3
              morphA = band * (slope%4==1?-1.f:1.f);
              morphB = high;
          }
          morph = morphA * morphARatio + morphB * morphBRatio;
          if (!rightConnected[MORPH]){
            morph[0] = mono ? morph[0]-morph[1] : (morph[0]+morph[1])/2.f;
            morph[2] = mono ? morph[2]-morph[3] : (morph[2]+morph[3])/2.f;
            morph[1] = morph[3] = 0.f;
          }
          morph = softClip(morph);
          if (oversample>1)
            morph = morphDownSample[s].process(morph);
        }
        if (outConnected[LOW]){
          if (!rightConnected[LOW]){
            low[0] = mono ? low[0]-low[1] : (low[0]+low[1])/2.f;
            low[2] = mono ? low[2]-low[3] : (low[2]+low[3])/2.f;
            low[1] = low[3] = 0.f;
          }
          low = softClip(low);
          if (oversample>1)
            low = lowDownSample[s].process(low);
        }
        if (outConnected[HIGH]){
          if (!rightConnected[HIGH]){
            high[0] = mono ? high[0]-high[1] : (high[0]+high[1])/2.f;
            high[2] = mono ? high[2]-high[3] : (high[2]+high[3])/2.f;
            high[1] = high[3] = 0.f;
          }
          high = softClip(high);
          if (oversample>1)
            high = highDownSample[s].process(high);
        }
        if (outConnected[BAND]){
          if (!rightConnected[BAND]){
            band[0] = mono ? band[0]-band[1] : (band[0]+band[1])/2.f;
            band[2] = mono ? band[2]-band[3] : (band[2]+band[3])/2.f;
            band[1] = band[3] = 0.f;
          }
          band = softClip(band);
          if (oversample>1)
            band = bandDownSample[s].process(band);
        }
        if (outConnected[NOTCH]){
          if (!rightConnected[NOTCH]){
            notch[0] = mono ? notch[0]-notch[1] : (notch[0]+notch[1])/2.f;
            notch[2] = mono ? notch[2]-notch[3] : (notch[2]+notch[3])/2.f;
            notch[1] = notch[3] = 0.f;
          }
          notch = softClip(notch);
          if (oversample>1)
            notch = notchDownSample[s].process(notch);
        }
      } // end oversample loop
      if (range==0 && !disableDCBlock){
        if (outConnected[MORPH])
          morph = dcBlockFilter[MORPH][s].process(morph);
        if (outConnected[LOW])
          low = dcBlockFilter[LOW][s].process(low);
        if (outConnected[HIGH])
          high = dcBlockFilter[HIGH][s].process(high);
        if (outConnected[BAND])
          band = dcBlockFilter[BAND][s].process(band);
        if (outConnected[NOTCH])
          notch = dcBlockFilter[NOTCH][s].process(notch);
      }
      outputs[L_MORPH_OUTPUT].setVoltage(morph[0], c1);
      outputs[R_MORPH_OUTPUT].setVoltage(morph[1], c1);
      outputs[L_MORPH_OUTPUT].setVoltage(morph[2], c2);
      outputs[R_MORPH_OUTPUT].setVoltage(morph[3], c2);
      
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
     
    } // end poly channel loop
    outputs[L_MORPH_OUTPUT].setChannels(channels);
    outputs[L_LOW_OUTPUT].setChannels(channels);
    outputs[L_HIGH_OUTPUT].setChannels(channels);
    outputs[L_BAND_OUTPUT].setChannels(channels);
    outputs[L_NOTCH_OUTPUT].setChannels(channels);
    outputs[R_MORPH_OUTPUT].setChannels(channels);
    outputs[R_LOW_OUTPUT].setChannels(channels);
    outputs[R_HIGH_OUTPUT].setChannels(channels);
    outputs[R_BAND_OUTPUT].setChannels(channels);
    outputs[R_NOTCH_OUTPUT].setChannels(channels);
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "disableDCBlock", json_boolean(disableDCBlock));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "disableDCBlock"))) {
      disableDCBlock = json_boolean_value(val);
    }
  }
        
};

struct SVFWidget : VenomWidget {
  
  struct RangeSwitch : GlowingSvgSwitchLockable {
    RangeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
    }
  };

  struct SlopeSwitch : GlowingSvgSwitchLockable {
    SlopeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPinkButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };
  
  struct DirSwitch : GlowingSvgSwitchLockable {
    DirSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };

  struct MonoSwitch : GlowingSvgSwitchLockable {
    MonoSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
    }
  };

  struct MorphSwitch : GlowingSvgSwitchLockable {
    MorphSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  SVFWidget(SVF* module) {
    setModule(module);
    setVenomPanel("SVF");
    
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(26.5f,49.5f), module, SVF::FREQ_PARAM));
    addParam(createLockableParamCentered<SlopeSwitch>(Vec(50.5f,30.f), module, SVF::SLOPE_PARAM));
    addParam(createLockableParamCentered<RangeSwitch>(Vec(63.5f,30.f), module, SVF::RANGE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(67.5f,60.5f), module, SVF::FREQ_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(88.5f, 34.5f), module, SVF::VOCT_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(108.5f, 60.5f), module, SVF::FREQ_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(26.5f,89.5f), module, SVF::RES_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(67.5f,89.5f), module, SVF::RES_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(108.5f, 89.5f), module, SVF::RES_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(26.5f,124.5f), module, SVF::DRIVE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(67.5f,124.5f), module, SVF::DRIVE_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(108.5f, 124.5f), module, SVF::DRIVE_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(26.5f,159.5f), module, SVF::SPREAD_PARAM));
    addParam(createLockableParamCentered<DirSwitch>(Vec(50.5f,143.f), module, SVF::SPREAD_DIR_PARAM));
    addParam(createLockableParamCentered<MonoSwitch>(Vec(63.5f,143.f), module, SVF::SPREAD_MONO_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(67.5f,159.5f), module, SVF::SPREAD_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(108.5f, 159.5f), module, SVF::SPREAD_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(26.5f,194.5f), module, SVF::FDBK_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(67.5f,194.5f), module, SVF::FDBK_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(108.5f, 194.5f), module, SVF::FDBK_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(26.5f,229.5f), module, SVF::MORPH_PARAM));
    addParam(createLockableParamCentered<MorphSwitch>(Vec(50.5f,213.f), module, SVF::MORPH_MODE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(67.5f,229.5f), module, SVF::MORPH_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(108.5f, 229.5f), module, SVF::MORPH_CV_INPUT));

    addInput(createInputCentered<PolyPort>(Vec(19.5f, 267.f), module, SVF::L_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(49.5f, 267.f), module, SVF::R_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(85.5f,267.5f), module, SVF::L_MORPH_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(115.5f,267.5f), module, SVF::R_MORPH_OUTPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(19.5f,305.5f), module, SVF::L_LOW_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(49.5f,305.5f), module, SVF::R_LOW_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(85.5f,305.5f), module, SVF::L_HIGH_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(115.5f,305.5f), module, SVF::R_HIGH_OUTPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(19.5f,343.5f), module, SVF::L_BAND_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(49.5f,343.5f), module, SVF::R_BAND_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(85.5f,343.5f), module, SVF::L_NOTCH_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(115.5f,343.5f), module, SVF::R_NOTCH_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    SVF* module = static_cast<SVF*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Disable audio output DC block", "", &module->disableDCBlock));    
    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelSVF = createModel<SVF, SVFWidget>("SVF");

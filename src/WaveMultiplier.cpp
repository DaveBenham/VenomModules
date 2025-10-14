// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "Filter.hpp"
#include "math.hpp"

#define LIGHT_OFF 0.02f
#define LIGHT_ON  1.f

#define DEPTH 0
#define LEVEL 1
#define WAVE  2

struct WaveMultiplier : VenomModule {

  enum ParamId {
    MASTER_PARAM,
    ENUMS(FREQ_PARAM,4),
    DEPTH_PARAM,
    DEPTH_CV_PARAM,
    ENUMS(SHIFT_CV_PARAM,4),
    ENUMS(SHIFT_PARAM,4),
    DC_PARAM,
    OVER_PARAM,
    ENUMS(MUTE_PARAM,4),
    MUTE_IN_PARAM,
    LEVEL_CV_PARAM,
    LEVEL_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    VOCT_INPUT,
    DEPTH_INPUT,
    ENUMS(SHIFT_INPUT,4),
    WAVE_INPUT,
    LEVEL_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(MOD_OUTPUT,4),
    ENUMS(PULSE_OUTPUT,4),
    ENUMS(WAVE_OUTPUT,4),
    MIX_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    DC_LIGHT,
    ENUMS(MUTE_LIGHT,4),
    MUTE_IN_LIGHT,
    LIGHTS_LEN
  };
  
  int oversample = 0, sampleRate = 0;
  int oversampleValues[6]{1,2,4,8,16,32};
  OversampleFilter_4 threshUpSample[16],
                     miscUpSample[16],
                     pulseDownSample[16],
                     shiftWaveDownSample[16];
  OversampleFilter   mixDownSample[16];
  DCBlockFilter_4    pulseDCBlock[16],
                     shiftWaveDCBlock[16];
  DCBlockFilter      mixDCBlock[16];
  simd::float_4      phasor[16]{};

  WaveMultiplier() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam(MASTER_PARAM, -4.f, 4.f, -4.f, "Master LFO frequency");
    configInput(VOCT_INPUT, "Master LFO V/Oct");

    configInput(DEPTH_INPUT, "Shift depth CV");
    configParam(DEPTH_CV_PARAM, -1.f, 1.f, 0.f, "Shift depth CV amount", "%", 0, 100, 0);
    configParam(DEPTH_PARAM, 0.f, 1.f, 1.f, "Shift depth", " V", 0, 5, 0);

    configSwitch<FixedSwitchQuantity>(DC_PARAM, 0.f, 1.f, 1.f, "DC Block", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 2.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});

    float freqDflt[4] {-0.72f, -0.27f, 0.19f, 0.88f}; 
    for (int i=0; i<4; i++) {
      std::string iStr = std::to_string(i+1);
      configParam(FREQ_PARAM+i, -1.f, 1.f, freqDflt[i], "LFO " + iStr + " frequency offset");
      configOutput(MOD_OUTPUT+i, "LFO " + iStr);
      configInput(SHIFT_INPUT+i, "Shift threshold CV " + iStr);
      configParam(SHIFT_CV_PARAM+i, 0.f, 1.f, 1.f, "Shift threshold CV amount " + iStr, "%", 0, 100, 0);
      configParam(SHIFT_PARAM+i, -5.f, 5.f, 0.f, "Shift threshold " + iStr, " V");
      configOutput(PULSE_OUTPUT+i, "Pulse " + iStr);
      configOutput(WAVE_OUTPUT+i, "Shifted wave " + iStr);
      configSwitch<FixedSwitchQuantity>(MUTE_PARAM+i, 0.f, 1.f, 0.f, "Mute shifted wave " + iStr, {"Off", "On"});
    }

    configInput(WAVE_INPUT, "Wave");
    configSwitch<FixedSwitchQuantity>(MUTE_IN_PARAM, 0.f, 1.f, 0.f, "Mute input wave", {"Off", "On"});
    configInput(LEVEL_INPUT, "Mix output level CV");
    configParam(LEVEL_CV_PARAM, -1.f, 1.f, 0.f, "Mix output level CV amount", "%", 0, 100, 0);
    configParam(LEVEL_PARAM, 0.f, 1.f, 0.25f, "Mix output level", "%", 0, 100, 0);
    configOutput(MIX_OUTPUT, "Shifted wave mix");

    configBypass(WAVE_INPUT, MIX_OUTPUT);
    
    oversampleStages = 5;
  }
  
  void setOversample() override {
    if (oversample > 1) {
      for (int i=0; i<16; i++){
        threshUpSample[i].setOversample(oversample, oversampleStages);
        miscUpSample[i].setOversample(oversample, oversampleStages);
        pulseDownSample[i].setOversample(oversample, oversampleStages);
        shiftWaveDownSample[i].setOversample(oversample, oversampleStages);
        mixDownSample[i].setOversample(oversample, oversampleStages);
      }
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    
    using float_4 = simd::float_4;
    
    if (oversample != oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())]) {
      oversample = oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())];
      setOversample();
      sampleRate = 0;
    }
    if (sampleRate != args.sampleRate){
      sampleRate = args.sampleRate;
      for (int i=0; i<16; i++){
        pulseDCBlock[i].init(oversample, sampleRate);
        shiftWaveDCBlock[i].init(oversample, sampleRate);
        mixDCBlock[i].init(oversample, sampleRate);
      }
    }

    // get channel count
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++) {
      int c = inputs[i].getChannels();
      if (c>channels)
        channels = c;
    }

    bool multiMod = inputs[VOCT_INPUT].getChannels()>1,
         hasPulseOut = false,
         hasShiftWaveOut = false,
         hasMixOut = outputs[MIX_OUTPUT].isConnected();

    float baseFreq = 0.f,
          k = args.sampleTime * 2.f,
          depthAmt = params[DEPTH_CV_PARAM].getValue()/10.f,
          depthParam = params[DEPTH_PARAM].getValue(),
          level = 0.f,
          levelAmt = params[LEVEL_CV_PARAM].getValue()/10.f,
          levelParam = params[LEVEL_PARAM].getValue(),
          depth = 0.f,
          wave = 0.f,
          mixOut = 0.f;

    float_4 freq{},
            delta{},
            tri{},
            misc{},
            thresh{},
            threshAmt{},
            threshParam{},
            pulse{},
            shiftWave{};

    for (int i=0; i<4; i++){
      threshAmt[i] = params[SHIFT_CV_PARAM+i].getValue();
      threshParam[i] = params[SHIFT_PARAM+i].getValue();
      if (outputs[PULSE_OUTPUT+i].isConnected())
        hasPulseOut = true;
      if (outputs[WAVE_OUTPUT+i].isConnected())
        hasShiftWaveOut = true;
    }
    for (int c=0; c<channels; c++){
      if (!c || multiMod){
        baseFreq = params[MASTER_PARAM].getValue() + inputs[VOCT_INPUT].getVoltage(c);
        for (int i=0; i<4; i++)
          freq[i] = baseFreq + params[FREQ_PARAM+i].getValue();
        freq = dsp::exp2_taylor5(freq);
        phasor[c] += freq * k;
        phasor[c] -= simd::ifelse(phasor[c]>1.f, 1.f, 0.f);
        tri = phasor[c] + simd::ifelse(phasor[c]<0.75f, 0.25f, -0.75f);
        tri = simd::ifelse(tri<0.5f, tri, (1.f-tri)) * 20.f - 5.f;
        for (int i=0; i<4; i++)
          outputs[MOD_OUTPUT+i].setVoltage(tri[i], c);
      }
      for (int i=0; i<4; i++)
        thresh[i] = inputs[SHIFT_INPUT+i].getNormalPolyVoltage(tri[i], c);
      misc[DEPTH] = inputs[DEPTH_INPUT].getPolyVoltage(c);
      misc[LEVEL] = inputs[LEVEL_INPUT].getPolyVoltage(c);
      misc[WAVE] = inputs[WAVE_INPUT].getPolyVoltage(c);
      for (int o=0; o<oversample; o++){
        if (oversample>1){
          thresh = threshUpSample[c].process(o ? float_4::zero() : thresh * oversample);
          misc = miscUpSample[c].process(o ? float_4::zero() : misc * oversample);
        }
        thresh = thresh * threshAmt + threshParam;
        depth = misc[DEPTH] * depthAmt + depthParam;
        wave = misc[WAVE];
        pulse = simd::ifelse(thresh>=wave, 5.f, -5.f);
        shiftWave = pulse * depth + wave;
        if (hasMixOut){
          mixOut = params[MUTE_IN_PARAM].getValue() ? 0.f : wave;
          for (int i=0; i<4; i++){
            if (!params[MUTE_PARAM+i].getValue())
              mixOut += shiftWave[i];
          }
          level = misc[LEVEL] * levelAmt + levelParam;
          mixOut *= level;
          if (params[DC_PARAM].getValue())
            mixOut = mixDCBlock[c].process(mixOut);
          if (oversample>1)
            mixOut = mixDownSample[c].process(mixOut);
        }
        if (hasPulseOut){
          if (params[DC_PARAM].getValue())
            pulse = pulseDCBlock[c].process(pulse);
          if (oversample>1)
            pulse = pulseDownSample[c].process(pulse);
        }
        if (hasShiftWaveOut){
          if (params[DC_PARAM].getValue())
            shiftWave = shiftWaveDCBlock[c].process(shiftWave);
          if (oversample>1)
            shiftWave = shiftWaveDownSample[c].process(shiftWave);
        }
      }
      for (int i=0; i<4; i++){
        outputs[PULSE_OUTPUT+i].setVoltage(pulse[i], c);
        outputs[WAVE_OUTPUT+i].setVoltage(shiftWave[i], c);
      }
      outputs[MIX_OUTPUT].setVoltage(mixOut, c);
    }
    for (int i=0; i<4; i++) {
      outputs[MOD_OUTPUT+i].setChannels(multiMod ? channels : 1);
      outputs[PULSE_OUTPUT+i].setChannels(channels);
      outputs[WAVE_OUTPUT+i].setChannels(channels);
      outputs[MIX_OUTPUT].setChannels(channels);
    }
  }

};

struct WaveMultiplierWidget : VenomWidget {

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

  WaveMultiplierWidget(WaveMultiplier* module) {
    setModule(module);
    setVenomPanel("WaveMultiplier");

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(20.5f, 48.5f), module, WaveMultiplier::MASTER_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(20.5f, 84.f), module, WaveMultiplier::VOCT_INPUT));

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 124.5f), module, WaveMultiplier::DEPTH_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(20.5f, 157.f), module, WaveMultiplier::DEPTH_CV_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(20.5f, 188.f), module, WaveMultiplier::DEPTH_PARAM));
    
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(17.5f,257.5f), module, WaveMultiplier::DC_PARAM, WaveMultiplier::DC_LIGHT));
    addParam(createLockableParam<OverSwitch>(Vec(8.5f, 287.f), module, WaveMultiplier::OVER_PARAM));
    
    float x = 82.5;
    for (int i=0; i<4; i++) {
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(x, 48.5f), module, WaveMultiplier::FREQ_PARAM+i));
      addOutput(createOutputCentered<PolyPort>(Vec(x, 84.f), module, WaveMultiplier::MOD_OUTPUT+i));
      addInput(createInputCentered<PolyPort>(Vec(x, 124.5f), module, WaveMultiplier::SHIFT_INPUT+i));
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(x, 157.f), module, WaveMultiplier::SHIFT_CV_PARAM+i));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(x, 188.f), module, WaveMultiplier::SHIFT_PARAM+i));
      addOutput(createOutputCentered<PolyPort>(Vec(x, 224.5f), module, WaveMultiplier::PULSE_OUTPUT+i));
      addOutput(createOutputCentered<PolyPort>(Vec(x, 257.5f), module, WaveMultiplier::WAVE_OUTPUT+i));
      addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(x, 297.5f), module, WaveMultiplier::MUTE_PARAM+i, WaveMultiplier::MUTE_LIGHT+i));
      x += 30.f;
    }

    addInput(createInputCentered<PolyPort>(Vec(20.5f, 342.5f), module, WaveMultiplier::WAVE_INPUT));
    addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(50.5, 342.5f), module, WaveMultiplier::MUTE_IN_PARAM, WaveMultiplier::MUTE_IN_LIGHT));
    addInput(createInputCentered<PolyPort>(Vec(80.5f, 342.5f), module, WaveMultiplier::LEVEL_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(110.5f, 342.5f), module, WaveMultiplier::LEVEL_CV_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(140.5f, 342.5f), module, WaveMultiplier::LEVEL_PARAM));
    addOutput(createOutputCentered<PolyPort>(Vec(172.f, 342.5f), module, WaveMultiplier::MIX_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    if(this->module) {
      WaveMultiplier* mod = static_cast<WaveMultiplier*>(this->module);
      mod->lights[WaveMultiplier::DC_LIGHT].setBrightness(mod->params[WaveMultiplier::DC_PARAM].getValue());
      for (int i=0; i<4; i++)
        mod->lights[WaveMultiplier::MUTE_LIGHT+i].setBrightness(mod->params[WaveMultiplier::MUTE_PARAM+i].getValue());
      mod->lights[WaveMultiplier::MUTE_IN_LIGHT].setBrightness(mod->params[WaveMultiplier::MUTE_IN_PARAM].getValue());
    }
  }
};

Model* modelWaveMultiplier = createModel<WaveMultiplier, WaveMultiplierWidget>("WaveMultiplier");

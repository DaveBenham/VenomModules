// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f
#define MAX_HARMONIC 63

struct XM_OP : VenomModule {
 
  enum ParamId {
    WAVE_PARAM,
    XM_TYPE_PARAM,
    FDBK_TYPE_PARAM,
    CURVE_PARAM,
    ATK_PARAM,
    DEC_PARAM,
    SUS_PARAM,
    REL_PARAM,
    ATK_CV_PARAM,
    DEC_CV_PARAM,
    SUS_CV_PARAM,
    REL_CV_PARAM,
    MULT_PARAM,
    DIV_PARAM,
    DTUNE_PARAM,
    MULT_CV_PARAM,
    DIV_CV_PARAM,
    DTUNE_CV_PARAM,
    LEVEL_ENV_PARAM,
    DEPTH_ENV_PARAM,
    FDBK_ENV_PARAM,
    LEVEL_PARAM,
    DEPTH_PARAM,
    FDBK_PARAM,
    OVER_PARAM,
    LEVEL_CV_PARAM,
    DEPTH_CV_PARAM,
    FDBK_CV_PARAM,
    SYNC_PARAM,
    ENV_OUT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    MOD_INPUT,
    LEVEL_INPUT,
    DEPTH_INPUT,
    FDBK_INPUT,
    GATE_INPUT,
    VOCT_INPUT,
    XMOD_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    ENV_OUTPUT,
    OSC_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    SYNC_LIGHT,
    LIGHTS_LEN
  };

  using float_4 = simd::float_4;

  int oversample = 0,
      sampleRate = 0,
      oldChannels = 0;
  std::vector<int> oversampleValues = {1,2,4,8,16,32};
  OversampleFilter_4 upSample[4]{}, downSample[4]{};
  float const minTime = pow(2.f,-12.f);
  float const maxTime = pow(2.f,7.5f);
  float_4 envPhasor[4]{},
          stage[4]{},
          relStart[4]{},
          prevEnvOut[4]{},
          gate[4]{},
          vcoPhasor[4]{},
          prevVcoOut[4]{};
  DCBlockFilter_4 xmodDcBlockFilter[4]{},
                  fdbkDcBlockFilter[4]{};
  
  int wave = 0,
      xmType = 0,
      fdbkType = 0;

  float harmonic[MAX_HARMONIC+1] = {
    #include "XM_OP.data"
  };
  
  XM_OP() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(WAVE_PARAM, 0.f, 3.f, 0.f, "Waveform", {"Sine", "Triangle", "Square", "Saw"});
    configSwitch<FixedSwitchQuantity>(XM_TYPE_PARAM, 0.f, 5.f, 0.f, "X-Mod type", {"Frequency modulation", "Phase modulation", "Ring modulation", "Amplitude modulation", "Rectified amplitude modulation", "Offset amplitude modulation"});
    configSwitch<FixedSwitchQuantity>(FDBK_TYPE_PARAM, 0.f, 5.f, 0.f, "Feedback type", {"Frequency modulation", "Phase modulation", "Ring modulation", "Amplitude modulation", "Rectified amplitude modulation", "Offset amplitude modulation"});
    configParam(CURVE_PARAM, 0.f, 1.f, 0.65f, "Envelope curve", "%", 0.f, 100.f);

    configParam(ATK_PARAM, -10.f, 3.5f, -6.5f, "Envelope attack time", " msec", 2.f, 1000.f, 0.f);
    configParam(DEC_PARAM, -10.f, 3.5f, -1.f, "Envelope decay time", " msec", 2.f, 1000.f, 0.f);
    configParam(SUS_PARAM, 0.f, 1.f, 0.7f, "Envelope sustain level", "%", 0.f, 100.f);
    configParam(REL_PARAM, -10.f, 3.5f, 1.f, "Envelope release time", " msec", 2.f, 1000.f, 0.f);

    configParam(ATK_CV_PARAM, -1.f, 1.f, 0.f, "Envelope attack time mod amount", "%", 0.f, 100.f);
    configParam(DEC_CV_PARAM, -1.f, 1.f, 0.f, "Envelope decay time mod amount", "%", 0.f, 100.f);
    configParam(SUS_CV_PARAM, -1.f, 1.f, 0.f, "Envelope sustain level mod amount", "%", 0.f, 100.f);
    configParam(REL_CV_PARAM, -1.f, 1.f, 0.f, "Envelope release time mod amount", "%", 0.f, 100.f);

    configParam(MULT_PARAM, 0.f, MAX_HARMONIC, 0.f, "VCO frequency multiplier", "", 0.f, 1.f, 1.f);
    configParam(DIV_PARAM, 0.f, MAX_HARMONIC, 0.f, "VCO frequency divisor", "", 0.f, 1.f, 1.f);
    configParam(DTUNE_PARAM, -1.f/12.f, 1.f/12.f, 0.f, "VCO detune", " cents", 0.f, 1200.f);

    configParam(MULT_CV_PARAM, -1.f, 1.f, 0.f, "VCO frequency multiplier mod amount", "%", 0.f, 100.f);
    configParam(DIV_CV_PARAM, -1.f, 1.f, 0.f, "VCO frequency divisor mod amount", "%", 0.f, 100.f);
    configParam(DTUNE_CV_PARAM, -1.f, 1.f, 0.f, "VCO detune mod amount", "%", 0.f, 100.f);
    configInput(MOD_INPUT, "Modulation");

    configSwitch<FixedSwitchQuantity>(LEVEL_ENV_PARAM, 0.f, 2.f, 0.f, "Level envelope", {"Off", "On", "Inverted"});
    configSwitch<FixedSwitchQuantity>(DEPTH_ENV_PARAM, 0.f, 2.f, 0.f, "X-Mod depth envelope", {"Off", "On", "Inverted"});
    configSwitch<FixedSwitchQuantity>(FDBK_ENV_PARAM, 0.f, 2.f, 0.f, "Feedback depth envelope", {"Off", "On", "Inverted"});

    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 2.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});

    configParam(LEVEL_PARAM, 0.f, 1.f, 1.f, "VCA Level");
    configParam(DEPTH_PARAM, -1.f, 1.f, 0.f, "VCO X-Mod depth", "%", 0.f, 100.f);
    configParam(FDBK_PARAM, -1.f, 1.f, 0.f, "VCO Feedback depth", "%", 0.f, 100.f);

    configParam(LEVEL_CV_PARAM, -1.f, 1.f, 0.f, "VCA Level CV amount", "%", 0.f, 10.f);
    configParam(DEPTH_CV_PARAM, -1.f, 1.f, 0.f, "VCO X-Mod depth CV amount", "%", 0.f, 100.f);
    configParam(FDBK_CV_PARAM, -1.f, 1.f, 0.f, "VCO feedback depth CV amount", "%", 0.f, 10.f);
    configSwitch<FixedSwitchQuantity>(SYNC_PARAM, 0.f, 1.f, 0.f, "Sync VCO", {"Off", "On"});

    configInput(LEVEL_INPUT, "VCA Level CV");
    configInput(DEPTH_INPUT, "VCO X-Mod depth CV");
    configInput(FDBK_INPUT, "VCO Feedback depth CV");
    configInput(GATE_INPUT, "Envelope gate");

    configInput(VOCT_INPUT, "VCO V/Oct");
    configInput(XMOD_INPUT, "VCO X-Mod");
    configSwitch<FixedSwitchQuantity>(ENV_OUT_PARAM, 0.f, 1.f, 0.f, "Envelope output mode", {"Normal", "Inverted"});
    configOutput(ENV_OUTPUT, "Envelope");
    configOutput(OSC_OUTPUT, "VCA");
   
    oversampleStages = 5;
  }

  float_4 sinSimd_1000(float_4 t) {
    t = simd::ifelse(t > 500.f, 1000.f - t, t) * 0.002f - 0.5f;
    float_4 t2 = t * t;
    return -(((-0.540347 * t2 + 2.53566) * t2 - 5.16651) * t2 + 3.14159) * t;
  }
  
  void setOversample() override {
    for (int i=0; i<4; i++){
      upSample[i].setOversample(oversample, oversampleStages);
      downSample[i].setOversample(oversample, oversampleStages);
    }
  }
  
  void loadPhases(float_4* phases, float_4 phasor, float_4 delta){
    phases[0] = phasor - 2 * delta + ifelse(phasor < 2 * delta, 1.f, ifelse(phasor > (1+2*delta),-1.f,0.f));
    phases[1] = phasor - delta + ifelse(phasor < delta, 1.f, ifelse(phasor > (1+delta),-1.f,0.f));
    phases[2] = phasor;
  }

  float_4 aliasSuppressedSaw(float_4* phases, float_4 denInv) {
    float_4 sawBuffer[3];
    for (int i = 0; i < 3; ++i) {
      float_4 p = phases[i] + phases[i] - 1.0;
      sawBuffer[i] = (p * p * p - p) / 6.0;
    }
    return ((sawBuffer[0] - sawBuffer[1] - sawBuffer[1] + sawBuffer[2])*denInv + 1.f) / 2.f;
  }

  float_4 aliasSuppressedOffsetSaw(float_4* phases, float_4 pw, float_4 denInv) {
    float_4 buffer[3];
    for (int i = 0; i < 3; ++i) {
      float_4 p = phases[i] + phases[i] - 1.0f;
      p += pw + pw;
      p += ifelse(p>1.f, -2.f, 0.f);
      buffer[i] = (p * p * p - p) / 6.0f;
    }
    return ((buffer[0] - buffer[1] - buffer[1] + buffer[2])*denInv + 1.f) / 2.f;
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    // update oversample
    if (oversample != oversampleValues[params[OVER_PARAM].getValue()]) {
      oversample = oversampleValues[params[OVER_PARAM].getValue()];
      setOversample();
      sampleRate = 0;
    }
    
    // update sampleRate
    if (sampleRate != args.sampleRate){
      sampleRate = args.sampleRate;
      for (int i=0; i<4; i++){
        xmodDcBlockFilter[i].init(oversample, sampleRate);
        fdbkDcBlockFilter[i].init(oversample, sampleRate);
      }
    }

    // get channel count
    int channels=1;
    for (int i=0; i<INPUTS_LEN; i++){
      if(inputs[i].getChannels() > channels)
        channels = inputs[i].getChannels();
    }
    // clear dropped channels
    for (int c=channels+1; c<oldChannels; c++) {
      gate[c/4][c%4] = 0.f;
      envPhasor[c/4][c%4] = 0.f;
    }
    oldChannels = channels;

    int wave = params[WAVE_PARAM].getValue(),
        xmodType = params[XM_TYPE_PARAM].getValue(),
        fdbkType = params[FDBK_TYPE_PARAM].getValue();         
    
    float atkParam = params[ATK_PARAM].getValue(),
          decParam = params[DEC_PARAM].getValue(),
          susParam = params[SUS_PARAM].getValue(),
          relParam = params[REL_PARAM].getValue(),
          atkCVAmt = params[ATK_CV_PARAM].getValue(),
          decCVAmt = params[DEC_CV_PARAM].getValue(),
          susCVAmt = params[SUS_CV_PARAM].getValue()/10.f,
          relCVAmt = params[REL_CV_PARAM].getValue(),
          shape = -params[CURVE_PARAM].getValue() * 0.95f,
          multParam = params[MULT_PARAM].getValue(),
          divParam = params[DIV_PARAM].getValue(),
          dtuneParam = params[DTUNE_PARAM].getValue(),
          multCVAmt = params[MULT_CV_PARAM].getValue(),
          divCVAmt = params[DIV_CV_PARAM].getValue(),
          dtuneCVAmt = params[DTUNE_CV_PARAM].getValue(),
          levelParam = params[LEVEL_PARAM].getValue(),
          depthParam = params[DEPTH_PARAM].getValue(),
          fdbkParam = params[FDBK_PARAM].getValue(),
          levelCVAmt = params[LEVEL_CV_PARAM].getValue(),
          depthCVAmt = params[DEPTH_CV_PARAM].getValue(),
          fdbkCVAmt = params[FDBK_CV_PARAM].getValue(),
          levelEnv = params[LEVEL_ENV_PARAM].getValue(),
          depthEnv = params[DEPTH_ENV_PARAM].getValue(),
          fdbkEnv = params[FDBK_ENV_PARAM].getValue(),
          invEnv = params[ENV_OUT_PARAM].getValue(),
          k =  1000.f * args.sampleTime / oversample;          

    for (int s=0, c=0; c<channels; s++, c+=4) {
      float_4 mod = inputs[MOD_INPUT].getPolyVoltageSimd<float_4>(c),
              susLevel = clamp(susParam + mod*susCVAmt),
              delta = ifelse(stage[s]==1.f, args.sampleTime / clamp(pow(2.f, mod*atkCVAmt + atkParam), minTime, maxTime),
                        ifelse(stage[s]==2.f, args.sampleTime / clamp(pow(2.f, mod*decCVAmt + decParam), minTime, maxTime),
                          ifelse(stage[s]==4.f, args.sampleTime / clamp(pow(2.f, mod*relCVAmt + relParam), minTime, maxTime), 0.f)));
      envPhasor[s] = clamp(envPhasor[s]+delta);
      float_4 curve = normSigmoid(envPhasor[s], shape),
              envOut = ifelse(stage[s]<=1.f, curve,
                         ifelse(stage[s]==2.f, 1.f - curve*(1.f - susLevel),
                           ifelse(stage[s]==3.f, susLevel, relStart[s]*(1.f - curve)))),
              gateV = inputs[GATE_INPUT].getPolyVoltageSimd<float_4>(c),
              newGate = ifelse(gateV>2.f, 1.f,
                          ifelse(gateV<0.2f, 0.f, gate[s])),
              newStage = ifelse(newGate>gate[s], 1.f,
                           ifelse(newGate<gate[s], 4.f,
                             ifelse(envPhasor[s]<1.f, stage[s],
                               ifelse(stage[s]==4, 0.f, stage[s]+1))));
      envPhasor[s] = ifelse(newStage==stage[s], envPhasor[s],
                       ifelse(newStage==1.f, invNormSigmoid(envOut, shape),
                         ifelse((newStage==4.f) & (envOut<susLevel), invNormSigmoid((susLevel-envOut)/susLevel, shape), 0.f)));
      relStart[s] = ifelse((newStage!=stage[s]) & (newStage==4.f), envOut, relStart[s]);

      float_4 baseFreq = inputs[VOCT_INPUT].getPolyVoltageSimd<float_4>(c)
                         + dtuneParam + mod*dtuneCVAmt,
              xmod = inputs[XMOD_INPUT].getPolyVoltageSimd<float_4>(c),
              level = clamp(levelParam * (levelEnv==1.f ? envOut : levelEnv ? 1.f-envOut : 1.f)
                            + levelCVAmt * inputs[LEVEL_INPUT].getPolyVoltageSimd<float_4>(c)/10.f),
              depth = depthParam * (depthEnv==1.f ? envOut : depthEnv ? 1.f-envOut : 1.f)
                      + depthCVAmt * inputs[DEPTH_INPUT].getPolyVoltageSimd<float_4>(c)/10.f,
              fdbk = fdbkParam * (fdbkEnv==1.f ? envOut : fdbkEnv ? 1.f-envOut : 1.f)
                      + fdbkCVAmt * inputs[FDBK_INPUT].getPolyVoltageSimd<float_4>(c)/10.f,
              vcoOut{};
      for (int i=0; i<4; i++){
        baseFreq[i] += harmonic[clamp(static_cast<int>(multParam + mod[i]*10.f*multCVAmt),0,MAX_HARMONIC)];
        baseFreq[i] -= harmonic[clamp(static_cast<int>(divParam  + mod[i]*10.f*divCVAmt ),0,MAX_HARMONIC)];
      }
      baseFreq = dsp::exp2_taylor5(baseFreq);
      for (int o=0; o<oversample; o++) {
        if (oversample>1)
          xmod = upSample[s].process(o ? 0.f : xmod*oversample);
        float_4 freq = baseFreq;
        if (xmodType == 0)
          freq += xmodDcBlockFilter[s].process(xmod) * depth;
        if (fdbkType == 0)
          freq += fdbkDcBlockFilter[s].process(prevVcoOut[s]) * fdbk;
        freq *= dsp::FREQ_C4;
        float_4 basePhaseDelta = freq * k;
        vcoPhasor[s] += basePhaseDelta;
        // vals for dpw antialias
        basePhaseDelta *= 0.001f;
        float_4 lowFreq = simd::abs(basePhaseDelta) < 1e-3,
                denInv = 1.f/basePhaseDelta;
        denInv = denInv * denInv * 0.25;
        //
        vcoPhasor[s] = fmod(vcoPhasor[s], 1000.f);
        if (o==0)
          vcoPhasor[s] = ifelse(newGate>gate[s], 0.f, vcoPhasor[s]);
        float_4 phases[3]{},
                wavePhasor = vcoPhasor[s],
                sawPhasor{},
                offsetSawPhasor{};
        if (xmodType == 1) // PM
          wavePhasor += xmod * depth * 250.f;
        if (fdbkType == 1) // PM
          wavePhasor += prevVcoOut[s] * fdbk * 125.f;
        wavePhasor = simd::fmod(wavePhasor, 1000.f);
        wavePhasor = simd::ifelse(wavePhasor<0.f, wavePhasor+1000.f, wavePhasor);
        switch (wave) {
          case 0: // sin
            vcoOut = sinSimd_1000(wavePhasor + simd::ifelse(wavePhasor>250.f, -250.f, 750.f)) * 5.f;
            break;
          case 1: // tri
            wavePhasor += simd::ifelse(wavePhasor<750.f, 250.f, -750.f);
            vcoOut = ifelse(wavePhasor<500.f, wavePhasor*.002f, (1000.f-wavePhasor)*.002f) * 10.f - 5.f;
            break;
          case 2: // sqr
            vcoOut = ifelse(wavePhasor<500.f, 5.f, -5.f);
            loadPhases(phases, wavePhasor * 0.001f, basePhaseDelta);
            sawPhasor = aliasSuppressedSaw(phases, denInv);
            offsetSawPhasor = aliasSuppressedOffsetSaw(phases, 0.5f, denInv);
            vcoOut = ifelse(lowFreq, vcoOut, (offsetSawPhasor - sawPhasor) * 10.f);
            break;
          default: // saw
            wavePhasor *= 0.001f;
            loadPhases(phases, wavePhasor, basePhaseDelta);
            wavePhasor = ifelse(lowFreq, wavePhasor, aliasSuppressedSaw(phases, denInv));
            vcoOut = wavePhasor * 10.f - 5.f;
        }
        float_4 saveVcoOut = vcoOut;
        if (xmodType == 2) // RM
          vcoOut = crossfade( vcoOut, vcoOut * xmod/5.f, clamp(fabs(depth)));
        if (fdbkType == 2) // RM
          vcoOut = crossfade( vcoOut, vcoOut * prevVcoOut[s]/5.f, clamp(fabs(fdbk)));
        if (xmodType == 3) // AM
          vcoOut = crossfade( vcoOut, vcoOut * ifelse(xmod<0.f, 0.f, xmod/5.f), clamp(fabs(depth)));
        if (fdbkType == 3) // AM
          vcoOut = crossfade( vcoOut, vcoOut * ifelse(prevVcoOut[s]<0.f, 0.f, prevVcoOut[s]/5.f), clamp(fabs(fdbk)));
        if (xmodType == 4) // rect AM
          vcoOut = crossfade( vcoOut, vcoOut * fabs(xmod)/5.f, clamp(fabs(depth)));
        if (fdbkType == 4) // rect AM
          vcoOut = crossfade( vcoOut, vcoOut * fabs(prevVcoOut[s])/5.f, clamp(fabs(fdbk)));
        if (xmodType == 5) // offset AM
          vcoOut = crossfade( vcoOut, vcoOut * fabs(xmod+5.f)/10.f, clamp(fabs(depth)));
        if (fdbkType == 5) // offset AM
          vcoOut = crossfade( vcoOut, vcoOut * fabs(prevVcoOut[s]+5.f)/10.f, clamp(fabs(fdbk)));
        if (oversample>1)
          vcoOut = downSample[s].process(vcoOut);
        prevVcoOut[s] = saveVcoOut;
      }
      vcoOut *= level;
      gate[s] = newGate;
      stage[s] = newStage;
      outputs[ENV_OUTPUT].setVoltageSimd((invEnv ? 1.f-envOut : envOut)*10.f, c);
      outputs[OSC_OUTPUT].setVoltageSimd(vcoOut, c);
    }
    outputs[ENV_OUTPUT].setChannels(channels);
    outputs[OSC_OUTPUT].setChannels(channels);
  }
  
};

struct XM_OPWidget : VenomWidget {
  
  struct WaveSwitch : GlowingSvgSwitchLockable {
    WaveSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/wave_sin.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/wave_tri.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/wave_sqr.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/wave_saw.svg")));
    }
  };

  struct OpSwitch : GlowingSvgSwitchLockable {
    OpSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/xm_FM.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/xm_PM.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/xm_RM.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/xm_AM.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/xm_AMrect.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/xm_AMoffset.svg")));
    }
  };

  struct EnvSwitch : GlowingSvgSwitchLockable {
    EnvSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
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

  struct EnvOutSwitch : GlowingSvgSwitchLockable {
    EnvOutSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
    }
  };

  XM_OPWidget(XM_OP* module) {
    setModule(module);
    setVenomPanel("XM_OP");
    
    addParam(createLockableParamCentered<WaveSwitch>(Vec(22.5f,49.5f), module, XM_OP::WAVE_PARAM));
    addParam(createLockableParamCentered<OpSwitch>(Vec(57.5f,49.5f), module, XM_OP::XM_TYPE_PARAM));
    addParam(createLockableParamCentered<OpSwitch>(Vec(92.5f,49.5f), module, XM_OP::FDBK_TYPE_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(127.5f,49.5f), module, XM_OP::CURVE_PARAM));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f,89.5f), module, XM_OP::ATK_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(57.5f,89.5f), module, XM_OP::DEC_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(92.5f,89.5f), module, XM_OP::SUS_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(127.5f,89.5f), module, XM_OP::REL_PARAM));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(22.5f,121.f), module, XM_OP::ATK_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(57.5f,121.f), module, XM_OP::DEC_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(92.5f,121.f), module, XM_OP::SUS_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(127.5f,121.f), module, XM_OP::REL_CV_PARAM));

    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(Vec(22.5f,157.f), module, XM_OP::MULT_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(Vec(57.5f,157.f), module, XM_OP::DIV_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(92.5f,157.f), module, XM_OP::DTUNE_PARAM));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(22.5f,188.5f), module, XM_OP::MULT_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(57.5f,188.5f), module, XM_OP::DIV_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(92.5f,188.5f), module, XM_OP::DTUNE_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(127.5f, 188.5f), module, XM_OP::MOD_INPUT));

    addParam(createLockableParamCentered<EnvSwitch>(Vec(22.5f,207.5f), module, XM_OP::LEVEL_ENV_PARAM));
    addParam(createLockableParamCentered<EnvSwitch>(Vec(57.5f,207.5f), module, XM_OP::DEPTH_ENV_PARAM));
    addParam(createLockableParamCentered<EnvSwitch>(Vec(92.5f,207.5f), module, XM_OP::FDBK_ENV_PARAM));

    addParam(createLockableParamCentered<OverSwitch>(Vec(127.5f,230.f), module, XM_OP::OVER_PARAM));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f,235.f), module, XM_OP::LEVEL_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(57.5f,235.f), module, XM_OP::DEPTH_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(92.5f,235.f), module, XM_OP::FDBK_PARAM));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(22.5f,266.5f), module, XM_OP::LEVEL_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(57.5f,266.5f), module, XM_OP::DEPTH_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(92.5f,266.5f), module, XM_OP::FDBK_CV_PARAM));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(127.5f,266.5f), module, XM_OP::SYNC_PARAM, XM_OP::SYNC_LIGHT));

    addInput(createInputCentered<PolyPort>(Vec(22.5f, 298.5f), module, XM_OP::LEVEL_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(57.5f, 298.5f), module, XM_OP::DEPTH_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(92.5f, 298.5f), module, XM_OP::FDBK_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(127.5f, 298.5f), module, XM_OP::GATE_INPUT));

    addInput(createInputCentered<PolyPort>(Vec(22.5f, 339.5f), module, XM_OP::VOCT_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(57.5f, 339.5f), module, XM_OP::XMOD_INPUT));
    addParam(createLockableParamCentered<EnvOutSwitch>(Vec(105.f,323.f), module, XM_OP::ENV_OUT_PARAM));
    addOutput(createOutputCentered<PolyPort>(Vec(92.5f,339.5f), module, XM_OP::ENV_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(127.5f,339.5f), module, XM_OP::OSC_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    XM_OP* mod = dynamic_cast<XM_OP*>(this->module);
    if(mod) {
      mod->lights[XM_OP::SYNC_LIGHT].setBrightness(mod->params[XM_OP::SYNC_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }
  }

};

Model* modelXM_OP = createModel<XM_OP, XM_OPWidget>("XM_OP");

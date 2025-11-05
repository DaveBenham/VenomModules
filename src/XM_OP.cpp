// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "Filter.hpp"
#include "math.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f
#define MAX_HARMONIC 63

namespace Venom {

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
    SYNC_RTRG_MODE_PARAM,
    ENV_OUT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    SMOD_INPUT,
    RMOD_INPUT,
    SYNC_RTRG_INPUT,
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
  
  bool levelVelo = false,
       depthVelo = false,
       fdbkVelo = false;

  std::string veloParamNames[3] = {"VCA velocity floor", "VCO X-Mod velocity floor", "VCO feedback velocity floor"},
              veloPortNames[3] = {"VCA velocity", "VCO X-Mod velocity", "VCO feedback velocity"},
              cvAmtParamNames[3] = {"VCA level CV amount", "VCO X-Mod depth CV amount", "VCO feedback depth CV amount"},
              cvAmtPortNames[3] = {"VCA level CV", "VCO X-Mod depth CV", "VCO feedback depth CV"};

  int oversample = 0,
      sampleRate = 0,
      oldChannels = 0,
      syncRtrgMode = 0;
  std::vector<int> oversampleValues = {1,2,4,8,16,32};
  OversampleFilter_4 upSample[4]{}, downSample[4]{};
  float const minTime = pow(2.f,-12.f);
  float const maxTime = pow(2.f,7.5f);
  float_4 envPhasor[4]{},
          stage[4]{},
          relStart[4]{},
          prevEnvOut[4]{},
          gate[4]{},
          syncRtrg[4]{},
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
    configSwitch<FixedSwitchQuantity>(XM_TYPE_PARAM, 0.f, 6.f, 0.f, "X-Mod type", {"Frequency modulation - AC coupled", "Frequency modulation - DC coupled", "Phase modulation", "Ring modulation", "Amplitude modulation", "Rectified amplitude modulation", "Offset amplitude modulation"});
    configSwitch<FixedSwitchQuantity>(FDBK_TYPE_PARAM, 0.f, 6.f, 0.f, "Feedback type", {"Frequency modulation - AC coupled", "Frequency modulation - DC coupled", "Phase modulation", "Ring modulation", "Amplitude modulation", "Rectified amplitude modulation", "Offset amplitude modulation"});
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
    configInput(SMOD_INPUT, "envelope Stage modulation");

    configParam(MULT_CV_PARAM, -1.f, 1.f, 0.f, "VCO frequency multiplier mod amount", "%", 0.f, 100.f);
    configParam(DIV_CV_PARAM, -1.f, 1.f, 0.f, "VCO frequency divisor mod amount", "%", 0.f, 100.f);
    configParam(DTUNE_CV_PARAM, -1.f, 1.f, 0.f, "VCO detune mod amount", "%", 0.f, 100.f);
    configInput(RMOD_INPUT, "frequency Ratio modulation");

    configSwitch<FixedSwitchQuantity>(LEVEL_ENV_PARAM, 0.f, 6.f, 0.f, "Level envelope", {"Off", "Knob", "CV", "Both", "Knob Inverted", "CV Inverted", "Both Inverted"});
    configSwitch<FixedSwitchQuantity>(DEPTH_ENV_PARAM, 0.f, 6.f, 0.f, "X-Mod depth envelope", {"Off", "Knob", "CV", "Both", "Knob Inverted", "CV Inverted", "Both Inverted"});
    configSwitch<FixedSwitchQuantity>(FDBK_ENV_PARAM, 0.f, 6.f, 0.f, "Feedback depth envelope", {"Off", "Knob", "CV", "Both", "Knob Inverted", "CV Inverted", "Both Inverted"});

    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 2.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});

    configParam(LEVEL_PARAM, 0.f, 1.f, 1.f, "VCA Level", "%", 0.f, 100.f);
    configParam(DEPTH_PARAM, -1.f, 1.f, 0.f, "VCO X-Mod depth", "%", 0.f, 100.f);
    configParam(FDBK_PARAM, -1.f, 1.f, 0.f, "VCO Feedback depth", "%", 0.f, 100.f);

    configParam(LEVEL_CV_PARAM, -1.f, 1.f, 0.f, cvAmtParamNames[0], "%", 0.f, 100.f);
    configParam(DEPTH_CV_PARAM, -1.f, 1.f, 0.f, cvAmtParamNames[1], "%", 0.f, 100.f);
    configParam(FDBK_CV_PARAM, -1.f, 1.f, 0.f, cvAmtParamNames[2], "%", 0.f, 100.f);
    configSwitch<FixedSwitchQuantity>(SYNC_RTRG_MODE_PARAM, 0.f, 2.f, 0.f, "Sync/Rtrg input mode", {"VCO sync", "Envelope retrigger and VCO sync (Gate also syncs)", "Envelope retrigger, no VCO sync"});
    configInput(SYNC_RTRG_INPUT, "VCO sync");

    configInput(LEVEL_INPUT, cvAmtPortNames[0]);
    configInput(DEPTH_INPUT, cvAmtPortNames[1]);
    configInput(FDBK_INPUT, cvAmtPortNames[2]);
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
  
  void toggleVelocity(int port, int param, bool &velo) {
    velo = !velo;
    inputInfos[port]->name = velo ? veloPortNames[port - LEVEL_INPUT] : cvAmtPortNames[port - LEVEL_INPUT];
    paramQuantities[param]->name = velo ? veloParamNames[param - LEVEL_CV_PARAM] : cvAmtParamNames[param - LEVEL_CV_PARAM];
    paramQuantities[param]->displayMultiplier = velo ? 50.f : 100.f;
    paramQuantities[param]->displayOffset = velo ? 50.f : 0.f;
  }
  
  void computeVal(float_4 &var, int envMode, float_4 &env, float paramVal, float cvAmt, int cvPort, int channel) {
    switch (envMode) {
      case 0: // off
        var = paramVal + cvAmt * inputs[cvPort].getPolyVoltageSimd<float_4>(channel)/10.f;
        break;
      case 1: // knob only
        var = (paramVal * env) + (cvAmt * inputs[cvPort].getPolyVoltageSimd<float_4>(channel)/10.f);
        break;
      case 2: // CV
        var = paramVal + (cvAmt * inputs[cvPort].getPolyVoltageSimd<float_4>(channel)/10.f * env);
        break;
      case 3: // both
        var = (paramVal + cvAmt * inputs[cvPort].getPolyVoltageSimd<float_4>(channel)/10.f) * env;
        break;
      case 4: // knob inverted
        var = (paramVal * (1.f - env)) + (cvAmt * inputs[cvPort].getPolyVoltageSimd<float_4>(channel)/10.f);
        break;
      case 5: // CV inverted
        var = paramVal + (cvAmt * inputs[cvPort].getPolyVoltageSimd<float_4>(channel)/10.f * (1.f - env));
        break;
      case 6: // both inverted
        var = (paramVal + cvAmt * inputs[cvPort].getPolyVoltageSimd<float_4>(channel)/10.f) * (1.f - env);
        break;
    }
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
    
    if (params[SYNC_RTRG_MODE_PARAM].getValue() != syncRtrgMode) {
      syncRtrgMode = params[SYNC_RTRG_MODE_PARAM].getValue();
      switch(syncRtrgMode) {
        case 0:
          if (inputInfos[SYNC_RTRG_INPUT]->name == inputExtensions[SYNC_RTRG_INPUT].factoryName)
            inputInfos[SYNC_RTRG_INPUT]->name = "VCO sync";
          inputExtensions[SYNC_RTRG_INPUT].factoryName = "VCO sync";
          if (inputInfos[GATE_INPUT]->name == inputExtensions[GATE_INPUT].factoryName)
            inputInfos[GATE_INPUT]->name = "Envelope gate";
          inputExtensions[GATE_INPUT].factoryName = "Envelope gate";
          break;
        case 1:
          if (inputInfos[SYNC_RTRG_INPUT]->name == inputExtensions[SYNC_RTRG_INPUT].factoryName)
            inputInfos[SYNC_RTRG_INPUT]->name = "Envelope retrigger and VCO sync";
          inputExtensions[SYNC_RTRG_INPUT].factoryName = "Envelope retrigger and VCO sync";
          if (inputInfos[GATE_INPUT]->name == inputExtensions[GATE_INPUT].factoryName)
            inputInfos[GATE_INPUT]->name = "Envelope gate and VCO sync";
          inputExtensions[GATE_INPUT].factoryName = "Envelope gate and VCO sync";
          break;
        default: //2
          if (inputInfos[SYNC_RTRG_INPUT]->name == inputExtensions[SYNC_RTRG_INPUT].factoryName)
            inputInfos[SYNC_RTRG_INPUT]->name = "Envelope retrigger";
          inputExtensions[SYNC_RTRG_INPUT].factoryName = "Envelope retrigger";
          if (inputInfos[GATE_INPUT]->name == inputExtensions[GATE_INPUT].factoryName)
            inputInfos[GATE_INPUT]->name = "Envelope gate";
          inputExtensions[GATE_INPUT].factoryName = "Envelope gate";
      }
    }

    int wave = params[WAVE_PARAM].getValue(),
        xmodType = params[XM_TYPE_PARAM].getValue(),
        fdbkType = params[FDBK_TYPE_PARAM].getValue(),
        levelEnv = params[LEVEL_ENV_PARAM].getValue(),
        depthEnv = params[DEPTH_ENV_PARAM].getValue(),
        fdbkEnv = params[FDBK_ENV_PARAM].getValue(),
        invEnv = params[ENV_OUT_PARAM].getValue();
    
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
          dtuneCVAmt = params[DTUNE_CV_PARAM].getValue()/120.f,
          levelParam = params[LEVEL_PARAM].getValue(),
          depthParam = params[DEPTH_PARAM].getValue(),
          fdbkParam = params[FDBK_PARAM].getValue(),
          levelCVAmt = params[LEVEL_CV_PARAM].getValue(),
          depthCVAmt = params[DEPTH_CV_PARAM].getValue(),
          fdbkCVAmt = params[FDBK_CV_PARAM].getValue(),
          k =  1000.f * args.sampleTime / oversample;          

    if (levelVelo != (levelEnv == 3 || levelEnv == 6))
      toggleVelocity(LEVEL_INPUT, LEVEL_CV_PARAM, levelVelo);
    if (depthVelo != (depthEnv == 3 || depthEnv == 6))
      toggleVelocity(DEPTH_INPUT, DEPTH_CV_PARAM, depthVelo);
    if (fdbkVelo != (fdbkEnv == 3 || fdbkEnv == 6))
      toggleVelocity(FDBK_INPUT, FDBK_CV_PARAM, fdbkVelo);
    if (levelVelo)
      levelCVAmt = (levelCVAmt + 1.f)*0.5f;
    if (depthVelo)
      depthCVAmt = (depthCVAmt + 1.f)*0.5f;
    if (fdbkVelo)
      fdbkCVAmt = (fdbkCVAmt + 1.f)*0.5f;

    for (int s=0, c=0; c<channels; s++, c+=4) {
      float_4 rmod = inputs[RMOD_INPUT].getPolyVoltageSimd<float_4>(c),
              smod = inputs[SMOD_INPUT].getPolyVoltageSimd<float_4>(c),
              susLevel = clamp(susParam + smod*susCVAmt),
              delta = ifelse(stage[s]==1.f, args.sampleTime / clamp(pow(2.f, smod*atkCVAmt + atkParam), minTime, maxTime),
                        ifelse(stage[s]==2.f, args.sampleTime / clamp(pow(2.f, smod*decCVAmt + decParam), minTime, maxTime),
                          ifelse(stage[s]==4.f, args.sampleTime / clamp(pow(2.f, smod*relCVAmt + relParam), minTime, maxTime), 0.f)));
      envPhasor[s] = clamp(envPhasor[s]+delta);
      float_4 curve = normSigmoid(envPhasor[s], shape),
              envOut = ifelse(stage[s]<=1.f, curve,
                         ifelse(stage[s]==2.f, 1.f - curve*(1.f - susLevel),
                           ifelse(stage[s]==3.f, susLevel, relStart[s]*(1.f - curve)))),
              gateV = inputs[GATE_INPUT].getPolyVoltageSimd<float_4>(c),
              newGate = ifelse(gateV>2.f, 1.f,
                          ifelse(gateV<0.2f, 0.f, gate[s])),
              syncRtrgV = inputs[SYNC_RTRG_INPUT].getPolyVoltageSimd<float_4>(c),
              newSyncRtrg = ifelse(syncRtrgV>2.f, 1.f,
                              ifelse(syncRtrgV<0.2f, 0.f, syncRtrg[s])),
              newStage = ifelse(newGate>gate[s], 1.f,
                           ifelse(newGate<gate[s], 4.f,
                             ifelse(envPhasor[s]<1.f, stage[s],
                               ifelse(stage[s]==4.f, 0.f, stage[s]+1))));
      if (syncRtrgMode >= 1)
        newStage = ifelse((newStage>=2.f) & (newStage<=3.f) & (newSyncRtrg>syncRtrg[s]), 1.f, newStage);
      envPhasor[s] = ifelse(newStage==stage[s], envPhasor[s],
                       ifelse(newStage==1.f, invNormSigmoid(envOut, shape),
                         ifelse((newStage==4.f) & (envOut<susLevel), invNormSigmoid((susLevel-envOut)/susLevel, shape), 0.f)));
      relStart[s] = ifelse((newStage!=stage[s]) & (newStage==4.f), envOut, relStart[s]);

      float_4 baseFreq = inputs[VOCT_INPUT].getPolyVoltageSimd<float_4>(c)
                         + dtuneParam + rmod*dtuneCVAmt,
              xmod = inputs[XMOD_INPUT].getPolyVoltageSimd<float_4>(c),
              level{},
              depth{},
              fdbk{},
              vcoOut{};
      computeVal(level, levelEnv, envOut, levelParam, levelCVAmt, LEVEL_INPUT, c);
      level = clamp(level);
      computeVal(depth, depthEnv, envOut, depthParam, depthCVAmt, DEPTH_INPUT, c);
      computeVal(fdbk, fdbkEnv, envOut, fdbkParam, fdbkCVAmt, FDBK_INPUT, c);
      for (int i=0; i<4; i++){
        baseFreq[i] += harmonic[clamp(static_cast<int>(multParam + rmod[i]*10.f*multCVAmt),0,MAX_HARMONIC)];
        baseFreq[i] -= harmonic[clamp(static_cast<int>(divParam  + rmod[i]*10.f*divCVAmt ),0,MAX_HARMONIC)];
      }
      baseFreq = dsp::exp2_taylor5(baseFreq);
      for (int o=0; o<oversample; o++) {
        if (oversample>1)
          xmod = upSample[s].process(o ? 0.f : xmod*oversample);
        float_4 freq = baseFreq;
        if (xmodType == 0) // FM AC coupled
          freq += xmodDcBlockFilter[s].process(xmod) * depth;
        if (xmodType == 1) // FM DC coupled
          freq += xmod * depth;
        if (fdbkType == 0) // FM AC coupled
          freq += fdbkDcBlockFilter[s].process(prevVcoOut[s]) * fdbk;
        if (fdbkType == 1) // FM DC coupled
          freq += prevVcoOut[s] * fdbk;
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
        if (o==0 && syncRtrgMode==1)
          vcoPhasor[s] = ifelse((newStage==1.f) & (newStage!=stage[s]), 0.f, vcoPhasor[s]);
        if (o==0 && syncRtrgMode==0)
          vcoPhasor[s] = ifelse(newSyncRtrg>syncRtrg[s], 0.f, vcoPhasor[s]);
        float_4 phases[3]{},
                wavePhasor = vcoPhasor[s],
                sawPhasor{},
                offsetSawPhasor{};
        if (xmodType == 2) // PM
          wavePhasor += xmod * depth * 250.f;
        if (fdbkType == 2) // PM
          wavePhasor += prevVcoOut[s] * fdbk * 62.5f;
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
        if (xmodType == 3) // RM
          vcoOut = crossfade( vcoOut, vcoOut * xmod/ifelse(depth<0.f, -5.f, 5.f), clamp(fabs(depth)));
        if (fdbkType == 3) // RM
          vcoOut = crossfade( vcoOut, vcoOut * prevVcoOut[s]/ifelse(fdbk<0.f, -5.f, 5.f), clamp(fabs(fdbk)));
        if (xmodType == 4) // AM
          vcoOut = crossfade( vcoOut, vcoOut * ifelse((depth<0.f)&(xmod<0.f), -xmod/5.f, ifelse((depth>0.f)&(xmod>0.f), xmod/5.f, 0.f)), clamp(fabs(depth)));
        if (fdbkType == 4) // AM
          vcoOut = crossfade( vcoOut, vcoOut * ifelse((fdbk<0.f)&(prevVcoOut[s]<0.f), -prevVcoOut[s]/5.f, ifelse((fdbk>0.f)&(prevVcoOut[s]>0.f), prevVcoOut[s]/5.f, 0.f)), clamp(fabs(fdbk)));
        if (xmodType == 5) // rect AM
          vcoOut = crossfade( vcoOut, vcoOut * ifelse(depth<0.f, -1.f, 1.f)*fabs(xmod)/5.f, clamp(fabs(depth)));
        if (fdbkType == 5) // rect AM
          vcoOut = crossfade( vcoOut, vcoOut * ifelse(fdbk<0.f, -1.f, 1.f)*fabs(prevVcoOut[s])/5.f, clamp(fabs(fdbk)));
        if (xmodType == 6) // offset AM
          vcoOut = crossfade( vcoOut, vcoOut * clamp(ifelse(depth<0.f, -xmod, xmod)+5.f, 0.f, 100.f)/10.f, clamp(fabs(depth)));
        if (fdbkType == 6) // offset AM
          vcoOut = crossfade( vcoOut, vcoOut * clamp(ifelse(fdbk<0.f, -prevVcoOut[s], prevVcoOut[s])+5.f, 0.f, 100.f)/10.f, clamp(fabs(fdbk)));
        if (oversample>1)
          vcoOut = downSample[s].process(vcoOut);
        prevVcoOut[s] = saveVcoOut;
      }
      vcoOut *= level;
      gate[s] = newGate;
      syncRtrg[s] = newSyncRtrg;
      stage[s] = newStage;
      outputs[ENV_OUTPUT].setVoltageSimd((invEnv ? 1.f-envOut : envOut)*10.f, c);
      outputs[OSC_OUTPUT].setVoltageSimd(vcoOut, c);
    }
    outputs[ENV_OUTPUT].setChannels(channels);
    outputs[OSC_OUTPUT].setChannels(channels);
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "veloAvail", json_boolean(true));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    int val;
    if (!json_object_get(rootJ, "veloAvail")){
      if ((val=params[XM_TYPE_PARAM].getValue()))
        params[XM_TYPE_PARAM].setValue(val+1);
      if ((val=params[FDBK_TYPE_PARAM].getValue()))
        params[FDBK_TYPE_PARAM].setValue(val+1);
      if ((val=params[LEVEL_ENV_PARAM].getValue()>1))
        params[LEVEL_ENV_PARAM].setValue(val+2);
      if ((val=params[DEPTH_ENV_PARAM].getValue()>1))
        params[DEPTH_ENV_PARAM].setValue(val+2);
      if ((val=params[FDBK_ENV_PARAM].getValue()>1))
        params[FDBK_ENV_PARAM].setValue(val+2);
    }
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
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/xm_FM_DC.svg")));
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
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
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
  
  struct SyncRtrgSwitch : GlowingSvgSwitchLockable {
    SyncRtrgSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
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
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(127.5f,48.5f), module, XM_OP::CURVE_PARAM));

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
    addInput(createInputCentered<PolyPort>(Vec(127.5f, 153.f), module, XM_OP::SMOD_INPUT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(22.5f,188.5f), module, XM_OP::MULT_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(57.5f,188.5f), module, XM_OP::DIV_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(92.5f,188.5f), module, XM_OP::DTUNE_CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(127.5f, 188.5f), module, XM_OP::RMOD_INPUT));

    addParam(createLockableParamCentered<EnvSwitch>(Vec(22.5f,207.5f), module, XM_OP::LEVEL_ENV_PARAM));
    addParam(createLockableParamCentered<EnvSwitch>(Vec(57.5f,207.5f), module, XM_OP::DEPTH_ENV_PARAM));
    addParam(createLockableParamCentered<EnvSwitch>(Vec(92.5f,207.5f), module, XM_OP::FDBK_ENV_PARAM));

    addParam(createLockableParamCentered<OverSwitch>(Vec(127.5f,228.f), module, XM_OP::OVER_PARAM));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f,235.f), module, XM_OP::LEVEL_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(57.5f,235.f), module, XM_OP::DEPTH_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(92.5f,235.f), module, XM_OP::FDBK_PARAM));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(22.5f,266.5f), module, XM_OP::LEVEL_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(57.5f,266.5f), module, XM_OP::DEPTH_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(92.5f,266.5f), module, XM_OP::FDBK_CV_PARAM));
    addParam(createLockableParamCentered<SyncRtrgSwitch>(Vec(109.5f,256.5f), module, XM_OP::SYNC_RTRG_MODE_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(127.5f,264.f), module, XM_OP::SYNC_RTRG_INPUT));

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

};

}

Model* modelVenomXM_OP = createModel<Venom::XM_OP, Venom::XM_OPWidget>("XM_OP");

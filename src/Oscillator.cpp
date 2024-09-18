// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"
#include <float.h>

#define GRID_PARAM  SIN_SHAPE_PARAM
#define ASGN_PARAM  SIN_ASIGN_PARAM
#define GRID_INPUT  SIN_SHAPE_INPUT
#define GRID_LIGHT  SIN_SHAPE_LIGHT
#define GRID_OUTPUT SIN_OUTPUT
#define SIN 0
#define TRI 1
#define SQR 2
#define SAW 3
#define MIX 4
#define LINFM 5

struct Oscillator : VenomModule {
 
  enum ParamId {
    MODE_PARAM,
    OVER_PARAM,
    PW_PARAM,
    MIXSHP_PARAM,
    DC_PARAM,
    FREQ_PARAM,
    OCTAVE_PARAM,
    RESET_POLY_PARAM,
    EXP_PARAM,
    LIN_PARAM,
    
    SIN_SHAPE_PARAM,
    TRI_SHAPE_PARAM,
    SQR_SHAPE_PARAM,
    SAW_SHAPE_PARAM,
    MIX_SHAPE_PARAM,
    SIN_SHAPE_AMT_PARAM,
    TRI_SHAPE_AMT_PARAM,
    SQR_SHAPE_AMT_PARAM,
    SAW_SHAPE_AMT_PARAM,
    MIX_SHAPE_AMT_PARAM,

    SIN_PHASE_PARAM,
    TRI_PHASE_PARAM,
    SQR_PHASE_PARAM,
    SAW_PHASE_PARAM,
    MIX_PHASE_PARAM,
    SIN_PHASE_AMT_PARAM,
    TRI_PHASE_AMT_PARAM,
    SQR_PHASE_AMT_PARAM,
    SAW_PHASE_AMT_PARAM,
    MIX_PHASE_AMT_PARAM,

    SIN_OFFSET_PARAM,
    TRI_OFFSET_PARAM,
    SQR_OFFSET_PARAM,
    SAW_OFFSET_PARAM,
    MIX_OFFSET_PARAM,
    SIN_OFFSET_AMT_PARAM,
    TRI_OFFSET_AMT_PARAM,
    SQR_OFFSET_AMT_PARAM,
    SAW_OFFSET_AMT_PARAM,
    MIX_OFFSET_AMT_PARAM,

    SIN_LEVEL_PARAM,
    TRI_LEVEL_PARAM,
    SQR_LEVEL_PARAM,
    SAW_LEVEL_PARAM,
    MIX_LEVEL_PARAM,
    SIN_LEVEL_AMT_PARAM,
    TRI_LEVEL_AMT_PARAM,
    SQR_LEVEL_AMT_PARAM,
    SAW_LEVEL_AMT_PARAM,
    MIX_LEVEL_AMT_PARAM,

    SIN_ASIGN_PARAM,
    TRI_ASIGN_PARAM,
    SQR_ASIGN_PARAM,
    SAW_ASIGN_PARAM,
    
    SINSHP_PARAM,
    TRISHP_PARAM,
    SAWSHP_PARAM,

    PARAMS_LEN
  };
  
  enum InputId {
    EXP_INPUT,
    LIN_INPUT,
    EXP_DEPTH_INPUT,
    LIN_DEPTH_INPUT,
    VOCT_INPUT,
    SYNC_INPUT,
    
    SIN_SHAPE_INPUT,
    TRI_SHAPE_INPUT,
    SQR_SHAPE_INPUT,
    SAW_SHAPE_INPUT,
    MIX_SHAPE_INPUT,
    
    SIN_PHASE_INPUT,
    TRI_PHASE_INPUT,
    SQR_PHASE_INPUT,
    SAW_PHASE_INPUT,
    MIX_PHASE_INPUT,
    
    SIN_OFFSET_INPUT,
    TRI_OFFSET_INPUT,
    SQR_OFFSET_INPUT,
    SAW_OFFSET_INPUT,
    MIX_OFFSET_INPUT,
    
    SIN_LEVEL_INPUT,
    TRI_LEVEL_INPUT,
    SQR_LEVEL_INPUT,
    SAW_LEVEL_INPUT,
    MIX_LEVEL_INPUT,

    REV_INPUT,

    INPUTS_LEN
  };
  enum OutputId {
    SIN_OUTPUT,
    TRI_OUTPUT,
    SQR_OUTPUT,
    SAW_OUTPUT,
    MIX_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(REV_LIGHT,2),
    ENUMS(EXP_LIGHT,2),
    ENUMS(LIN_LIGHT,2),
    ENUMS(SYNC_LIGHT,2),
    
    ENUMS(SIN_SHAPE_LIGHT,2),
    ENUMS(TRI_SHAPE_LIGHT,2),
    ENUMS(SQR_SHAPE_LIGHT,2),
    ENUMS(SAW_SHAPE_LIGHT,2),
    ENUMS(MIX_SHAPE_LIGHT,2),

    ENUMS(SIN_PHASE_LIGHT,2),
    ENUMS(TRI_PHASE_LIGHT,2),
    ENUMS(SQR_PHASE_LIGHT,2),
    ENUMS(SAW_PHASE_LIGHT,2),
    ENUMS(MIX_PHASE_LIGHT,2),

    ENUMS(SIN_OFFSET_LIGHT,2),
    ENUMS(TRI_OFFSET_LIGHT,2),
    ENUMS(SQR_OFFSET_LIGHT,2),
    ENUMS(SAW_OFFSET_LIGHT,2),
    ENUMS(MIX_OFFSET_LIGHT,2),

    ENUMS(SIN_LEVEL_LIGHT,2),
    ENUMS(TRI_LEVEL_LIGHT,2),
    ENUMS(SQR_LEVEL_LIGHT,2),
    ENUMS(SAW_LEVEL_LIGHT,2),
    ENUMS(MIX_LEVEL_LIGHT,2),
    
    SIN_RM_LIGHT,
    TRI_RM_LIGHT,
    SQR_RM_LIGHT,
    SAW_RM_LIGHT,
    MIX_RM_LIGHT,
    
    LIN_DC_LIGHT,
    
    SIN_VCA_LIGHT,
    TRI_VCA_LIGHT,
    SQR_VCA_LIGHT,
    SAW_VCA_LIGHT,
    MIX_VCA_LIGHT,

    LIGHTS_LEN
  };

  bool clampLevel = true;
  bool disableOver[INPUTS_LEN]{};
  bool unity5[5]{};
  bool bipolar[5]{};
  bool oldShpCV[4]{};
  float lvlScale[5]{0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
  float shpScale[4]{0.2f, 0.2f, 0.2f, 0.2f};
  bool softSync = false;
  bool alternate = false;
  bool lfo = false;
  using float_4 = simd::float_4;
  int oversample = -1;
  std::vector<int> oversampleValues = {1,2,4,8,16,32};
  OversampleFilter_4 expUpSample[4]{}, linUpSample[4]{}, revUpSample[4]{}, syncUpSample[4]{},
                     shapeUpSample[4][5]{}, phaseUpSample[4][5]{}, offsetUpSample[4][5]{}, levelUpSample[4][5]{},
                     outDownSample[4][5]{};
  float_4 phasor[4]{}, phasorDir[4]{1.f, 1.f, 1.f, 1.f};
  DCBlockFilter_4 dcBlockFilter[4][6]{}; // Sin, Tri, Sqr, Saw, Mix, Lin FM Input
  bool linDCCouple = false;
  dsp::SchmittTrigger syncTrig[16], revTrig[16];
  float modeFreq[3] = {dsp::FREQ_C4, 2.f, 100.f}, biasFreq = 0.02f;
  int currentMode = 0;
  int mode = 0;
  bool once = false;
  bool noRetrigger = false;
  bool gated = false;
  float_4 onceActive[4]{};
  int modeDefaultOver[3] = {2, 0, 2};
  static constexpr float maxFreq = 12000.f;
  
  struct PWQuantity : ParamQuantity {
    float getDisplayValue() override {
      float val = ParamQuantity::getDisplayValue();
      if (!(module->params[PW_PARAM].getValue()))
        val = clamp(val, 3.f, 97.f);
      return val;
    }
  };

  struct FreqQuantity : ParamQuantity {
    float getDisplayValue() override {
      Oscillator* mod = reinterpret_cast<Oscillator*>(this->module);
      float freq = 0.f;
      if (mod->mode < 2)
        freq = pow(2.f, mod->params[FREQ_PARAM].getValue() + mod->params[OCTAVE_PARAM].getValue()) * mod->modeFreq[mod->mode];
      else
        freq = mod->params[FREQ_PARAM].getValue() * mod->biasFreq;
      return freq < maxFreq ? freq : maxFreq;
    }
    void setDisplayValue(float v) override {
      Oscillator* mod = reinterpret_cast<Oscillator*>(this->module);
      if (v > maxFreq) v = maxFreq;
      if (mod->mode < 2)
        setValue(clamp(std::log2f(v / mod->modeFreq[mod->mode]) - mod->params[OCTAVE_PARAM].getValue(), -4.f, 4.f));
      else
        setValue(clamp(v / mod->biasFreq, -4.f, 4.f));
    }
  };


  Oscillator() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 7.f, 0.f, "Frequency Mode", {"Audio frequency", "Low frequency", "0Hz carrier", "Triggered audio one shot", "Retriggered audio one shot", "Gated audio one shot", "Retriggered LFO one shot", "Gated LFO one shot"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 2.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    configSwitch<FixedSwitchQuantity>(DC_PARAM,   0.f, 1.f, 0.f, "DC Block", {"Off", "On"});
    configButton(RESET_POLY_PARAM, "Reset polyphony count");

    configSwitch<FixedSwitchQuantity>(SINSHP_PARAM, 0.f, 4.f, 0.f, "Sine Shape Mode", {"log/exp", "J-curve", "S-curve", "Rectify", "Normalized Rectify"});
    configSwitch<FixedSwitchQuantity>(TRISHP_PARAM, 0.f, 5.f, 0.f, "Triangle Shape Mode", {"log/exp", "J-curve", "S-curve", "Rectify", "Normalized Rectify", "Morph SIN <--> TRI <--> SQR"});
    configSwitch<FixedSwitchQuantity>(PW_PARAM, 0.f, 2.f, 0.f, "Square Shape Mode", {"Limited PWM 3%-97%", "Full PWM 0%-100%", "Morph TRI <--> SQR <--> SAW"});
    configSwitch<FixedSwitchQuantity>(SAWSHP_PARAM, 0.f, 5.f, 0.f, "Saw Shape Mode", {"log/exp", "J-curve", "S-curve", "Rectify", "Normalized Rectify", "Morph SQR <--> SAW <--> EVEN"});
    configSwitch<FixedSwitchQuantity>(MIXSHP_PARAM, 0.f, 5.f, 0.f, "Mix Shape Mode", {"Sum (No shaping)", "Saturate Sum", "Fold Sum", "Average (No shaping)", "Saturate Average", "Fold Average"});

    
    configParam<FreqQuantity>(FREQ_PARAM, -4.f, 4.f, 0.f, "Frequency", " Hz");
    configParam(OCTAVE_PARAM, -4.f, 4.f, 0.f, "Octave");
    configLight(REV_LIGHT, "Soft sync oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";
    configInput(REV_INPUT, "Soft sync");
    configParam(EXP_PARAM, -1.f, 1.f, 0.f, "Exponential FM", "%", 0.f, 100.f);
    configParam(LIN_PARAM, -1.f, 1.f, 0.f, "Linear FM", "%", 0.f, 100.f);
    configInput(EXP_INPUT, "Exponential FM");
    configLight(EXP_LIGHT, "Exponential FM oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";
    configInput(LIN_INPUT, "Linear FM");
    configLight(LIN_LIGHT, "Linear FM oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";
    configLight(LIN_DC_LIGHT, "Linear FM DC coupled indicator");
    configInput(EXP_DEPTH_INPUT, "Exponential FM depth");
    configInput(LIN_DEPTH_INPUT, "Linear FM depth");
    configInput(VOCT_INPUT, "V/Oct");
    configInput(SYNC_INPUT, "Sync");
    configLight(SYNC_LIGHT, "Sync oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";

    std::string xStr[5]{"Sine","Triangle","Square","Saw","Mix"};
    std::string yStr[4]{" shape"," phase"," offset"," level"};
    for (int y=0; y<4; y++){
      for (int x=0; x<5; x++){
        switch (y) {
          case 0: // shape
            switch (x) {
              case SQR:
                configParam<PWQuantity>(GRID_PARAM+y*10+x, -1.f, 1.f, 0.f, xStr[x]+yStr[y], "%", 0.f, 50.f, 50.f);
                break;
              case MIX:
                configParam(GRID_PARAM+y*10+x, -1.f, 1.f, -1.f, xStr[x]+yStr[y], "%", 0.f, 50.f, 50.f);
                break;
              default:
                configParam(GRID_PARAM+y*10+x, -1.f, 1.f, 0.f, xStr[x]+yStr[y], "%", 0.f, 100.f);
            }
            break;
          case 1: // phase
            configParam(GRID_PARAM+y*10+x, -1.f, 1.f, 0.f, ((x==4)?"Global":xStr[x])+yStr[y], "\u00B0", 0.f, 180.f);
            break;
          case 2: // offset
            configParam(GRID_PARAM+y*10+x, -1.f, 1.f, 0.f, xStr[x]+yStr[y], " V", 0.f, 5.f);
            break;
          case 3: // level
            configParam(GRID_PARAM+y*10+x, -1.f, 1.f, 0.f, xStr[x]+yStr[y], "%", 0.f, 100.f);
            configLight(SIN_RM_LIGHT+x, xStr[x]+" VCA unity = 5V indicator");
            configLight(SIN_VCA_LIGHT+x, xStr[x]+" Bipolar VCA indicator");
            break;
        }
        configParam(GRID_PARAM+y*10+x+5, -1.f, 1.f, 0.f, ((x==4&&y==1)?"Global":xStr[x])+yStr[y]+" CV amount", "%", 0.f, 100.f);
        configInput(GRID_INPUT+y*5+x, ((x==4&&y==1)?"Global":xStr[x])+yStr[y]+" CV");
        configLight(GRID_LIGHT+y*10+x*2, ((x==4&&y==1)?"Global":xStr[x])+yStr[y]+" oversample indicator")->description = "off = N/A, yellow = oversampled, red = disabled";
      }
    }
    for (int x=0; x<4; x++){
      configSwitch<FixedSwitchQuantity>(ASGN_PARAM+x, 0.f, 2.f, 0.f, xStr[x]+" level assignment", {"Mix output", xStr[x]+" output", "Both "+xStr[x]+" and Mix output"});
    }
    for (int x=0; x<5; x++){
      configOutput(GRID_OUTPUT+x, xStr[x]);
    }
  }

  float_4 sinSimd_1000(float_4 t) {
    t = simd::ifelse(t > 500.f, 1000.f - t, t) * 0.002f - 0.5f;
    float_4 t2 = t * t;
    return -(((-0.540347 * t2 + 2.53566) * t2 - 5.16651) * t2 + 3.14159) * t;
  }
  
  void setMode() {
    currentMode = static_cast<int>(params[MODE_PARAM].getValue());
    mode = currentMode>5 ? 1 : currentMode>2 ? 0 : currentMode;
    if (!paramExtensions[OVER_PARAM].locked)
      params[OVER_PARAM].setValue(modeDefaultOver[mode]);
    paramQuantities[OVER_PARAM]->defaultValue = modeDefaultOver[mode];
    paramExtensions[OVER_PARAM].factoryDflt = modeDefaultOver[mode];
    once = (currentMode>2);
    noRetrigger = (currentMode==3);
    gated = (currentMode==5) || (currentMode==7);
    for (int i=0; i<4; i++) onceActive[i] = float_4::zero();
  }

  void setUnity5(int indx, bool rm) {
    unity5[indx] = rm;
    lvlScale[indx] = rm ? 0.2f : 0.1f;
    lights[Oscillator::SIN_RM_LIGHT+indx].setBrightness(rm);
  }
  
  void setBipolar(int indx, bool val) {
    bipolar[indx] = val;
    lights[Oscillator::SIN_VCA_LIGHT+indx].setBrightness(val);
  }

  void setOldShpCV(int indx, bool val) {
    oldShpCV[indx] = val;
    shpScale[indx] = val ? 0.1f : 0.2f;
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    if (currentMode != static_cast<int>(params[MODE_PARAM].getValue())) {
      setMode();
    }

    if (oversample != oversampleValues[params[OVER_PARAM].getValue()]) {
      oversample = oversampleValues[params[OVER_PARAM].getValue()];
      for (int i=0; i<4; i++){
        expUpSample[i].setOversample(oversample);
        linUpSample[i].setOversample(oversample);
        revUpSample[i].setOversample(oversample);
        syncUpSample[i].setOversample(oversample);
        for (int j=0; j<5; j++){
          shapeUpSample[i][j].setOversample(oversample);
          phaseUpSample[i][j].setOversample(oversample);
          offsetUpSample[i][j].setOversample(oversample);
          levelUpSample[i][j].setOversample(oversample);
          outDownSample[i][j].setOversample(oversample);
        }
      }
    }
    // get channel count
    int channels = 1;
    if (!params[RESET_POLY_PARAM].getValue()){
      for (int i=0; i<INPUTS_LEN; i++) {
        int c = inputs[i].getChannels();
        if (c>channels)
          channels = c;
      }
    }
    int simdCnt = (channels+3)/4;
    
    float_4 expIn{}, linIn{}, expDepthIn[4]{}, linDepthIn[4]{}, vOctIn[4]{}, revIn{}, syncIn{}, freq[4]{},
            shapeIn[5]{}, phaseIn[5]{}, offsetIn[5]{}, levelIn[5]{},
            sinOut[4]{}, triOut[4]{}, sqrOut[4]{}, sawOut[4]{}, mixOut[4]{},
            sinPhasor{}, triPhasor{}, sqrPhasor{}, sawPhasor{}, globalPhasor{},
            shapeSign{};
    float vOctParm = mode<2 ? params[FREQ_PARAM].getValue() + params[OCTAVE_PARAM].getValue() : params[FREQ_PARAM].getValue();
    float k =  1000.f * args.sampleTime / oversample;
    
    if (alternate != (mode==2)) {
      alternate = !alternate;
      paramQuantities[FREQ_PARAM]->name = alternate ? "Bias" : "Frequency";
      paramQuantities[OCTAVE_PARAM]->name = alternate ? "Linear FM range" : "Octave";
      inputInfos[VOCT_INPUT]->name = alternate ? "Bias" : "V/Oct";
      paramQuantities[EXP_PARAM]->name = alternate ? "Unused" : "Exponential FM";
      inputInfos[EXP_INPUT]->name = alternate ? "Unused" : "Exponential FM";
      inputInfos[EXP_DEPTH_INPUT]->name = alternate ? "Unused" : "Exponential FM depth";

      paramExtensions[FREQ_PARAM].factoryName = paramQuantities[FREQ_PARAM]->name;
      paramExtensions[OCTAVE_PARAM].factoryName = paramQuantities[OCTAVE_PARAM]->name;
      inputExtensions[VOCT_INPUT].factoryName = inputInfos[VOCT_INPUT]->name;
      paramExtensions[EXP_PARAM].factoryName = paramQuantities[EXP_PARAM]->name;
      inputExtensions[EXP_INPUT].factoryName = inputInfos[EXP_INPUT]->name;
      inputExtensions[EXP_DEPTH_INPUT].factoryName = inputInfos[EXP_DEPTH_INPUT]->name;
    }
    
    if (softSync != inputs[REV_INPUT].isConnected()) {
      if (softSync) {
        for (int i=0; i<4; i++) phasorDir[i] = 1.f;
      }
      softSync = !softSync;
    }
    
    int sinMode = static_cast<int>(params[SINSHP_PARAM].getValue());
    int triMode = static_cast<int>(params[TRISHP_PARAM].getValue());
    int sqrMode = static_cast<int>(params[PW_PARAM].getValue());
    int sawMode = static_cast<int>(params[SAWSHP_PARAM].getValue());
    int mixMode = static_cast<int>(params[MIXSHP_PARAM].getValue());
    int mixType = mixMode % 3;
    
    bool procSin = outputs[SIN_OUTPUT].isConnected() || (outputs[MIX_OUTPUT].isConnected() && params[SIN_ASIGN_PARAM].getValue() != 1.f);
    bool procTri = outputs[TRI_OUTPUT].isConnected() || (outputs[MIX_OUTPUT].isConnected() && params[TRI_ASIGN_PARAM].getValue() != 1.f);
    bool procSqr = outputs[SQR_OUTPUT].isConnected() || (outputs[MIX_OUTPUT].isConnected() && params[SQR_ASIGN_PARAM].getValue() != 1.f);
    bool procSaw = outputs[SAW_OUTPUT].isConnected() || (outputs[MIX_OUTPUT].isConnected() && params[SAW_ASIGN_PARAM].getValue() != 1.f);
    bool procMix = outputs[MIX_OUTPUT].isConnected();
    bool procOver[INPUTS_LEN]{};
    for (int i=0; i<INPUTS_LEN; i++)
      procOver[i] = oversample>1 && inputs[i].isConnected() && !disableOver[i];
    // main loops
    for (int o=0; o<oversample; o++){
      for (int s=0, c=0; s<simdCnt; s++, c+=4){
        float_4 level{}, mixDiv{};
        // Main Phasor
        if (!o) {
          if (!alternate) {
            if (s==0 || inputs[EXP_DEPTH_INPUT].isPolyphonic()) {
              expDepthIn[s] = simd::clamp( inputs[EXP_DEPTH_INPUT].getNormalPolyVoltageSimd<float_4>(5.f,c)/5.f, -1.f, 1.f);
            } else expDepthIn[s] = expDepthIn[0];
          }
          if (s==0 || inputs[LIN_DEPTH_INPUT].isPolyphonic()) {
            linDepthIn[s] = simd::clamp( inputs[LIN_DEPTH_INPUT].getNormalPolyVoltageSimd<float_4>(5.f,c)/5.f, -1.f, 1.f);
          } else linDepthIn[s] = linDepthIn[0];
          if (s==0 || inputs[VOCT_INPUT].isPolyphonic()) {
            vOctIn[s] = inputs[VOCT_INPUT].getPolyVoltageSimd<float_4>(c);
          } else vOctIn[s] = vOctIn[0];
        }
        if (!alternate) {
          if (s==0 || inputs[EXP_INPUT].isPolyphonic()) {
            expIn = (o && !disableOver[EXP_INPUT]) ? float_4::zero() : inputs[EXP_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[EXP_INPUT]){
              if (o==0) expIn *= oversample;
              expIn = expUpSample[s].process(expIn);
            }
          } // else preserve prior expIn value
        }
        if (s==0 || inputs[LIN_INPUT].isPolyphonic()) {
          linIn = (o && !disableOver[LIN_INPUT]) ? float_4::zero() : inputs[LIN_INPUT].getPolyVoltageSimd<float_4>(c);
          if (procOver[LIN_INPUT]){
            if (o==0) linIn *= oversample;
            linIn = linUpSample[s].process(linIn);
          }
        } // else preserve prior linIn value
        if (inputs[LIN_INPUT].isConnected() && !linDCCouple)
          linIn = dcBlockFilter[s][LINFM].process(linIn/*, oversample*/);
        if (s==0 || inputs[MIX_PHASE_INPUT].isPolyphonic()) {
          phaseIn[MIX] = (o && !disableOver[MIX_PHASE_INPUT]) ? float_4::zero() : inputs[MIX_PHASE_INPUT].getPolyVoltageSimd<float_4>(c);
          if (procOver[MIX_PHASE_INPUT]){
            if (o==0) phaseIn[MIX] *= oversample;
            phaseIn[MIX] = phaseUpSample[s][MIX].process(phaseIn[MIX]);
          }
        } // else preserve prior phaseIn[MIX] value
        float_4 rev{};
        if (inputs[REV_INPUT].isConnected()) {
          if (s==0 || inputs[REV_INPUT].isPolyphonic()) {
            revIn = (o && !disableOver[REV_INPUT]) ? float_4::zero() : inputs[REV_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[REV_INPUT]){
              if (o==0) revIn *= oversample;
              revIn = revUpSample[s].process(revIn);
            }
          } // else preserve prior value
          for (int i=0; i<4; i++){
            rev[i] = revTrig[c+i].process(revIn[i], 0.2f, 2.f);
          }
        }
        float_4 sync{};
        if (inputs[SYNC_INPUT].isConnected()) {
          if (s==0 || inputs[SYNC_INPUT].isPolyphonic()) {
            syncIn = (o && !disableOver[SYNC_INPUT]) ? float_4::zero() : inputs[SYNC_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SYNC_INPUT]){
              if (o==0) syncIn *= oversample;
              syncIn = syncUpSample[s].process(syncIn);
            }
          } // else preserve prior syncIn value
          for (int i=0; i<4; i++){
            sync[i] = syncTrig[c+i].process(syncIn[i], 0.2f, 2.f) && !(noRetrigger && onceActive[s][i]);
          }
        } else onceActive[s] = float_4::zero();
        if (!alternate) {
          freq[s] = vOctIn[s] + vOctParm + expIn*expDepthIn[s]*params[EXP_PARAM].getValue();
          freq[s] = dsp::exp2_taylor5(freq[s]) + linIn*linDepthIn[s]*params[LIN_PARAM].getValue();
        } else {
          freq[s] = (vOctParm + vOctIn[s])*biasFreq + linIn*linDepthIn[s]*params[LIN_PARAM].getValue()*((params[OCTAVE_PARAM].getValue()+4.f)*3.f+1.f);
        }
        freq[s] *= modeFreq[mode];
        freq[s] = simd::ifelse(freq[s] > maxFreq, maxFreq, freq[s]);
        phasorDir[s] = simd::ifelse(rev>0.f, phasorDir[s]*-1.f, phasorDir[s]);
        phasorDir[s] = simd::ifelse(sync>0.f, 1.f, phasorDir[s]);
        phasor[s] += freq[s] * phasorDir[s] * k;
        float_4 tempPhasor = simd::fmod(phasor[s], 1000.f);
        tempPhasor = simd::ifelse(tempPhasor<0.f, tempPhasor+1000.f, tempPhasor);
        if (once)
          onceActive[s] = simd::ifelse(tempPhasor != phasor[s], float_4::zero(), onceActive[s]);
        phasor[s] = tempPhasor;
        phasor[s] = simd::ifelse(sync>0.f, float_4::zero(), phasor[s]);
        if (once)
          onceActive[s] = simd::ifelse(sync>float_4::zero(), 1.f, onceActive[s]);
        if (gated){
          for (int i=0; i<4; i++){
            if (!syncTrig[c+i].isHigh())
              onceActive[s][i] = 0.f;
          }
        }

        // Global (Mix) Phase
        globalPhasor = phasor[s] + (phaseIn[MIX]*params[MIX_PHASE_AMT_PARAM].getValue() + params[MIX_PHASE_PARAM].getValue()*2.f)*250.f;

        mixOut[s] = float_4::zero();

        // Sine
        if (procSin)
        {
          if (s==0 || inputs[SIN_SHAPE_INPUT].isPolyphonic()) {
            shapeIn[SIN] = (o && !disableOver[SIN_SHAPE_INPUT]) ? float_4::zero() : inputs[SIN_SHAPE_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SIN_SHAPE_INPUT]){
              if (o==0) shapeIn[SIN] *= oversample;
              shapeIn[SIN] = shapeUpSample[s][SIN].process(shapeIn[SIN]);
            }
          } // preserve prior shapeIn[SIN] value
          float_4 shape = clamp(shapeIn[SIN]*params[SIN_SHAPE_AMT_PARAM].getValue()*shpScale[SIN] + params[SIN_SHAPE_PARAM].getValue(), -1.f, 1.f);
          if (s==0 || inputs[SIN_PHASE_INPUT].isPolyphonic()) {
            phaseIn[SIN] = (o && !disableOver[SIN_PHASE_INPUT]) ? float_4::zero() : inputs[SIN_PHASE_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SIN_PHASE_INPUT]){
              if (o==0) phaseIn[SIN] *= oversample;
              phaseIn[SIN] = phaseUpSample[s][SIN].process(phaseIn[SIN]);
            }
          } // else preserve prior phaseIn[SIN] value
          sinPhasor = globalPhasor + (phaseIn[SIN]*params[SIN_PHASE_AMT_PARAM].getValue() + params[SIN_PHASE_PARAM].getValue()*2.f)*250.f - 250.f;
          sinPhasor = simd::fmod(sinPhasor, 1000.f);
          sinPhasor = simd::ifelse(sinPhasor<0.f, sinPhasor+1000.f, sinPhasor);
          sinPhasor = sinSimd_1000(sinPhasor);
          switch (sinMode) {
            case 0:  // exp/log
              sinOut[s] = crossfade(sinPhasor, ifelse(shape>0.f, 11.f*sinPhasor/(10.f*simd::abs(sinPhasor)+1.f), simd::sgn(sinPhasor)*simd::pow(sinPhasor,4)), ifelse(shape>0.f, shape, -shape))*5.f;
              break;
            case 1:  // J curve
              sinOut[s] = (normSigmoid((sinPhasor+1.f)/2.f, -shape*0.9f)*2.f-1.f) * 5.f;
              break;
            case 2: // S curve
              sinOut[s] = normSigmoid(sinPhasor, -shape*0.9f) * 5.f;
              break;
            default: // Rectify
              shape = -shape;
              float_4 shapeSign = simd::sgn(shape);
              sinOut[s] = simd::ifelse(shapeSign==0, sinPhasor, -(shapeSign*simd::abs(-sinPhasor+shapeSign-shape)-shapeSign+shape));
              if (sinMode==4 /*Normalized Rectify*/)
                sinOut[s] = -((1+simd::abs(shape))*-sinOut[s]-shape);
              sinOut[s] *= 5.f;
          }

          if (s==0 || inputs[SIN_LEVEL_INPUT].isPolyphonic()) {
            levelIn[SIN] = (o && !disableOver[SIN_LEVEL_INPUT]) ? float_4::zero() : inputs[SIN_LEVEL_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SIN_LEVEL_INPUT]){
              if (o==0) levelIn[SIN] *= oversample;
              levelIn[SIN] = levelUpSample[s][SIN].process(levelIn[SIN]);
            }
          } // else preserve prior levelIn[SIN] value
          level = bipolar[SIN] ? levelIn[SIN] : simd::ifelse(levelIn[SIN]>0.f, levelIn[SIN], 0.f);
          level = level * params[SIN_LEVEL_AMT_PARAM].getValue() * lvlScale[SIN] + params[SIN_LEVEL_PARAM].getValue();
          if (clampLevel)
            level = simd::clamp(level, -1.f, 1.f);
          if (params[SIN_ASIGN_PARAM].getValue()!=1) {
            mixOut[s] += sinOut[s] * level;
            mixDiv += simd::fabs(level);
          }

          if (s==0 || inputs[SIN_OFFSET_INPUT].isPolyphonic()) {
            offsetIn[SIN] = (o && !disableOver[SIN_OFFSET_INPUT]) ? float_4::zero() : inputs[SIN_OFFSET_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SIN_OFFSET_INPUT]){
              if (o==0) offsetIn[SIN] *= oversample;
              offsetIn[SIN] = offsetUpSample[s][SIN].process(offsetIn[SIN]);
            }
          } // else preserve prior offsetIn[SIN] value
          sinOut[s] += clamp(offsetIn[SIN]*params[SIN_OFFSET_AMT_PARAM].getValue() + params[SIN_OFFSET_PARAM].getValue()*5.f, -5.f, 5.f);
          if (params[SIN_ASIGN_PARAM].getValue()!=0)
            sinOut[s] *= level;  
        }
        
        // Triangle
        if (procTri)
        {
          if (s==0 || inputs[TRI_SHAPE_INPUT].isPolyphonic()) {
            shapeIn[TRI] = (o && !disableOver[TRI_SHAPE_INPUT]) ? float_4::zero() : inputs[TRI_SHAPE_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[TRI_SHAPE_INPUT]){
              if (o==0) shapeIn[TRI] *= oversample;
              shapeIn[TRI] = shapeUpSample[s][TRI].process(shapeIn[TRI]);
            }
          } // else preserve prior shapeIn[TRI] value
          float_4 shape = clamp(shapeIn[TRI]*params[TRI_SHAPE_AMT_PARAM].getValue()*shpScale[TRI] + params[TRI_SHAPE_PARAM].getValue(), -1.f, 1.f);
          if (s==0 || inputs[TRI_PHASE_INPUT].isPolyphonic()) {
            phaseIn[TRI] = (o && !disableOver[TRI_PHASE_INPUT]) ? float_4::zero() : inputs[TRI_PHASE_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[TRI_PHASE_INPUT]){
              if (o==0) phaseIn[TRI] *= oversample;
              phaseIn[TRI] = phaseUpSample[s][TRI].process(phaseIn[TRI]);
            }
          } // else preserve prior phaseIn[TRI] value
          triPhasor = globalPhasor + (phaseIn[TRI]*params[TRI_PHASE_AMT_PARAM].getValue() + params[TRI_PHASE_PARAM].getValue()*2.f)*250.f + 250.f;
          triPhasor = simd::fmod(triPhasor, 1000.f);
          triPhasor = simd::ifelse(triPhasor<0.f, triPhasor+1000.f, triPhasor);
          if (triMode<=2) shape = simd::ifelse(triPhasor<500.f, shape, -shape);
          triPhasor = simd::ifelse(triPhasor<500.f, triPhasor*.002f, (1000.f-triPhasor)*.002f);
          switch (triMode) {
            case 0:  // exp/log
              triOut[s] = crossfade(triPhasor, ifelse(shape>0.f, 11.f*triPhasor/(10.f*simd::abs(triPhasor)+1.f), simd::sgn(triPhasor)*simd::pow(triPhasor,4)), ifelse(shape>0.f, shape, -shape))*10.f-5.f;
              break;
            case 1:  // J curve
              triOut[s] = normSigmoid(triPhasor, -shape*0.8) * 10.f - 5.f;
              break;
            case 2: // S curve
              triOut[s] = normSigmoid(triPhasor*2.f-1.f, -shape*0.8) * 5.f;
              break;
            case 3: // Rectify
            case 4: // Normalized Rectify
              shape = -shape;
              shapeSign = simd::sgn(shape);
              triOut[s] = triPhasor*2.f-1.f;
              triOut[s] = simd::ifelse(shapeSign==0, triOut[s], -(shapeSign*simd::abs(-triOut[s]+shapeSign-shape)-shapeSign+shape));
              if (triMode==4 /*Normalized Rectify*/)
                triOut[s] = -((1+simd::abs(shape))*-triOut[s]-shape);
              triOut[s] *= 5.f;
              break;
            default: // 5 morph sine <--> triangle <--> square
              triOut[s] = (triPhasor*10.f - 5.f) * (1.f - simd::abs(shape)); // triangle component
              // sine and square components
              triPhasor = globalPhasor + (phaseIn[TRI]*params[SIN_PHASE_AMT_PARAM].getValue() + params[SIN_PHASE_PARAM].getValue()*2.f)*250.f;
              triPhasor = simd::fmod(triPhasor - simd::ifelse(shape<=0.f, 250.f, 0.f), 1000.f);
              triPhasor = simd::ifelse(triPhasor<0.f, triPhasor+1000.f, triPhasor);
              triOut[s] += simd::ifelse( shape<=0.f,
                                      sinSimd_1000(triPhasor)*5.f * -shape, // sine component
                                      simd::ifelse(triPhasor<500.f, 5.f, -5.f) * shape // square component
                                    );
          }

          if (s==0 || inputs[TRI_LEVEL_INPUT].isPolyphonic()) {
            levelIn[TRI] = (o && !disableOver[TRI_LEVEL_INPUT]) ? float_4::zero() : inputs[TRI_LEVEL_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[TRI_LEVEL_INPUT]){
              if (o==0) levelIn[TRI] *= oversample;
              levelIn[TRI] = levelUpSample[s][TRI].process(levelIn[TRI]);
            }
          } // else preserve prior levelIn[TRI] value
          level = bipolar[TRI] ? levelIn[TRI] : simd::ifelse(levelIn[TRI]>0.f, levelIn[TRI], 0.f);
          level = level * params[TRI_LEVEL_AMT_PARAM].getValue() * lvlScale[TRI] + params[TRI_LEVEL_PARAM].getValue();
          if (clampLevel)
            level = simd::clamp(level, -1.f, 1.f);
          if (params[TRI_ASIGN_PARAM].getValue()!=1){
            mixOut[s] += triOut[s] * level;
            mixDiv += simd::fabs(level);
          }

          if (s==0 || inputs[TRI_OFFSET_INPUT].isPolyphonic()) {
            offsetIn[TRI] = (o && !disableOver[TRI_OFFSET_INPUT]) ? float_4::zero() : inputs[TRI_OFFSET_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[TRI_OFFSET_INPUT]){
              if (o==0) offsetIn[TRI] *= oversample;
              offsetIn[TRI] = offsetUpSample[s][TRI].process(offsetIn[TRI]);
            }
          } // else preserve prior offsetIn[TRI] value
          triOut[s] += clamp(offsetIn[TRI]*params[TRI_OFFSET_AMT_PARAM].getValue() + params[TRI_OFFSET_PARAM].getValue()*5.f, -5.f, 5.f);
          if (params[TRI_ASIGN_PARAM].getValue()!=0)
            triOut[s] *= level;  
        }
        
        // Square
        if (procSqr)
        {
          if (s==0 || inputs[SQR_SHAPE_INPUT].isPolyphonic()) {
            shapeIn[SQR] = (o && !disableOver[SQR_SHAPE_INPUT]) ? float_4::zero() : inputs[SQR_SHAPE_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SQR_SHAPE_INPUT]){
              if (o==0) shapeIn[SQR] *= oversample;
              shapeIn[SQR] = shapeUpSample[s][SQR].process(shapeIn[SQR]);
            }
          } // else preserve prior shapeIn[SQR] value
          if (s==0 || inputs[SQR_PHASE_INPUT].isPolyphonic()) {
            phaseIn[SQR] = (o && !disableOver[SQR_PHASE_INPUT]) ? float_4::zero() : inputs[SQR_PHASE_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SQR_PHASE_INPUT]){
              if (o==0) phaseIn[SQR] *= oversample;
              phaseIn[SQR] = phaseUpSample[s][SQR].process(phaseIn[SQR]);
            }
          } // else preserve prior phaseIn[SQR] value
          sqrPhasor = globalPhasor + (phaseIn[SQR]*params[SQR_PHASE_AMT_PARAM].getValue() + params[SQR_PHASE_PARAM].getValue()*2.f)*250.f;
          sqrPhasor = simd::fmod(sqrPhasor, 1000.f);
          sqrPhasor = simd::ifelse(sqrPhasor<0.f, sqrPhasor+1000.f, sqrPhasor);
          if (sqrMode==2) { // morph tri <--> sqr <--> saw
            float_4 shape = clamp(shapeIn[SQR]*params[SQR_SHAPE_AMT_PARAM].getValue()*shpScale[SQR] + params[SQR_SHAPE_PARAM].getValue(), -1.f, 1.f);
            sqrOut[s] = simd::ifelse(sqrPhasor<500.f, 5.f, -5.f) * (1.f - simd::abs(shape)); // square component
            // triangle and saw components
            sqrPhasor = globalPhasor + (phaseIn[SQR]*params[SQR_PHASE_AMT_PARAM].getValue() + params[SQR_PHASE_PARAM].getValue()*2.f)*250.f;
            sqrPhasor = simd::fmod(sqrPhasor + simd::ifelse(shape<=0.f, 250.f, 500.f), 1000.f);
            sqrPhasor = simd::ifelse(sqrPhasor<0.f, sqrPhasor+1000.f, sqrPhasor);
            sqrOut[s] += simd::ifelse( shape<=0.f, 
                                    (simd::ifelse(sqrPhasor<500.f, sqrPhasor, (1000.f-sqrPhasor))*.02f - 5.f) * -shape, // triangle component
                                    (sqrPhasor*0.01f - 5.f) * shape // saw component
                                  );
          } else { // PWM
            float_4 flip = (shapeIn[SQR]*params[SQR_SHAPE_AMT_PARAM].getValue()*shpScale[SQR] + params[SQR_SHAPE_PARAM].getValue() + 1.f) * 500.f;
            if (!sqrMode) flip = clamp( flip, 30.f, 970.f );
            sqrOut[s] = simd::ifelse(sqrPhasor<flip, 5.f, -5.f);
          }
          if (s==0 || inputs[SQR_LEVEL_INPUT].isPolyphonic()) {
            levelIn[SQR] = (o && !disableOver[SQR_LEVEL_INPUT]) ? float_4::zero() : inputs[SQR_LEVEL_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SQR_LEVEL_INPUT]){
              if (o==0) levelIn[SQR] *= oversample;
              levelIn[SQR] = levelUpSample[s][SQR].process(levelIn[SQR]);
            }
          } // else preserve prior levelIn[SQR] value
          level = bipolar[SQR] ? levelIn[SQR] : simd::ifelse(levelIn[SQR]>0.f, levelIn[SQR], 0.f);
          level = level * params[SQR_LEVEL_AMT_PARAM].getValue() * lvlScale[SQR] + params[SQR_LEVEL_PARAM].getValue();
          if (clampLevel)
            level = simd::clamp(level, -1.f, 1.f);
          if (params[SQR_ASIGN_PARAM].getValue()!=1){
            mixOut[s] += sqrOut[s] * level;
            mixDiv += simd::fabs(level);
          }

          if (s==0 || inputs[SQR_OFFSET_INPUT].isPolyphonic()) {
            offsetIn[SQR] = (o && !disableOver[SQR_OFFSET_INPUT]) ? float_4::zero() : inputs[SQR_OFFSET_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SQR_OFFSET_INPUT]){
              if (o==0) offsetIn[SQR] *= oversample;
              offsetIn[SQR] = offsetUpSample[s][SQR].process(offsetIn[SQR]);
            }
          } // else preserve prior offsetIn[SQR] value
          sqrOut[s] += clamp(offsetIn[SQR]*params[SQR_OFFSET_AMT_PARAM].getValue() + params[SQR_OFFSET_PARAM].getValue()*5.f, -5.f, 5.f);
          if (params[SQR_ASIGN_PARAM].getValue()!=0)
            sqrOut[s] *= level;  
        }
        
        // Saw
        if (procSaw)
        {
          if (s==0 || inputs[SAW_SHAPE_INPUT].isPolyphonic()) {
            shapeIn[SAW] = (o && !disableOver[SAW_SHAPE_INPUT]) ? float_4::zero() : inputs[SAW_SHAPE_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SAW_SHAPE_INPUT]){
              if (o==0) shapeIn[SAW] *= oversample;
              shapeIn[SAW] = shapeUpSample[s][SAW].process(shapeIn[SAW]);
            }
          } // else preserve prior shapeIn[SAW] value
          float_4 shape = clamp(shapeIn[SAW]*params[SAW_SHAPE_AMT_PARAM].getValue()*shpScale[SAW] + params[SAW_SHAPE_PARAM].getValue(), -1.f, 1.f);
          if (s==0 || inputs[SAW_PHASE_INPUT].isPolyphonic()) {
            phaseIn[SAW] = (o && !disableOver[SAW_PHASE_INPUT]) ? float_4::zero() : inputs[SAW_PHASE_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SAW_PHASE_INPUT]){
              if (o==0) phaseIn[SAW] *= oversample;
              phaseIn[SAW] = phaseUpSample[s][SAW].process(phaseIn[SAW]);
            }
          } // else preserve prior phaseIn[SAW] value
          sawPhasor = globalPhasor + (phaseIn[SAW]*params[SAW_PHASE_AMT_PARAM].getValue() + params[SAW_PHASE_PARAM].getValue()*2.f)*250.f;
          sawPhasor = simd::fmod(sawPhasor, 1000.f);
          sawPhasor = simd::ifelse(sawPhasor<0.f, sawPhasor+1000.f, sawPhasor);
          sawPhasor *= 0.001f;
          switch (sawMode) {
            case 0:  // exp/log
              sawOut[s] = crossfade(sawPhasor, ifelse(shape>0.f, 11.f*sawPhasor/(10.f*simd::abs(sawPhasor)+1.f), simd::sgn(sawPhasor)*simd::pow(sawPhasor,4)), ifelse(shape>0.f, shape, -shape))*10.f-5.f;
              break;
            case 1:  // J Curve
              sawOut[s] = normSigmoid(sawPhasor, -shape*0.90) * 10.f - 5.f;
              break;
            case 2: // S Curve
              sawOut[s] = normSigmoid(sawPhasor*2.f-1.f, -shape*0.85) * 5.f;
              break;
            case 3: // Rectify
            case 4: // Normalized Rectify
              shape = -shape;
              shapeSign = simd::sgn(shape);
              sawOut[s] = sawPhasor*2.f-1.f;
              sawOut[s] = simd::ifelse(shapeSign==0, sawOut[s], -(shapeSign*simd::abs(-sawOut[s]+shapeSign-shape)-shapeSign+shape));
              if (sawMode==4 /*Normalized Rectify*/)
                sawOut[s] = -((1+simd::abs(shape))*-sawOut[s]-shape);
              sawOut[s] *= 5.f;
              break;
            default: // 5 morph square <--> saw <--> even
              sawOut[s] = (sawPhasor*10.f - 5.f) * simd::ifelse(shape<0.f, 1.f + shape, 1.f); // saw component
              // square component
              sawPhasor = globalPhasor + (phaseIn[SAW]*params[SAW_PHASE_AMT_PARAM].getValue() + params[SAW_PHASE_PARAM].getValue()*2.f)*250.f;
              sawPhasor = simd::fmod(sawPhasor + simd::ifelse(shape<=0.f, 500.f, 0.f), 1000.f);
              sawPhasor = simd::ifelse(sawPhasor<0.f, sawPhasor+1000.f, sawPhasor);
              sawOut[s] += simd::ifelse(sawPhasor<500.f, 5.f, -5.f) * simd::abs(shape) * simd::ifelse(shape<0.f, 1.f, 0.5f);
              // sine component
              sawPhasor = globalPhasor + (phaseIn[SAW]*params[SAW_PHASE_AMT_PARAM].getValue() + params[SAW_PHASE_PARAM].getValue()*2.f)*250.f;
              sawPhasor = simd::fmod(sawPhasor, 1000.f);
              sawPhasor = simd::ifelse(sawPhasor<0.f, sawPhasor+1000.f, sawPhasor);
              sawOut[s] += simd::ifelse(shape<0.f, 0.f, sinSimd_1000(sawPhasor) * 3.175 * shape);
          }

          if (s==0 || inputs[SAW_LEVEL_INPUT].isPolyphonic()) {
            levelIn[SAW] = (o && !disableOver[SAW_LEVEL_INPUT]) ? float_4::zero() : inputs[SAW_LEVEL_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SAW_LEVEL_INPUT]){
              if (o==0) levelIn[SAW] *= oversample;
              levelIn[SAW] = levelUpSample[s][SAW].process(levelIn[SAW]);
            }
          } // else preserve prior levelIn[SAW] value
          level = bipolar[SAW] ? levelIn[SAW] : simd::ifelse(levelIn[SAW]>0.f, levelIn[SAW], 0.f);
          level = level * params[SAW_LEVEL_AMT_PARAM].getValue() * lvlScale[SAW] + params[SAW_LEVEL_PARAM].getValue();
          if (clampLevel)
            level = simd::clamp(level, -1.f, 1.f);
          if (params[SAW_ASIGN_PARAM].getValue()!=1){
            mixOut[s] += sawOut[s] * level;
            mixDiv += simd::fabs(level);
          }

          if (s==0 || inputs[SAW_OFFSET_INPUT].isPolyphonic()) {
            offsetIn[SAW] = (o && !disableOver[SAW_OFFSET_INPUT]) ? float_4::zero() : inputs[SAW_OFFSET_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[SAW_OFFSET_INPUT]){
              if (o==0) offsetIn[SAW] *= oversample;
              offsetIn[SAW] = offsetUpSample[s][SAW].process(offsetIn[SAW]);
            }
          } // else preserve prior offsetIn[SAW] value
          sawOut[s] += clamp(offsetIn[SAW]*params[SAW_OFFSET_AMT_PARAM].getValue() + params[SAW_OFFSET_PARAM].getValue()*5.f, -5.f, 5.f);
          if (params[SAW_ASIGN_PARAM].getValue()!=0)
            sawOut[s] *= level;  
        }
        
        // Mix
        if (procMix) {
          int folds=10;
          if (mixMode > 2) {
            mixOut[s] = simd::ifelse(mixDiv>0.f, mixOut[s]/mixDiv, mixOut[s]);
            folds=3;
          }
          if (mixType) {
            if (s==0 || inputs[MIX_SHAPE_INPUT].isPolyphonic()) {
              shapeIn[MIX] = (o && !disableOver[MIX_SHAPE_INPUT]) ? float_4::zero() : inputs[MIX_SHAPE_INPUT].getPolyVoltageSimd<float_4>(c);
              if (procOver[MIX_SHAPE_INPUT]){
                if (o==0) shapeIn[MIX] *= oversample;
                shapeIn[MIX] = shapeUpSample[s][MIX].process(shapeIn[MIX]);
              }
            } // else preserve prior shapeIn[MIX] value
            float_4 drive = clamp(shapeIn[MIX]*params[MIX_SHAPE_AMT_PARAM].getValue() + params[MIX_SHAPE_PARAM].getValue()+1.f, 0.f, 3.f)*2.f + 1.f;
            if (mixType==1){
              mixOut[s] = softClip<float_4>(mixOut[s]*2.f*drive)/2.f;
            }
            if (mixType==2){
              mixOut[s] *= drive;
              float_4 clmp;
              for (int i=0; i<folds; i++){
                clmp = clamp(mixOut[s],-5,5);
                mixOut[s] = clmp + clmp - mixOut[s];
              }
            }
          }
          if (s==0 || inputs[MIX_OFFSET_INPUT].isPolyphonic()) {
            offsetIn[MIX] = (o && !disableOver[MIX_OFFSET_INPUT]) ? float_4::zero() : inputs[MIX_OFFSET_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[MIX_OFFSET_INPUT]){
              if (o==0) offsetIn[MIX] *= oversample;
              offsetIn[MIX] = offsetUpSample[s][MIX].process(offsetIn[MIX]);
            }
          } // else preserve prior offsetIn[MIX] value
          mixOut[s] += clamp(offsetIn[MIX]*params[MIX_OFFSET_AMT_PARAM].getValue() + params[MIX_OFFSET_PARAM].getValue()*5.f, -5.f, 5.f);
          if (s==0 || inputs[MIX_LEVEL_INPUT].isPolyphonic()) {
            levelIn[MIX] = (o && !disableOver[MIX_LEVEL_INPUT]) ? float_4::zero() : inputs[MIX_LEVEL_INPUT].getPolyVoltageSimd<float_4>(c);
            if (procOver[MIX_LEVEL_INPUT]){
              if (o==0) levelIn[MIX] *= oversample;
              levelIn[MIX] = levelUpSample[s][MIX].process(levelIn[MIX]);
            }
          } // else preserve prior levelIn[MIX] value
          level = bipolar[MIX] ? levelIn[MIX] : simd::ifelse(levelIn[MIX]>0.f, levelIn[MIX], 0.f);
          level = level * params[MIX_LEVEL_AMT_PARAM].getValue() * lvlScale[MIX] + params[MIX_LEVEL_PARAM].getValue();
          if (clampLevel)
            level = simd::clamp(level, -1.f, 1.f);
          mixOut[s] *= level;
        }

        // FINAL PROCESSING
        // Handle one shots
        if (once){
          sinOut[s] = simd::ifelse(onceActive[s]==float_4::zero(), float_4::zero(), sinOut[s]);
          triOut[s] = simd::ifelse(onceActive[s]==float_4::zero(), float_4::zero(), triOut[s]);
          sqrOut[s] = simd::ifelse(onceActive[s]==float_4::zero(), float_4::zero(), sqrOut[s]);
          sawOut[s] = simd::ifelse(onceActive[s]==float_4::zero(), float_4::zero(), sawOut[s]);
          mixOut[s] = simd::ifelse(onceActive[s]==float_4::zero(), float_4::zero(), mixOut[s]);
        }
        // Remove DC offset
        if (params[DC_PARAM].getValue()) {
          if (outputs[SIN_OUTPUT].isConnected())
            sinOut[s] = dcBlockFilter[s][SIN].process(sinOut[s]/*, oversample*/);
          if (outputs[TRI_OUTPUT].isConnected())
            triOut[s] = dcBlockFilter[s][TRI].process(triOut[s]/*, oversample*/);
          if (outputs[SQR_OUTPUT].isConnected())
            sqrOut[s] = dcBlockFilter[s][SQR].process(sqrOut[s]/*, oversample*/);
          if (outputs[SAW_OUTPUT].isConnected())
            sawOut[s] = dcBlockFilter[s][SAW].process(sawOut[s]/*, oversample*/);
          if (outputs[MIX_OUTPUT].isConnected())
            mixOut[s] = dcBlockFilter[s][MIX].process(mixOut[s]/*, oversample*/);
        }
        // Downsample outputs
        if (oversample>1) {
          if (outputs[SIN_OUTPUT].isConnected())
            sinOut[s] = outDownSample[s][SIN].process(sinOut[s]);
          if (outputs[TRI_OUTPUT].isConnected())
            triOut[s] = outDownSample[s][TRI].process(triOut[s]);
          if (outputs[SQR_OUTPUT].isConnected())
            sqrOut[s] = outDownSample[s][SQR].process(sqrOut[s]);
          if (outputs[SAW_OUTPUT].isConnected())
            sawOut[s] = outDownSample[s][SAW].process(sawOut[s]);
          if (outputs[MIX_OUTPUT].isConnected())
            mixOut[s] = outDownSample[s][MIX].process(mixOut[s]);
        }
      }
    }
    
    float_4 out{};
    for (int s=0, c=0; s<simdCnt; s++, c+=4) {
      outputs[SIN_OUTPUT].setVoltageSimd( sinOut[s], c );
      outputs[TRI_OUTPUT].setVoltageSimd( triOut[s], c );
      outputs[SQR_OUTPUT].setVoltageSimd( sqrOut[s], c );
      outputs[SAW_OUTPUT].setVoltageSimd( sawOut[s], c );
      outputs[MIX_OUTPUT].setVoltageSimd( mixOut[s], c );
    }
    outputs[SIN_OUTPUT].setChannels(channels);
    outputs[TRI_OUTPUT].setChannels(channels);
    outputs[SQR_OUTPUT].setChannels(channels);
    outputs[SAW_OUTPUT].setChannels(channels);
    outputs[MIX_OUTPUT].setChannels(channels);
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_t* array = json_array();
    for (int i=0; i<INPUTS_LEN; i++)
      json_array_append_new(array, json_boolean(disableOver[i]));
    json_object_set_new(rootJ, "disableOver", array);
    array = json_array();
    for (int i=0; i<5; i++){
      json_array_append_new(array, json_boolean(unity5[i]));
    }
    json_object_set_new(rootJ, "unity5", array);
    array = json_array();
    for (int i=0; i<5; i++){
      json_array_append_new(array, json_boolean(bipolar[i]));
    }
    json_object_set_new(rootJ, "bipolar", array);
    array = json_array();
    for (int i=0; i<4; i++){
      json_array_append_new(array, json_boolean(oldShpCV[i]));
    }
    json_object_set_new(rootJ, "oldShpCV", array);
    json_object_set_new(rootJ, "linDCCouple", json_boolean(linDCCouple));
    json_object_set_new(rootJ, "overParam", json_integer(params[OVER_PARAM].getValue()));
    json_object_set_new(rootJ, "clampLevel", json_boolean(clampLevel));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* array;
    json_t* val;
    size_t index;
    if ((array = json_object_get(rootJ, "disableOver"))) {
      json_array_foreach(array, index, val){
        disableOver[index] = json_boolean_value(val);
      }
    }
    if (!(array = json_object_get(rootJ, "unity5"))) {
      array = json_object_get(rootJ, "ringMod");
    }
    if (array) {
      json_array_foreach(array, index, val){
        setUnity5( index, json_boolean_value(val));
      }
    }
    if ((array = json_object_get(rootJ, "bipolar"))) {
      json_array_foreach(array, index, val){
        setBipolar( index, json_boolean_value(val));
      }
    }
    else {
      for (int i=0; i<5; i++)
        setBipolar(i, true);
    }
    if ((array = json_object_get(rootJ, "oldShpCV"))) {
      json_array_foreach(array, index, val){
        setOldShpCV( index, json_boolean_value(val));
      }
    }
    else {
      setOldShpCV(SIN, true);
      setOldShpCV(TRI, true);
      setOldShpCV(SAW, true);
    }
    if ((val = json_object_get(rootJ, "linDCCouple"))) {
      linDCCouple = json_boolean_value(val);
    }
    setMode();
    if ((val = json_object_get(rootJ, "overParam"))) {
      params[OVER_PARAM].setValue(json_integer_value(val));
    }
    if ((val = json_object_get(rootJ, "clampLevel"))) {
      clampLevel = json_boolean_value(val);
    }
    else {
      clampLevel = false;
    }
  }
  
};

struct OscillatorWidget : VenomWidget {
  
  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPinkButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
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

  struct PWSwitch : GlowingSvgSwitchLockable {
    PWSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
    }
  };

  struct MixShpSwitch : GlowingSvgSwitchLockable {
    MixShpSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };
  
  struct ShpSwitch : GlowingSvgSwitchLockable {
    ShpSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPinkButtonSwitch.svg")));
    }
  };

  struct DCBlockSwitch : GlowingSvgSwitchLockable {
    DCBlockSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
    }
  };

  struct AssignSwitch : GlowingSvgSwitchLockable {
    AssignSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };
  
  struct OverPort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      Oscillator* module = static_cast<Oscillator*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem("Disable oversampling", "", &module->disableOver[portId]));
      PolyPort::appendContextMenu(menu);
    }
  };
  
  struct LinPort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      Oscillator* module = static_cast<Oscillator*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem("Disable oversampling", "", &module->disableOver[portId]));
      menu->addChild(createBoolPtrMenuItem("DC coupled", "", &module->linDCCouple));
      PolyPort::appendContextMenu(menu);
    }
  };

  struct LevelPort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      Oscillator* module = static_cast<Oscillator*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem("Disable oversampling", "", &module->disableOver[portId]));
      menu->addChild(createBoolMenuItem(
        "VCA unity = 5V", "",
        [=]() {return module->unity5[portId-Oscillator::SIN_LEVEL_INPUT];},
        [=](bool rm) {
          module->setUnity5(portId-Oscillator::SIN_LEVEL_INPUT, rm);
        }
      ));
      menu->addChild(createBoolMenuItem(
        "Bipolar VCA (ring mod)", "",
        [=]() {return module->bipolar[portId-Oscillator::SIN_LEVEL_INPUT];},
        [=](bool val) {
          module->setBipolar(portId-Oscillator::SIN_LEVEL_INPUT, val);
        }
      ));
      PolyPort::appendContextMenu(menu);
    }
  };
  
  struct ShapePort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      Oscillator* module = static_cast<Oscillator*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem("Disable oversampling", "", &module->disableOver[portId]));
      menu->addChild(createBoolMenuItem(
        "20 VPP full range (old behavior)", "",
        [=]() {return module->oldShpCV[portId-Oscillator::SIN_SHAPE_INPUT];},
        [=](bool val) {
          module->setOldShpCV(portId-Oscillator::SIN_SHAPE_INPUT, val);
        }
      ));
      PolyPort::appendContextMenu(menu);
    }
  };
  
  template <class TOverPort>
  TOverPort* createOverInputCentered(math::Vec pos, engine::Module* module, int inputId) {
    TOverPort* o = createInputCentered<TOverPort>(pos, module, inputId);
    o->portId = inputId;
    return o;
  }

  OscillatorWidget(Oscillator* module) {
    setModule(module);
    setVenomPanel("Oscillator");
    
    addParam(createLockableParamCentered<ModeSwitch>(Vec(14.5f,37.5f), module, Oscillator::MODE_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(35.5f,37.5f), module, Oscillator::OVER_PARAM));
    addParam(createLockableParamCentered<DCBlockSwitch>(Vec(56.5f,37.5f), module, Oscillator::DC_PARAM));
    addParam(createLockableParamCentered<GlowingTinyButtonLockable>(Vec(77.5f,37.5f), module, Oscillator::RESET_POLY_PARAM));

    addParam(createLockableParamCentered<ShpSwitch>(Vec(130.0f,49.0f), module, Oscillator::SINSHP_PARAM));
    addParam(createLockableParamCentered<ShpSwitch>(Vec(175.0f,49.0f), module, Oscillator::TRISHP_PARAM));
    addParam(createLockableParamCentered<PWSwitch>(Vec(220.0f,49.0f), module, Oscillator::PW_PARAM));
    addParam(createLockableParamCentered<ShpSwitch>(Vec(265.0f,49.0f), module, Oscillator::SAWSHP_PARAM));
    addParam(createLockableParamCentered<MixShpSwitch>(Vec(310.0f,49.0f), module, Oscillator::MIXSHP_PARAM));
    
    addParam(createLockableParamCentered<RoundHugeBlackKnobLockable>(Vec(46.5f,93.5f), module, Oscillator::FREQ_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(29.f,157.f), module, Oscillator::OCTAVE_PARAM));

    addInput(createOverInputCentered<OverPort>(Vec(64.f, 158.f), module, Oscillator::REV_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(77.5f, 146.5f), module, Oscillator::REV_LIGHT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(29.f,206.f), module, Oscillator::EXP_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(64.f,206.f), module, Oscillator::LIN_PARAM));
    addInput(createOverInputCentered<OverPort>(Vec(29.f, 241.5f), module, Oscillator::EXP_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(42.5f, 230.f), module, Oscillator::EXP_LIGHT));
    addInput(createOverInputCentered<LinPort>(Vec(64.f, 241.5f), module, Oscillator::LIN_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(77.5f, 230.f), module, Oscillator::LIN_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<RedLight>>(Vec(77.5f, 253.f), module, Oscillator::LIN_DC_LIGHT));
    addInput(createInputCentered<PolyPort>(Vec(29.f, 290.5f), module, Oscillator::EXP_DEPTH_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(64.f, 290.5f), module, Oscillator::LIN_DEPTH_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(29.f, 335.5f), module, Oscillator::VOCT_INPUT));
    addInput(createOverInputCentered<OverPort>(Vec(64.f, 335.5f), module, Oscillator::SYNC_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(77.5f, 324.f), module, Oscillator::SYNC_LIGHT));
    
    float dx = 45.f;
    float dy = 59.f;
    for (int y=0; y<4; y++) {
      for (int x=0; x<5; x++) {
        addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(119.5f+dx*x,68.5f+dy*y), module, Oscillator::GRID_PARAM+y*10+x));
        addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(140.5f+dx*x,68.5f+dy*y), module, Oscillator::GRID_PARAM+y*10+x+5));
        switch (y) {
          case 0:
            if (x==2 || x==4)
              addInput(createOverInputCentered<OverPort>(Vec(130.f+dx*x,94.5f+dy*y), module, Oscillator::GRID_INPUT+y*5+x));
            else
              addInput(createOverInputCentered<ShapePort>(Vec(130.f+dx*x,94.5f+dy*y), module, Oscillator::GRID_INPUT+y*5+x));
            break;
          case 3:
            addInput(createOverInputCentered<LevelPort>(Vec(130.f+dx*x,94.5f+dy*y), module, Oscillator::GRID_INPUT+y*5+x));
            addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(116.5f+dx*x, 83.f+dy*y), module, Oscillator::SIN_RM_LIGHT+x));
            addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(116.5f+dx*x, 106.f+dy*y), module, Oscillator::SIN_VCA_LIGHT+x));
            break;
          default:
            addInput(createOverInputCentered<OverPort>(Vec(130.f+dx*x,94.5f+dy*y), module, Oscillator::GRID_INPUT+y*5+x));
        }
        addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(143.5f+dx*x, 83.f+dy*y), module, Oscillator::GRID_LIGHT+y*10+x*2));
      }
    }
    for (int x=0; x<4; x++) {
      addParam(createLockableParamCentered<AssignSwitch>(Vec(130.f+dx*x,291.5f), module, Oscillator::ASGN_PARAM+x));
    }
    for (int x=0; x<5; x++) {
      addOutput(createOutputCentered<PolyPort>(Vec(130.f+dx*x,335.5f), module, Oscillator::GRID_OUTPUT+x));
    }
  }

  void step() override {
    VenomWidget::step();
    Oscillator* mod = dynamic_cast<Oscillator*>(this->module);
    if(mod) {
      bool over = mod->params[Oscillator::OVER_PARAM].getValue();
      mod->lights[Oscillator::REV_LIGHT].setBrightness(over && !(mod->disableOver[Oscillator::REV_INPUT]) && mod->inputs[Oscillator::REV_INPUT].isConnected());
      mod->lights[Oscillator::REV_LIGHT+1].setBrightness(over && mod->disableOver[Oscillator::REV_INPUT] && mod->inputs[Oscillator::REV_INPUT].isConnected());
      mod->lights[Oscillator::EXP_LIGHT].setBrightness(over && !(mod->disableOver[Oscillator::EXP_INPUT]) && mod->inputs[Oscillator::EXP_INPUT].isConnected() && !(mod->alternate));
      mod->lights[Oscillator::EXP_LIGHT+1].setBrightness(over && mod->disableOver[Oscillator::EXP_INPUT] && mod->inputs[Oscillator::EXP_INPUT].isConnected() && !(mod->alternate));
      mod->lights[Oscillator::LIN_LIGHT].setBrightness(over && !(mod->disableOver[Oscillator::LIN_INPUT]) && mod->inputs[Oscillator::LIN_INPUT].isConnected());
      mod->lights[Oscillator::LIN_LIGHT+1].setBrightness(over && mod->disableOver[Oscillator::LIN_INPUT] && mod->inputs[Oscillator::LIN_INPUT].isConnected());
      mod->lights[Oscillator::SYNC_LIGHT].setBrightness(over && !(mod->disableOver[Oscillator::SYNC_INPUT]) && mod->inputs[Oscillator::SYNC_INPUT].isConnected());
      mod->lights[Oscillator::SYNC_LIGHT+1].setBrightness(over && mod->disableOver[Oscillator::SYNC_INPUT] && mod->inputs[Oscillator::SYNC_INPUT].isConnected());
      for (int y=0; y<4; y++) {
        for (int x=0; x<5; x++) {
          mod->lights[Oscillator::GRID_LIGHT+y*10+x*2].setBrightness(over && !(mod->disableOver[Oscillator::GRID_INPUT+y*5+x]) && mod->inputs[Oscillator::GRID_INPUT+y*5+x].isConnected());
          mod->lights[Oscillator::GRID_LIGHT+y*10+x*2+1].setBrightness(over && mod->disableOver[Oscillator::GRID_INPUT+y*5+x] && mod->inputs[Oscillator::GRID_INPUT+y*5+x].isConnected());
        }
      }
      mod->lights[Oscillator::LIN_DC_LIGHT].setBrightness(mod->linDCCouple);
    }
  }

  void appendContextMenu(Menu* menu) override {
    Oscillator* module = dynamic_cast<Oscillator*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Limit levels to 100%", "", &module->clampLevel));
    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelOscillator = createModel<Oscillator, OscillatorWidget>("Oscillator");

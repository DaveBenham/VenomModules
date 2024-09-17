// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"
#include <float.h>

struct VCOUnit : VenomModule {
 
  enum ParamId {
    MODE_PARAM,
    OVER_PARAM,
    DC_PARAM,
    FREQ_PARAM,
    WAVE_PARAM,
    OCTAVE_PARAM,
    RESET_POLY_PARAM,
    EXP_PARAM,
    LIN_PARAM,
    SHAPE_MODE_PARAM,
    SHAPE_PARAM,
    SHAPE_AMT_PARAM,
    PHASE_PARAM,
    PHASE_AMT_PARAM,
    OFFSET_PARAM,
    OFFSET_AMT_PARAM,
    LEVEL_PARAM,
    LEVEL_AMT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    EXP_INPUT,
    LIN_INPUT,
    EXP_DEPTH_INPUT,
    LIN_DEPTH_INPUT,
    VOCT_INPUT,
    SYNC_INPUT,
    REV_INPUT,
    SHAPE_INPUT,
    PHASE_INPUT,
    OFFSET_INPUT,
    LEVEL_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(REV_LIGHT,2),
    ENUMS(EXP_LIGHT,2),
    ENUMS(LIN_LIGHT,2),
    ENUMS(SYNC_LIGHT,2),
    ENUMS(SHAPE_LIGHT,2),
    ENUMS(PHASE_LIGHT,2),
    ENUMS(OFFSET_LIGHT,2),
    ENUMS(LEVEL_LIGHT,2),
    RM_LIGHT,
    VCA_LIGHT,
    LIN_DC_LIGHT,
    LIGHTS_LEN
  };

  bool clampLevel = true;
  bool disableOver[INPUTS_LEN]{};
  bool unity5 = false;
  bool bipolar = false;
  float lvlScale = 0.1f;
  float shpScale = 0.2f;
  bool softSync = false;
  bool alternate = false;
  bool lfo = false;
  using float_4 = simd::float_4;
  int oversample = -1;
  std::vector<int> oversampleValues = {1,2,4,8,16,32};
  OversampleFilter_4 expUpSample[4]{}, linUpSample[4]{}, revUpSample[4]{}, syncUpSample[4]{},
                     shapeUpSample[4]{}, phaseUpSample[4]{}, offsetUpSample[4]{}, levelUpSample[4]{},
                     outDownSample[4]{};
  float_4 phasor[4]{}, phasorDir[4]{1.f, 1.f, 1.f, 1.f};
  DCBlockFilter_4 linDcBlockFilter[4]{}, outDcBlockFilter[4]{};
  bool linDCCouple = false;
  dsp::SchmittTrigger syncTrig[16], revTrig[16];
  float modeFreq[3] = {dsp::FREQ_C4, 2.f, 100.f}, biasFreq = 0.02f;
  int currentMode = 0;
  int mode = 0;
  int wave = 0;
  bool once = false;
  bool noRetrigger = false;
  bool gated = false;
  float_4 onceActive[4]{};
  int modeDefaultOver[3] = {2, 0, 2};
  static constexpr float maxFreq = 12000.f;
  
  struct ShapeQuantity : ParamQuantity {
    float getDisplayValue() override {
      float val = ParamQuantity::getDisplayValue();
      if (displayOffset) { // displayOffset != 0 indicates square wave pulse width
        switch (static_cast<int>(module->params[SHAPE_MODE_PARAM].getValue())) {
          case 0:
            val = clamp(val, 3.f, 97.f);
            break;
          case 2:
            val = val*2.f - 100.f;
            break;
        }
      }  
      return val;
    }
    void setDisplayValue(float v) override {
      if (displayOffset && module->params[SHAPE_MODE_PARAM].getValue()==2.f)
        v = (v+100.f)/2.f;
      ParamQuantity::setDisplayValue(v);
    }
  };

  struct FreqQuantity : ParamQuantity {
    float getDisplayValue() override {
      VCOUnit* mod = reinterpret_cast<VCOUnit*>(this->module);
      float freq = 0.f;
      if (mod->mode < 2)
        freq = pow(2.f, mod->params[FREQ_PARAM].getValue() + mod->params[OCTAVE_PARAM].getValue()) * mod->modeFreq[mod->mode];
      else
        freq = mod->params[FREQ_PARAM].getValue() * mod->biasFreq;
      return freq < maxFreq ? freq : maxFreq;
    }
    void setDisplayValue(float v) override {
      VCOUnit* mod = reinterpret_cast<VCOUnit*>(this->module);
      if (v > maxFreq) v = maxFreq;
      if (mod->mode < 2)
        setValue(clamp(std::log2f(v / mod->modeFreq[mod->mode]) - mod->params[OCTAVE_PARAM].getValue(), -4.f, 4.f));
      else
        setValue(clamp(v / mod->biasFreq, -4.f, 4.f));
    }
  };


  VCOUnit() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 7.f, 0.f, "Frequency Mode", {"Audio frequency", "Low frequency", "0Hz carrier", "Triggered audio one shot", "Retriggered audio one shot", "Gated audio one shot", "Retriggered LFO one shot", "Gated LFO one shot"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 2.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    configSwitch<FixedSwitchQuantity>(DC_PARAM,   0.f, 1.f, 0.f, "DC Block", {"Off", "On"});
    configButton(RESET_POLY_PARAM, "Reset polyphony count");

    configParam<FreqQuantity>(FREQ_PARAM, -4.f, 4.f, 0.f, "Frequency", " Hz");
    configSwitch<FixedSwitchQuantity>(WAVE_PARAM, 0.f, 3.f, 0.f, "Waveform", {"Sine", "Triangle", "Square", "Saw"});
    configParam(OCTAVE_PARAM, -4.f, 4.f, 0.f, "Octave");
    configParam(EXP_PARAM, -1.f, 1.f, 0.f, "Exponential FM", "%", 0.f, 100.f);
    configParam(LIN_PARAM, -1.f, 1.f, 0.f, "Linear FM", "%", 0.f, 100.f);
    configInput(EXP_INPUT, "Exponential FM");
    configLight(EXP_LIGHT, "Exponential FM oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";
    configInput(LIN_INPUT, "Linear FM");
    configLight(LIN_LIGHT, "Linear FM oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";
    configLight(LIN_DC_LIGHT, "Linear FM DC coupled indicator");
    configInput(EXP_DEPTH_INPUT, "Exponential FM depth");
    configInput(LIN_DEPTH_INPUT, "Linear FM depth");
    configInput(SYNC_INPUT, "Hard Sync");
    configLight(SYNC_LIGHT, "Hard Sync oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";
    configInput(REV_INPUT, "Soft sync");
    configLight(REV_LIGHT, "Soft sync oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";
    configInput(VOCT_INPUT, "V/Oct");
    configOutput(OUTPUT, "");

    configSwitch<FixedSwitchQuantity>(SHAPE_MODE_PARAM, 0.f, 5.f, 0.f, "Shape Mode", {"log/exp", "J-curve", "S-curve", "Rectify", "Normalized Rectify", "Temp"});
    configParam<ShapeQuantity>(SHAPE_PARAM, -1.f, 1.f, 0.f, "Shape", "%", 0.f, 100.f, 0.f);
    configParam(SHAPE_AMT_PARAM, -1.f, 1.f, 0.f, "Shape CV amount", "%", 0.f, 100.f);
    configInput(SHAPE_INPUT, "Shape CV");
    configLight(SHAPE_LIGHT, "Shape oversample indicator")->description = "off = N/A, yellow = oversampled, red = disabled";

    configParam(PHASE_PARAM, -1.f, 1.f, 0.f, "Phase", "\u00B0", 0.f, 180.f);
    configParam(PHASE_AMT_PARAM, -1.f, 1.f, 0.f, "Phase CV amount", "%", 0.f, 100.f);
    configInput(PHASE_INPUT, "Phase CV");
    configLight(PHASE_LIGHT, "Phase oversample indicator")->description = "off = N/A, yellow = oversampled, red = disabled";

    configParam(OFFSET_PARAM, -1.f, 1.f, 0.f, "Offset", " V", 0.f, 5.f);
    configParam(OFFSET_AMT_PARAM, -1.f, 1.f, 0.f, "Offset CV amount", "%", 0.f, 100.f);
    configInput(OFFSET_INPUT, "Offset CV");
    configLight(OFFSET_LIGHT, "Offset oversample indicator")->description = "off = N/A, yellow = oversampled, red = disabled";

    configParam(LEVEL_PARAM, -1.f, 1.f, 1.f, "Level", "%", 0.f, 100.f);
    configParam(LEVEL_AMT_PARAM, -1.f, 1.f, 0.f, "Level CV amount", "%", 0.f, 100.f);
    configInput(LEVEL_INPUT, "Level CV");
    configLight(LEVEL_LIGHT, "Level oversample indicator")->description = "off = N/A, yellow = oversampled, red = disabled";
    configLight(RM_LIGHT, "VCA unity = 5V indicator");
    configLight(VCA_LIGHT, "Bipolar VCA indicator");
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
  
  void setWave() {
    wave = static_cast<int>(params[WAVE_PARAM].getValue());
    bool locked = paramExtensions[SHAPE_MODE_PARAM].locked;
    if (locked)
      setLock(false, SHAPE_MODE_PARAM);
    ParamQuantity *q = paramQuantities[SHAPE_PARAM];
    SwitchQuantity *sq = static_cast<SwitchQuantity*>(paramQuantities[SHAPE_MODE_PARAM]);
    switch (wave) {
      case 0: // SIN
        if (sq->getImmediateValue()>4)
          sq->setImmediateValue(4.f);
        sq->maxValue = 4.f;
        sq->labels = {"log/exp", "J-curve", "S-curve", "Rectify", "Normalized Rectify"};
        q->displayMultiplier = 100.f;
        q->displayOffset = 0.f;
        break;
      case 1: // TRI
        sq->maxValue = 5.f;
        sq->labels = {"log/exp", "J-curve", "S-curve", "Rectify", "Normalized Rectify", "Morph SIN <--> TRI <--> SQR"};
        q->displayMultiplier = 100.f;
        q->displayOffset = 0.f;
        break;
      case 2: // SQR
        if (sq->getImmediateValue()>2)
          sq->setImmediateValue(2.f);
        sq->maxValue = 2.f;
        sq->labels = {"Limited PWM 3%-97%", "Full PWM 0%-100%", "Morph TRI <--> SQR <--> SAW"};
        q->displayMultiplier = 50.f;
        q->displayOffset = 50.f;
        break;
      case 3: // SAW
        sq->maxValue = 5.f;
        sq->labels = {"log/exp", "J-curve", "S-curve", "Rectify", "Normalized Rectify", "Morph SQR <--> SAW <--> EVEN"};
        q->displayMultiplier = 100.f;
        q->displayOffset = 0.f;
        break;
    }
    if (locked)
      setLock(true, SHAPE_MODE_PARAM);
  }

  void setUnity5(bool rm) {
    unity5 = rm;
    lvlScale = rm ? 0.2f : 0.1f;
    lights[VCOUnit::RM_LIGHT].setBrightness(rm);
  }
  
  void setBipolar(bool val) {
    bipolar = val;
    lights[VCOUnit::VCA_LIGHT].setBrightness(val);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    if (currentMode != static_cast<int>(params[MODE_PARAM].getValue())) {
      setMode();
    }
    if (wave != static_cast<int>(params[WAVE_PARAM].getValue())) {
      setWave();
    }

    if (oversample != oversampleValues[params[OVER_PARAM].getValue()]) {
      oversample = oversampleValues[params[OVER_PARAM].getValue()];
      for (int i=0; i<4; i++){
        expUpSample[i].setOversample(oversample);
        linUpSample[i].setOversample(oversample);
        revUpSample[i].setOversample(oversample);
        syncUpSample[i].setOversample(oversample);
        shapeUpSample[i].setOversample(oversample);
        phaseUpSample[i].setOversample(oversample);
        offsetUpSample[i].setOversample(oversample);
        levelUpSample[i].setOversample(oversample);
        outDownSample[i].setOversample(oversample);
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
            shapeIn{}, phaseIn{}, offsetIn{}, levelIn{}, out[4]{}, wavePhasor{};
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
    
    int shapeMode = static_cast<int>(params[SHAPE_MODE_PARAM].getValue());
    bool procOver[INPUTS_LEN]{};
    for (int i=0; i<INPUTS_LEN; i++)
      procOver[i] = oversample>1 && inputs[i].isConnected() && !disableOver[i];
    // main loops
    for (int o=0; o<oversample; o++){
      for (int s=0, c=0; s<simdCnt; s++, c+=4){
        float_4 level{};
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
          linIn = linDcBlockFilter[s].process(linIn); //, oversample);
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

        // Process Waveform
        if (s==0 || inputs[SHAPE_INPUT].isPolyphonic()) {
          shapeIn = (o && !disableOver[SHAPE_INPUT]) ? float_4::zero() : inputs[SHAPE_INPUT].getPolyVoltageSimd<float_4>(c);
          if (procOver[SHAPE_INPUT]){
            if (o==0) shapeIn *= oversample;
            shapeIn = shapeUpSample[s].process(shapeIn);
          }
        } // preserve prior shapeIn value
        float_4 shape = clamp(shapeIn*params[SHAPE_AMT_PARAM].getValue()*shpScale + params[SHAPE_PARAM].getValue(), -1.f, 1.f);
        if (s==0 || inputs[PHASE_INPUT].isPolyphonic()) {
          phaseIn = (o && !disableOver[PHASE_INPUT]) ? float_4::zero() : inputs[PHASE_INPUT].getPolyVoltageSimd<float_4>(c);
          if (procOver[PHASE_INPUT]){
            if (o==0) phaseIn *= oversample;
            phaseIn = phaseUpSample[s].process(phaseIn);
          }
        } // else preserve prior phaseIn value

        float_4 shapeSign = 0.f;
        switch (wave) {
          case 0: // SIN
            wavePhasor = phasor[s] + (phaseIn*params[PHASE_AMT_PARAM].getValue() + params[PHASE_PARAM].getValue()*2.f)*250.f - 250.f;
            wavePhasor = simd::fmod(wavePhasor, 1000.f);
            wavePhasor = simd::ifelse(wavePhasor<0.f, wavePhasor+1000.f, wavePhasor);
            wavePhasor = sinSimd_1000(wavePhasor);
            switch (shapeMode) {
              case 0:  // exp/log
                out[s] = crossfade(wavePhasor, ifelse(shape>0.f, 11.f*wavePhasor/(10.f*simd::abs(wavePhasor)+1.f), simd::sgn(wavePhasor)*simd::pow(wavePhasor,4)), ifelse(shape>0.f, shape, -shape))*5.f;
                break;
              case 1:  // J curve
                out[s] = (normSigmoid((wavePhasor+1.f)/2.f, -shape*0.9f)*2.f-1.f) * 5.f;
                break;
              case 2: // S curve
                out[s] = normSigmoid(wavePhasor, -shape*0.9f) * 5.f;
                break;
              default: // 3 Rectify, 4 Normalized Rectify
                shape = -shape;
                shapeSign = simd::sgn(shape);
                out[s] = simd::ifelse(shapeSign==0, wavePhasor, -(shapeSign*simd::abs(-wavePhasor+shapeSign-shape)-shapeSign+shape));
                if (shapeMode==4) // Normalized rectify
                  out[s] = -((1+simd::abs(shape))*-out[s]-shape);
                out[s] *= 5.f;
            }
            break;
          case 1: // TRI
            wavePhasor = phasor[s] + (phaseIn*params[PHASE_AMT_PARAM].getValue() + params[PHASE_PARAM].getValue()*2.f)*250.f + 250.f;
            wavePhasor = simd::fmod(wavePhasor, 1000.f);
            wavePhasor = simd::ifelse(wavePhasor<0.f, wavePhasor+1000.f, wavePhasor);
            if (shapeMode<=2) shape = simd::ifelse(wavePhasor<500.f, shape, -shape);
            wavePhasor = simd::ifelse(wavePhasor<500.f, wavePhasor*.002f, (1000.f-wavePhasor)*.002f);
            switch (shapeMode) {
              case 0:  // exp/log
                out[s] = crossfade(wavePhasor, ifelse(shape>0.f, 11.f*wavePhasor/(10.f*simd::abs(wavePhasor)+1.f), simd::sgn(wavePhasor)*simd::pow(wavePhasor,4)), ifelse(shape>0.f, shape, -shape))*10.f-5.f;
                break;
              case 1:  // J curve
                out[s] = normSigmoid(wavePhasor, -shape*0.8) * 10.f - 5.f;
                break;
              case 2: // S curve
                out[s] = normSigmoid(wavePhasor*2.f-1.f, -shape*0.8) * 5.f;
                break;
              case 3: // Rectify
              case 4: // Normalized Rectify
                shape = -shape;
                shapeSign = simd::sgn(shape);
                out[s] = wavePhasor*2.f-1.f;
                out[s] = simd::ifelse(shapeSign==0, out[s], -(shapeSign*simd::abs(-out[s]+shapeSign-shape)-shapeSign+shape));
                if (shapeMode==4) // Normalized Rectify
                  out[s] = -((1+simd::abs(shape))*-out[s]-shape);
                out[s] *= 5.f;
                break;
              default: // 5 morph sine <--> triangle <--> square
                out[s] = (wavePhasor*10.f - 5.f) * (1.f - simd::abs(shape)); // triangle component
                // sine and square components
                wavePhasor = phasor[s] + (phaseIn*params[PHASE_AMT_PARAM].getValue() + params[PHASE_PARAM].getValue()*2.f)*250.f;
                wavePhasor = simd::fmod(wavePhasor - simd::ifelse(shape<=0.f, 250.f, 0.f), 1000.f);
                wavePhasor = simd::ifelse(wavePhasor<0.f, wavePhasor+1000.f, wavePhasor);
                out[s] += simd::ifelse( shape<=0.f,
                                        sinSimd_1000(wavePhasor)*5.f * -shape, // sine component
                                        simd::ifelse(wavePhasor<500.f, 5.f, -5.f) * shape // square component
                                      );
            } // end triangle shape switch
            break;
          case 2: // SQR
            wavePhasor = phasor[s] + (phaseIn*params[PHASE_AMT_PARAM].getValue() + params[PHASE_PARAM].getValue()*2.f)*250.f;
            wavePhasor = simd::fmod(wavePhasor, 1000.f);
            wavePhasor = simd::ifelse(wavePhasor<0.f, wavePhasor+1000.f, wavePhasor);
            if (shapeMode==2) { // morph tri <--> sqr <--> saw
              out[s] = simd::ifelse(wavePhasor<500.f, 5.f, -5.f) * (1.f - simd::abs(shape)); // square component
              // triangle and saw components
              wavePhasor = phasor[s] + (phaseIn*params[PHASE_AMT_PARAM].getValue() + params[PHASE_PARAM].getValue()*2.f)*250.f;
              wavePhasor = simd::fmod(wavePhasor + simd::ifelse(shape<=0.f, 250.f, 500.f), 1000.f);
              wavePhasor = simd::ifelse(wavePhasor<0.f, wavePhasor+1000.f, wavePhasor);
              out[s] += simd::ifelse( shape<=0.f, 
                                      (simd::ifelse(wavePhasor<500.f, wavePhasor, (1000.f-wavePhasor))*.02f - 5.f) * -shape, // triangle component
                                      (wavePhasor*0.01f - 5.f) * shape // saw component
                                    );
                                     
            } else { // PWM
              float_4 flip = (shapeIn*params[SHAPE_AMT_PARAM].getValue()*shpScale + params[SHAPE_PARAM].getValue() + 1.f) * 500.f;
              if (!shapeMode) flip = clamp( flip, 30.f, 970.f );
              out[s] = simd::ifelse(wavePhasor<flip, 5.f, -5.f);
            }
            break;
          default: // 3 SAW
            wavePhasor = phasor[s] + (phaseIn*params[PHASE_AMT_PARAM].getValue() + params[PHASE_PARAM].getValue()*2.f)*250.f;
            wavePhasor = simd::fmod(wavePhasor, 1000.f);
            wavePhasor = simd::ifelse(wavePhasor<0.f, wavePhasor+1000.f, wavePhasor);
            wavePhasor *= 0.001f;
            switch (shapeMode) {
              case 0:  // exp/log
                out[s] = crossfade(wavePhasor, ifelse(shape>0.f, 11.f*wavePhasor/(10.f*simd::abs(wavePhasor)+1.f), simd::sgn(wavePhasor)*simd::pow(wavePhasor,4)), ifelse(shape>0.f, shape, -shape))*10.f-5.f;
                break;
              case 1:  // J Curve
                out[s] = normSigmoid(wavePhasor, -shape*0.90) * 10.f - 5.f;
                break;
              case 2: // S Curve
                out[s] = normSigmoid(wavePhasor*2.f-1.f, -shape*0.85) * 5.f;
                break;
              case 3: // Rectify
              case 4: // Normalized Rectify
                shape = -shape;
                shapeSign = simd::sgn(shape);
                out[s] = wavePhasor*2.f-1.f;
                out[s] = simd::ifelse(shapeSign==0, out[s], -(shapeSign*simd::abs(-out[s]+shapeSign-shape)-shapeSign+shape));
                if (shapeMode==4) // Normalized Rectify
                  out[s] = -((1+simd::abs(shape))*-out[s]-shape);
                out[s] *= 5.f;
                break;
              default: // 5 morph square <--> saw <--> even
                out[s] = (wavePhasor*10.f - 5.f) * simd::ifelse(shape<0.f, 1.f + shape, 1.f); // saw component
                // square component
                wavePhasor = phasor[s] + (phaseIn*params[PHASE_AMT_PARAM].getValue() + params[PHASE_PARAM].getValue()*2.f)*250.f;
                wavePhasor = simd::fmod(wavePhasor + simd::ifelse(shape<=0.f, 500.f, 0.f), 1000.f);
                wavePhasor = simd::ifelse(wavePhasor<0.f, wavePhasor+1000.f, wavePhasor);
                out[s] += simd::ifelse(wavePhasor<500.f, 5.f, -5.f) * simd::abs(shape) * simd::ifelse(shape<0.f, 1.f, 0.5f);
                // sine component
                wavePhasor = phasor[s] + (phaseIn*params[PHASE_AMT_PARAM].getValue() + params[PHASE_PARAM].getValue()*2.f)*250.f;
                wavePhasor = simd::fmod(wavePhasor, 1000.f);
                wavePhasor = simd::ifelse(wavePhasor<0.f, wavePhasor+1000.f, wavePhasor);
                out[s] += simd::ifelse(shape<0.f, 0.f, sinSimd_1000(wavePhasor) * 3.175 * shape);
            } // end saw shape switch
        } // end wave switch

        if (s==0 || inputs[LEVEL_INPUT].isPolyphonic()) {
          levelIn = (o && !disableOver[LEVEL_INPUT]) ? float_4::zero() : inputs[LEVEL_INPUT].getPolyVoltageSimd<float_4>(c);
          if (procOver[LEVEL_INPUT]){
            if (o==0) levelIn *= oversample;
            levelIn = levelUpSample[s].process(levelIn);
          }
        } // else preserve prior levelIn value
        level = bipolar ? levelIn : simd::ifelse(levelIn>0.f, levelIn, 0.f);
        level = level * params[LEVEL_AMT_PARAM].getValue() * lvlScale + params[LEVEL_PARAM].getValue();
        if (clampLevel)
          level = simd::clamp(level, -1.f, 1.f);

        if (s==0 || inputs[OFFSET_INPUT].isPolyphonic()) {
          offsetIn = (o && !disableOver[OFFSET_INPUT]) ? float_4::zero() : inputs[OFFSET_INPUT].getPolyVoltageSimd<float_4>(c);
          if (procOver[OFFSET_INPUT]){
            if (o==0) offsetIn *= oversample;
            offsetIn = offsetUpSample[s].process(offsetIn);
          }
        } // else preserve prior offsetIn[SIN] value
        out[s] += clamp(offsetIn*params[OFFSET_AMT_PARAM].getValue() + params[OFFSET_PARAM].getValue()*5.f, -5.f, 5.f);
        out[s] *= level;  

        // Handle one shots
        if (once){
          out[s] = simd::ifelse(onceActive[s]==float_4::zero(), float_4::zero(), out[s]);
        }
        // Remove DC offset
        if (params[DC_PARAM].getValue()) {
          out[s] = outDcBlockFilter[s].process(out[s]); //, oversample);
        }
        // Downsample outputs
        if (oversample>1) {
          out[s] = outDownSample[s].process(out[s]);
        }
      }
    }
    
    // Write output
    for (int s=0, c=0; s<simdCnt; s++, c+=4) {
      outputs[OUTPUT].setVoltageSimd( out[s], c );
    }
    outputs[OUTPUT].setChannels(channels);
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_t* array = json_array();
    for (int i=0; i<INPUTS_LEN; i++)
      json_array_append_new(array, json_boolean(disableOver[i]));
    json_object_set_new(rootJ, "disableOver", array);
    json_object_set_new(rootJ, "unity5", json_boolean(unity5));
    json_object_set_new(rootJ, "bipolar", json_boolean(bipolar));
    json_object_set_new(rootJ, "linDCCouple", json_boolean(linDCCouple));
    json_object_set_new(rootJ, "overParam", json_integer(params[OVER_PARAM].getValue()));
    json_object_set_new(rootJ, "clampLevel", json_boolean(clampLevel));
    json_object_set_new(rootJ, "shapeModeParam", json_integer(params[SHAPE_MODE_PARAM].getValue()));
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
    if ((val = json_object_get(rootJ, "unity5"))) {
      setUnity5(json_boolean_value(val));
    }
    if ((val = json_object_get(rootJ, "bipolar"))) {
      setBipolar(json_boolean_value(val));
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
    setWave();
    if ((val = json_object_get(rootJ, "shapeModeParam"))) {
      params[SHAPE_MODE_PARAM].setValue(json_integer_value(val));
    }
  }
  
};

struct VCOUnitWidget : VenomWidget {
  
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

  struct DCBlockSwitch : GlowingSvgSwitchLockable {
    DCBlockSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
    }
  };

  struct WaveSwitch : GlowingSvgSwitchLockable {
    WaveSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/wave_sin.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/wave_tri.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/wave_sqr.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/wave_saw.svg")));
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

  struct OverPort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      VCOUnit* module = static_cast<VCOUnit*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem("Disable oversampling", "", &module->disableOver[portId]));
      PolyPort::appendContextMenu(menu);
    }
  };
  
  struct LinPort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      VCOUnit* module = static_cast<VCOUnit*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem("Disable oversampling", "", &module->disableOver[portId]));
      menu->addChild(createBoolPtrMenuItem("DC coupled", "", &module->linDCCouple));
      PolyPort::appendContextMenu(menu);
    }
  };

  struct LevelPort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      VCOUnit* module = static_cast<VCOUnit*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem("Disable oversampling", "", &module->disableOver[portId]));
      menu->addChild(createBoolMenuItem(
        "VCA unity = 5V", "",
        [=]() {return module->unity5;},
        [=](bool rm) {
          module->setUnity5(rm);
        }
      ));
      menu->addChild(createBoolMenuItem(
        "Bipolar VCA (ring mod)", "",
        [=]() {return module->bipolar;},
        [=](bool val) {
          module->setBipolar(val);
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

  VCOUnitWidget(VCOUnit* module) {
    setModule(module);
    setVenomPanel("VCOUnit");
    
    addParam(createLockableParamCentered<ModeSwitch>(Vec(24.f,45.5f), module, VCOUnit::MODE_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(111.f,45.5f), module, VCOUnit::OVER_PARAM));
    addParam(createLockableParamCentered<DCBlockSwitch>(Vec(24.f,75.5f), module, VCOUnit::DC_PARAM));
    addParam(createLockableParamCentered<GlowingTinyButtonLockable>(Vec(111.f,75.5f), module, VCOUnit::RESET_POLY_PARAM));

    addParam(createLockableParamCentered<RoundLargeBlackKnobLockable>(Vec(67.5f,63.5f), module, VCOUnit::FREQ_PARAM));

    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(Vec(24.f,116.f), module, VCOUnit::OCTAVE_PARAM));
    addParam(createLockableParamCentered<WaveSwitch>(Vec(67.5f,113.f), module, VCOUnit::WAVE_PARAM));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(24.f,162.f), module, VCOUnit::EXP_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(67.5f,162.f), module, VCOUnit::LIN_PARAM));

    addInput(createOverInputCentered<OverPort>(Vec(24.f, 197.5f), module, VCOUnit::EXP_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(37.5f, 186.f), module, VCOUnit::EXP_LIGHT));
    addInput(createOverInputCentered<LinPort>(Vec(67.5f, 197.5f), module, VCOUnit::LIN_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(81.f, 186.f), module, VCOUnit::LIN_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<RedLight>>(Vec(81.f, 209.f), module, VCOUnit::LIN_DC_LIGHT));

    addInput(createInputCentered<PolyPort>(Vec(24.f, 244.5f), module, VCOUnit::EXP_DEPTH_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(67.5f, 244.5f), module, VCOUnit::LIN_DEPTH_INPUT));

    addInput(createOverInputCentered<OverPort>(Vec(24.f, 293.5f), module, VCOUnit::SYNC_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(37.5f, 282.f), module, VCOUnit::SYNC_LIGHT));
    addInput(createOverInputCentered<OverPort>(Vec(67.5f, 293.5f), module, VCOUnit::REV_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(81.f, 282.f), module, VCOUnit::REV_LIGHT));

    addInput(createInputCentered<PolyPort>(Vec(24.f, 341.5f), module, VCOUnit::VOCT_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(67.5f,341.5f), module, VCOUnit::OUTPUT));
    
    addParam(createLockableParamCentered<ShpSwitch>(Vec(111.f,105.5f), module, VCOUnit::SHAPE_MODE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(100.5f,120.5f), module, VCOUnit::SHAPE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(121.5f,120.5f), module, VCOUnit::SHAPE_AMT_PARAM));
    addInput(createOverInputCentered<OverPort>(Vec(111.f,146.5f), module, VCOUnit::SHAPE_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(124.5f,135.f), module, VCOUnit::SHAPE_LIGHT));
    
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(100.5f,185.5f), module, VCOUnit::PHASE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(121.5f,185.5f), module, VCOUnit::PHASE_AMT_PARAM));
    addInput(createOverInputCentered<OverPort>(Vec(111.f,211.5f), module, VCOUnit::PHASE_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(124.5f,200.f), module, VCOUnit::PHASE_LIGHT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(100.5f,250.5f), module, VCOUnit::OFFSET_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(121.5f,250.5f), module, VCOUnit::OFFSET_AMT_PARAM));
    addInput(createOverInputCentered<OverPort>(Vec(111.f,276.5f), module, VCOUnit::OFFSET_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(124.5f,265.f), module, VCOUnit::OFFSET_LIGHT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(100.5f,315.5f), module, VCOUnit::LEVEL_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(121.5f,315.5f), module, VCOUnit::LEVEL_AMT_PARAM));
    addInput(createOverInputCentered<LevelPort>(Vec(111.f,341.5f), module, VCOUnit::LEVEL_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(124.5f,330.f), module, VCOUnit::LEVEL_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(97.5f,330.f), module, VCOUnit::RM_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(97.5f,353.f), module, VCOUnit::VCA_LIGHT));
  }

  void step() override {
    VenomWidget::step();
    VCOUnit* mod = dynamic_cast<VCOUnit*>(this->module);
    if(mod) {
      bool over = mod->params[VCOUnit::OVER_PARAM].getValue();
      mod->lights[VCOUnit::REV_LIGHT].setBrightness(over && !(mod->disableOver[VCOUnit::REV_INPUT]) && mod->inputs[VCOUnit::REV_INPUT].isConnected());
      mod->lights[VCOUnit::REV_LIGHT+1].setBrightness(over && mod->disableOver[VCOUnit::REV_INPUT] && mod->inputs[VCOUnit::REV_INPUT].isConnected());
      mod->lights[VCOUnit::EXP_LIGHT].setBrightness(over && !(mod->disableOver[VCOUnit::EXP_INPUT]) && mod->inputs[VCOUnit::EXP_INPUT].isConnected() && !(mod->alternate));
      mod->lights[VCOUnit::EXP_LIGHT+1].setBrightness(over && mod->disableOver[VCOUnit::EXP_INPUT] && mod->inputs[VCOUnit::EXP_INPUT].isConnected() && !(mod->alternate));
      mod->lights[VCOUnit::LIN_LIGHT].setBrightness(over && !(mod->disableOver[VCOUnit::LIN_INPUT]) && mod->inputs[VCOUnit::LIN_INPUT].isConnected());
      mod->lights[VCOUnit::LIN_LIGHT+1].setBrightness(over && mod->disableOver[VCOUnit::LIN_INPUT] && mod->inputs[VCOUnit::LIN_INPUT].isConnected());
      mod->lights[VCOUnit::SYNC_LIGHT].setBrightness(over && !(mod->disableOver[VCOUnit::SYNC_INPUT]) && mod->inputs[VCOUnit::SYNC_INPUT].isConnected());
      mod->lights[VCOUnit::SYNC_LIGHT+1].setBrightness(over && mod->disableOver[VCOUnit::SYNC_INPUT] && mod->inputs[VCOUnit::SYNC_INPUT].isConnected());
      mod->lights[VCOUnit::SHAPE_LIGHT].setBrightness(over && !(mod->disableOver[VCOUnit::SHAPE_INPUT]) && mod->inputs[VCOUnit::SHAPE_INPUT].isConnected());
      mod->lights[VCOUnit::SHAPE_LIGHT+1].setBrightness(over && mod->disableOver[VCOUnit::SHAPE_INPUT] && mod->inputs[VCOUnit::SHAPE_INPUT].isConnected());
      mod->lights[VCOUnit::PHASE_LIGHT].setBrightness(over && !(mod->disableOver[VCOUnit::PHASE_INPUT]) && mod->inputs[VCOUnit::PHASE_INPUT].isConnected());
      mod->lights[VCOUnit::PHASE_LIGHT+1].setBrightness(over && mod->disableOver[VCOUnit::PHASE_INPUT] && mod->inputs[VCOUnit::PHASE_INPUT].isConnected());
      mod->lights[VCOUnit::OFFSET_LIGHT].setBrightness(over && !(mod->disableOver[VCOUnit::OFFSET_INPUT]) && mod->inputs[VCOUnit::OFFSET_INPUT].isConnected());
      mod->lights[VCOUnit::OFFSET_LIGHT+1].setBrightness(over && mod->disableOver[VCOUnit::OFFSET_INPUT] && mod->inputs[VCOUnit::OFFSET_INPUT].isConnected());
      mod->lights[VCOUnit::LEVEL_LIGHT].setBrightness(over && !(mod->disableOver[VCOUnit::LEVEL_INPUT]) && mod->inputs[VCOUnit::LEVEL_INPUT].isConnected());
      mod->lights[VCOUnit::LEVEL_LIGHT+1].setBrightness(over && mod->disableOver[VCOUnit::LEVEL_INPUT] && mod->inputs[VCOUnit::LEVEL_INPUT].isConnected());
      mod->lights[VCOUnit::LIN_DC_LIGHT].setBrightness(mod->linDCCouple);
    }
  }

  void appendContextMenu(Menu* menu) override {
    VCOUnit* module = dynamic_cast<VCOUnit*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolPtrMenuItem("Limit level to 100%", "", &module->clampLevel));
    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelVCOUnit = createModel<VCOUnit, VCOUnitWidget>("VCOUnit");

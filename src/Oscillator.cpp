// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"
#include <float.h>

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

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

struct Oscillator : VenomModule {
 
  enum ParamId {
    MODE_PARAM,
    DC_PARAM,
    CLIP_PARAM,
    OVER_PARAM,
    FREQ_PARAM,
    OCTAVE_PARAM,
    SOFT_PARAM,
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

    SIN_ASIGN_PARAM,
    TRI_ASIGN_PARAM,
    SQR_ASIGN_PARAM,
    SAW_ASIGN_PARAM,

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
    
    SIN_LEVEL_INPUT,
    TRI_LEVEL_INPUT,
    SQR_LEVEL_INPUT,
    SAW_LEVEL_INPUT,
    MIX_LEVEL_INPUT,
    
    SIN_OFFSET_INPUT,
    TRI_OFFSET_INPUT,
    SQR_OFFSET_INPUT,
    SAW_OFFSET_INPUT,
    MIX_OFFSET_INPUT,

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
    SOFT_LIGHT,
    EXP_LIGHT,
    LIN_LIGHT,
    SYNC_LIGHT,
    
    SIN_SHAPE_LIGHT,
    TRI_SHAPE_LIGHT,
    SQR_SHAPE_LIGHT,
    SAW_SHAPE_LIGHT,
    MIX_SHAPE_LIGHT,

    SIN_PHASE_LIGHT,
    TRI_PHASE_LIGHT,
    SQR_PHASE_LIGHT,
    SAW_PHASE_LIGHT,
    MIX_PHASE_LIGHT,

    SIN_OFFSET_LIGHT,
    TRI_OFFSET_LIGHT,
    SQR_OFFSET_LIGHT,
    SAW_OFFSET_LIGHT,
    MIX_OFFSET_LIGHT,

    SIN_LEVEL_LIGHT,
    TRI_LEVEL_LIGHT,
    SQR_LEVEL_LIGHT,
    SAW_LEVEL_LIGHT,
    MIX_LEVEL_LIGHT,

    LIGHTS_LEN
  };

  using float_4 = simd::float_4;
  int oversample = -1;
  std::vector<int> oversampleValues = {1,2,4,8,16,32};
  OversampleFilter_4 expUpSample[4], linUpSample[4], syncUpSample[4],
                     shapeUpSample[4][5], phaseUpSample[4][5], offsetUpSample[4][5], levelUpSample[4][5],
                     outDownSample[4][5];
  float_4 phasor[4]{}, phasorDir[4]{1.f, 1.f, 1.f, 1.f};
  dsp::SchmittTrigger syncTrig[16];
  
  Oscillator() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 2.f, 0.f, "Mode", {"Audio frequency", "Low frequency", "0Hz carrier"});
    configSwitch<FixedSwitchQuantity>(DC_PARAM,   0.f, 1.f, 0.f, "Mix DC Block", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 4.f, 0.f, "Mix Clipping", {"Off", "Hard post-level", "Soft post-level", "Hard pre-level", "Soft pre-level"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 3.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    
    configParam(FREQ_PARAM, -4.f, 4.f, 0.f, "Frequency");
    configParam(OCTAVE_PARAM, -4.f, 4.f, 0.f, "Octave");
    configSwitch<FixedSwitchQuantity>(SOFT_PARAM, 0.f, 1.f, 0.f, "Sync mode", {"Hard", "Soft"});
    configParam(EXP_PARAM, -1.f, 1.f, 0.f, "Exponential FM");
    configParam(LIN_PARAM, -1.f, 1.f, 0.f, "Linear FM");
    configInput(EXP_INPUT, "Exponential FM input");
    configLight(EXP_LIGHT, "Exponential FM oversample indicator");
    configInput(LIN_INPUT, "Linear FM input");
    configLight(LIN_LIGHT, "Linear FM oversample indicator");
    configInput(EXP_DEPTH_INPUT, "Exponential FM depth input");
    configInput(LIN_DEPTH_INPUT, "Linear FM depth input");
    configInput(VOCT_INPUT, "V/Oct");
    configInput(SYNC_INPUT, "Sync");
    configLight(SYNC_LIGHT, "Sync oversample indicator");

    std::string xStr[5]{"Sine","Triangle","Square","Saw","Mix"};
    std::string yStr[4]{" shape"," phase"," offset"," level"};
    for (int y=0; y<4; y++){
      for (int x=0; x<5; x++){
        configParam(GRID_PARAM+y*10+x, -1.f, 1.f, 0.f, xStr[x]+yStr[y]);
        configParam(GRID_PARAM+y*10+x+5, -1.f, 1.f, 0.f, xStr[x]+yStr[y]+" CV amount");
        configInput(GRID_INPUT+y*5+x, xStr[x]+yStr[y]+" CV");
        configLight(GRID_LIGHT+y*5+x, xStr[x]+yStr[y]+" oversample indicator");
      }
    }
    for (int x=0; x<4; x++){
      configSwitch<FixedSwitchQuantity>(ASGN_PARAM+x, 0.f, 2.f, 0.f, xStr[x]+" level assignment", {"Mix output", xStr[x]+" output", "Both "+xStr[x]+" mix output"});
    }
    for (int x=0; x<5; x++){
      configOutput(GRID_OUTPUT+x, xStr[x]);
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (oversample != oversampleValues[params[OVER_PARAM].getValue()]) {
      oversample = oversampleValues[params[OVER_PARAM].getValue()];
      for (int i=0; i<4; i++){
        expUpSample[i].setOversample(oversample);
        linUpSample[i].setOversample(oversample);
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
    for (int i=0; i<INPUTS_LEN; i++) {
      int c = inputs[i].getChannels();
      if (c>channels)
        channels = c;
    }
    int simdCnt = (channels+3)/4;
    
    float_4 expIn[4]{}, linIn[4]{}, expDepthIn[4]{}, linDepthIn[4]{}, vOctIn[4]{}, syncIn[4]{}, freq[4]{},
            shapeIn[4][5]{}, phaseIn[4][5]{}, offsetIn[4][5]{}, levelIn[4][5]{},
            sinOut[4]{}, triOut[4]{}, sqrOut[4]{}, sawOut[4]{}, mixOut[4]{},
            sinPhasor{}, triPhasor{}, sqrPhasor{}, sawPhasor{}, globalPhasor{};
    float vOctParm = params[FREQ_PARAM].getValue() + params[OCTAVE_PARAM].getValue(),
          k =  1000.f * dsp::FREQ_C4 * args.sampleTime / oversample;
    bool soft = params[SOFT_PARAM].getValue();
    // main loops
    for (int o=0; o<oversample; o++){
      for (int s=0, c=0; s<simdCnt; s++, c+=4){
        if (!o) {
          if ((s==0 || inputs[EXP_DEPTH_INPUT].isPolyphonic())) {
            expDepthIn[s] = simd::clamp( inputs[EXP_DEPTH_INPUT].getNormalPolyVoltageSimd<float_4>(5.f,c)/5.f, -1.f, 1.f);
          } else expDepthIn[s] = expDepthIn[0];
          if (s==0 || inputs[LIN_DEPTH_INPUT].isPolyphonic()) {
            linDepthIn[s] = simd::clamp( inputs[LIN_DEPTH_INPUT].getNormalPolyVoltageSimd<float_4>(5.f,c)/5.f, -1.f, 1.f);
          } else linDepthIn[s] = linDepthIn[0];
          if (s==0 || inputs[VOCT_INPUT].isPolyphonic()) {
            vOctIn[s] = inputs[VOCT_INPUT].getPolyVoltageSimd<float_4>(c);
          } else vOctIn[s] = vOctIn[0];
        }
        if (s==0 || inputs[EXP_INPUT].isPolyphonic()) {
          expIn[s] = o ? float_4::zero() : inputs[EXP_INPUT].getPolyVoltageSimd<float_4>(c);
          if (oversample>1 && inputs[EXP_INPUT].isConnected()){
            if (o==0) expIn[s] *= oversample;
            expIn[s] = expUpSample[s].process(expIn[s]);
          }
        } else expIn[s] = expIn[0];
        if (s==0 || inputs[LIN_INPUT].isPolyphonic()) {
          linIn[s] = o ? float_4::zero() : inputs[LIN_INPUT].getPolyVoltageSimd<float_4>(c);
          if (oversample>1 && inputs[LIN_INPUT].isConnected()){
            if (o==0) linIn[s] *= oversample;
            linIn[s] = linUpSample[s].process(linIn[s]);
          }
        } else linIn[s] = linIn[0];
        if (s==0 || inputs[MIX_PHASE_INPUT].isPolyphonic()) {
          phaseIn[s][MIX] = o ? float_4::zero() : inputs[MIX_PHASE_INPUT].getPolyVoltageSimd<float_4>(c);
          if (oversample>1 && inputs[MIX_PHASE_INPUT].isConnected()){
            if (o==0) phaseIn[s][MIX] *= oversample;
            phaseIn[s][MIX] = phaseUpSample[s][MIX].process(phaseIn[s][MIX]);
          }
        } else phaseIn[s][MIX] = phaseIn[0][MIX];
        float_4 sync{};
        if (inputs[SYNC_INPUT].isConnected()) {
          if (s==0 || inputs[SYNC_INPUT].isPolyphonic()) {
            syncIn[s] = o ? float_4::zero() : inputs[SYNC_INPUT].getPolyVoltageSimd<float_4>(c);
            if (oversample>1 && inputs[SYNC_INPUT].isConnected()){
              if (o==0) syncIn[s] *= oversample;
              syncIn[s] = syncUpSample[s].process(syncIn[s]);
            }
          } else syncIn[s] = syncIn[0];
          for (int i=0; i<4; i++){
            sync[i] = syncTrig[c+i].process(syncIn[s][i], 0.2f, 2.f);
          }
        }
        freq[s] = vOctIn[s] + vOctParm + expIn[s]*expDepthIn[s]*params[EXP_PARAM].getValue();
        freq[s] = simd::pow(2.f, freq[s]) + linIn[s]*linDepthIn[s]*params[LIN_PARAM].getValue();
        phasorDir[s] = simd::ifelse(sync>(soft ? 0.f : 10.f), phasorDir[s]*-1.f, phasorDir[s]);
        phasor[s] += freq[s] * phasorDir[s] * k;
        phasor[s] = simd::fmod(phasor[s], 1000.f);
        phasor[s] = simd::ifelse(phasor[s]<0.f, phasor[s]+1000.f, phasor[s]);
        phasor[s] = simd::ifelse(sync>(soft ? 10.f : 0.f), float_4::zero(), phasor[s]);

        // Global (Mix) Phase
        globalPhasor = phasor[s] + (phaseIn[s][MIX]*params[MIX_PHASE_AMT_PARAM].getValue() + params[MIX_PHASE_PARAM].getValue())*250.f;

        // Sine
        
        // Triangle
        if (s==0 || inputs[TRI_PHASE_INPUT].isPolyphonic()) {
          phaseIn[s][TRI] = o ? float_4::zero() : inputs[TRI_PHASE_INPUT].getPolyVoltageSimd<float_4>(c);
          if (oversample>1 && inputs[TRI_PHASE_INPUT].isConnected()){
            if (o==0) phaseIn[s][TRI] *= oversample;
            phaseIn[s][TRI] = phaseUpSample[s][TRI].process(phaseIn[s][TRI]);
          }
        } else phaseIn[s][TRI] = phaseIn[0][TRI];
        triPhasor = globalPhasor + (phaseIn[s][TRI]*params[TRI_PHASE_AMT_PARAM].getValue() + params[TRI_PHASE_PARAM].getValue())*250.f - 250.f;
        triPhasor = simd::fmod(triPhasor, 1000.f);
        triPhasor = simd::ifelse(triPhasor<0.f, triPhasor+1000.f, triPhasor);
        triOut[s] = simd::ifelse(triPhasor<500.f, -triPhasor*.02f + 5.f, (triPhasor-500.f)*.02f - 5.f);
        if (oversample>1) {
          triOut[s] = outDownSample[s][TRI].process(triOut[s]);
        }
        
        // Square
        if (s==0 || inputs[SQR_PHASE_INPUT].isPolyphonic()) {
          phaseIn[s][SQR] = o ? float_4::zero() : inputs[SQR_PHASE_INPUT].getPolyVoltageSimd<float_4>(c);
          if (oversample>1 && inputs[SQR_PHASE_INPUT].isConnected()){
            if (o==0) phaseIn[s][SQR] *= oversample;
            phaseIn[s][SQR] = phaseUpSample[s][SQR].process(phaseIn[s][SQR]);
          }
        } else phaseIn[s][SQR] = phaseIn[0][SQR];
        sqrPhasor = globalPhasor + (phaseIn[s][SQR]*params[SQR_PHASE_AMT_PARAM].getValue() + params[SQR_PHASE_PARAM].getValue())*250.f;
        sqrPhasor = simd::fmod(sqrPhasor, 1000.f);
        sqrPhasor = simd::ifelse(sqrPhasor<0.f, sqrPhasor+1000.f, sqrPhasor);
        sqrOut[s] = simd::ifelse(sqrPhasor<500.f, 5.f, -5.f);
        if (oversample>1) {
          sqrOut[s] = outDownSample[s][SQR].process(sqrOut[s]);
        }
        
        // Saw
        if (s==0 || inputs[SAW_PHASE_INPUT].isPolyphonic()) {
          phaseIn[s][SAW] = o ? float_4::zero() : inputs[SAW_PHASE_INPUT].getPolyVoltageSimd<float_4>(c);
          if (oversample>1 && inputs[SAW_PHASE_INPUT].isConnected()){
            if (o==0) phaseIn[s][SAW] *= oversample;
            phaseIn[s][SAW] = phaseUpSample[s][SAW].process(phaseIn[s][SAW]);
          }
        } else phaseIn[s][SAW] = phaseIn[0][SAW];
        sawPhasor = globalPhasor + (phaseIn[s][SAW]*params[SAW_PHASE_AMT_PARAM].getValue() + params[SAW_PHASE_PARAM].getValue())*250.f;
        sawPhasor = simd::fmod(sawPhasor, 1000.f);
        sawPhasor = simd::ifelse(sawPhasor<0.f, sawPhasor+1000.f, sawPhasor);
        sawOut[s] = sawPhasor*0.01f - 5.f;
        if (oversample>1) {
          sawOut[s] = outDownSample[s][SAW].process(sawOut[s]);
        }
      }
    }
    
    float_4 out{};
    for (int s=0, c=0; s<simdCnt; s++, c+=4) {
      outputs[TRI_OUTPUT].setVoltageSimd( triOut[s], c );
      outputs[SQR_OUTPUT].setVoltageSimd( sqrOut[s], c );
      outputs[SAW_OUTPUT].setVoltageSimd( sawOut[s], c );
    }
    outputs[TRI_OUTPUT].setChannels(channels);
    outputs[SQR_OUTPUT].setChannels(channels);
    outputs[SAW_OUTPUT].setChannels(channels);
  }
  
};

struct OscillatorWidget : VenomWidget {
  
  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
    }
  };

  struct ClipSwitch : GlowingSvgSwitchLockable {
    ClipSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  struct DCBlockSwitch : GlowingSvgSwitchLockable {
    DCBlockSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
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

  struct AssignSwitch : GlowingSvgSwitchLockable {
    AssignSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };

  OscillatorWidget(Oscillator* module) {
    setModule(module);
    setVenomPanel("Oscillator");
    
    addParam(createLockableParamCentered<ModeSwitch>(Vec(19.5f,41.5f), module, Oscillator::MODE_PARAM));
    addParam(createLockableParamCentered<DCBlockSwitch>(Vec(37.5f,41.5f), module, Oscillator::DC_PARAM));
    addParam(createLockableParamCentered<ClipSwitch>(Vec(55.5f,41.5f), module, Oscillator::CLIP_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(73.5f,41.5f), module, Oscillator::OVER_PARAM));
    
    addParam(createLockableParamCentered<RoundHugeBlackKnobLockable>(Vec(46.5f,93.5f), module, Oscillator::FREQ_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(29.f,157.f), module, Oscillator::OCTAVE_PARAM));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(64.f, 157.f), module, Oscillator::SOFT_PARAM, Oscillator::SOFT_LIGHT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(29.f,206.f), module, Oscillator::EXP_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(64.f,206.f), module, Oscillator::LIN_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(29.f, 241.5f), module, Oscillator::EXP_INPUT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(42.5f, 230.f), module, Oscillator::EXP_LIGHT));
    addInput(createInputCentered<PolyPort>(Vec(64.f, 241.5f), module, Oscillator::LIN_INPUT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(77.5f, 230.f), module, Oscillator::LIN_LIGHT));
    addInput(createInputCentered<PolyPort>(Vec(29.f, 290.5f), module, Oscillator::EXP_DEPTH_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(64.f, 290.5f), module, Oscillator::LIN_DEPTH_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(29.f, 335.5f), module, Oscillator::VOCT_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(64.f, 335.5f), module, Oscillator::SYNC_INPUT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(77.5f, 324.f), module, Oscillator::SYNC_LIGHT));
    
    float dx = 45.f;
    float dy = 61.f;
    for (int y=0; y<4; y++) {
      for (int x=0; x<5; x++) {
        addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(119.5f+dx*x,59.5f+dy*y), module, Oscillator::GRID_PARAM+y*10+x));
        addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(140.5f+dx*x,59.5f+dy*y), module, Oscillator::GRID_PARAM+y*10+x+5));
        addInput(createInputCentered<PolyPort>(Vec(130.f+dx*x,85.5f+dy*y), module, Oscillator::GRID_INPUT+y*5+x));
        addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(143.5f+dx*x, 74.f+dy*y), module, Oscillator::GRID_LIGHT+y*5+x));
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
      mod->lights[Oscillator::SOFT_LIGHT].setBrightness(mod->params[Oscillator::SOFT_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      bool over = mod->params[Oscillator::OVER_PARAM].getValue();
      mod->lights[Oscillator::EXP_LIGHT].setBrightness(over && mod->inputs[Oscillator::EXP_INPUT].isConnected());
      mod->lights[Oscillator::LIN_LIGHT].setBrightness(over && mod->inputs[Oscillator::LIN_INPUT].isConnected());
      mod->lights[Oscillator::SYNC_LIGHT].setBrightness(over && mod->inputs[Oscillator::SYNC_INPUT].isConnected());
      for (int y=0; y<4; y++) {
        for (int x=0; x<5; x++) {
          mod->lights[Oscillator::GRID_LIGHT+y*5+x].setBrightness(over && mod->inputs[Oscillator::GRID_INPUT+y*5+x].isConnected());
        }
      }
    }
  }
};

Model* modelOscillator = createModel<Oscillator, OscillatorWidget>("Oscillator");

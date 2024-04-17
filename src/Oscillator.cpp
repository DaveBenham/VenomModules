// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

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
    
    ENUMS(GRID_PARAM,40),
    ENUMS(ASGN_PARAM,4),

/*
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

    SIN_ASIGN_PARM,
    TRI_ASIGN_PARAM,
    SQR_ASIGN_PARAM,
    SAW_ASIGN_PARAM,
*/
    PARAMS_LEN
  };
  enum InputId {
    EXP_INPUT,
    LIN_INPUT,
    EXP_DEPTH_INPUT,
    LIN_DEPTH_INPUT,
    VOCT_INPUT,
    SYNC_INPUT,
    
    ENUMS(GRID_INPUT,20),
/*
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
*/    
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(GRID_OUTPUT,5),
/*    
    SIN_OUTPUT,
    TRI_OUTPUT,
    SQR_OUTPUT,
    SAW_OUTPUT,
    MIX_OUTPUT,
*/    
    OUTPUTS_LEN
  };
  enum LightId {
    SOFT_LIGHT,
    LIGHTS_LEN
  };
  
  Oscillator() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 2.f, 0.f, "Mode", {"Audio frequency", "Low frequency", "0Hz carrier"});
    configSwitch<FixedSwitchQuantity>(DC_PARAM,   0.f, 1.f, 0.f, "Mix DC Block", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 4.f, 0.f, "Mix Clipping", {"Off", "Hard post-level", "Soft post-level", "Hard pre-level", "Soft pre-level"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 3.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    
    configParam(FREQ_PARAM, -1.f, 1.f, 0.f, "Frequency");
    configParam(OCTAVE_PARAM, -4.f, 4.f, 0.f, "Octave");
    configSwitch<FixedSwitchQuantity>(SOFT_PARAM, 0.f, 1.f, 0.f, "Sync mode", {"Hard", "Soft"});
    configParam(EXP_PARAM, -1.f, 1.f, 0.f, "Exponential FM");
    configParam(LIN_PARAM, -1.f, 1.f, 0.f, "Linear FM");
    configInput(EXP_INPUT, "Exponential FM input");
    configInput(LIN_INPUT, "Linear FM input");
    configInput(EXP_DEPTH_INPUT, "Exponential FM depth input");
    configInput(LIN_DEPTH_INPUT, "Linear FM depth input");
    configInput(VOCT_INPUT, "V/Oct");
    configInput(SYNC_INPUT, "Sync");

    std::string xStr[5]{"Sine","Triangle","Square","Saw","Mix"};
    std::string yStr[4]{" shape"," phase"," offset"," level"};
    for (int y=0; y<4; y++){
      for (int x=0; x<5; x++){
        configParam(GRID_PARAM+y*10+x, -1.f, 1.f, 0.f, xStr[x]+yStr[y]);
        configParam(GRID_PARAM+y*10+x+5, -1.f, 1.f, 0.f, xStr[x]+yStr[y]+" CV amount");
        configInput(GRID_INPUT+y*5+x, xStr[x]+yStr[y]+" CV");
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
    addInput(createInputCentered<PolyPort>(Vec(64.f, 241.5f), module, Oscillator::LIN_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(29.f, 290.5f), module, Oscillator::EXP_DEPTH_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(64.f, 290.5f), module, Oscillator::LIN_DEPTH_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(29.f, 335.5f), module, Oscillator::VOCT_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(64.f, 335.5f), module, Oscillator::SYNC_INPUT));
    
    float dx = 45.f;
    float dy = 61.f;
    for (int y=0; y<4; y++) {
      for (int x=0; x<5; x++) {
        addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(119.5f+dx*x,59.5f+dy*y), module, Oscillator::GRID_PARAM+y*10+x));
        addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(140.5f+dx*x,59.5f+dy*y), module, Oscillator::GRID_PARAM+y*10+x+5));
        addInput(createInputCentered<PolyPort>(Vec(130.f+dx*x,85.5f+dy*y), module, Oscillator::GRID_INPUT+y*5+x));
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
    }
  }
};

Model* modelOscillator = createModel<Oscillator, OscillatorWidget>("Oscillator");

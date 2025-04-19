// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"

#define LIGHT_OFF 0.02f
#define LIGHT_ON  1.f

struct WaveMultiplier : VenomModule {

  enum ParamId {
    MASTER_PARAM,
    ENUMS(FREQ_PARAM,4),
    DEPTH_PARAM,
    DEPTH_CV_PARAM,
    ENUMS(SHIFT_CV_PARAM,4),
    ENUMS(SHIFT_PARAM,4),
    HPF_PARAM,
    LPF_PARAM,
    OVER_PARAM,
    ENUMS(MUTE_PARAM,4),
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
    HPF_LIGHT,
    LPF_LIGHT,
    ENUMS(MUTE_LIGHT,4),
    LIGHTS_LEN
  };
  
  int oversample = 0;
  int oversampleValues[6]{1,2,4,8,16,32};
/*
  OversampleFilter_4 preUpSample[4]{}, stageUpSample[4]{}, biasUpSample[4]{}, upSample[4]{}, downSample[4]{};
  float stageRaw = -1.f;
  simd::float_4 stageParm{};
*/


  WaveMultiplier() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam(MASTER_PARAM, -4.f, 4.f, -4.f, "Master modulation frequency");
    configInput(VOCT_INPUT, "Master modulation V/Oct");

    configInput(DEPTH_INPUT, "Shift depth CV");
    configParam(DEPTH_CV_PARAM, -1.f, 1.f, 0.f, "Shift depth CV amount", "%", 0, 100, 0);
    configParam(DEPTH_PARAM, 0.f, 5.f, 5.f, "Shift depth", " V");

    configSwitch<FixedSwitchQuantity>(HPF_PARAM, 0.f, 1.f, 1.f, "High-pass filter", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(LPF_PARAM, 0.f, 1.f, 1.f, "Low-pass filter", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 2.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});

    float freqDflt[4] {-0.72f, -0.27f, 0.19f, 0.88f}; 
    for (int i=0; i<4; i++) {
      std::string iStr = std::to_string(i+1);
      configParam(FREQ_PARAM+i, -1.f, 1.f, freqDflt[i], "Modulation frequency " + iStr, "%", 0, 100, 0);
      configOutput(MOD_OUTPUT+i, "Modulation " + iStr);
      configInput(SHIFT_INPUT+i, "Shift threshold CV " + iStr);
      configParam(SHIFT_CV_PARAM+i, -1.f, 1.f, 1.f, "Shift threshold CV " + iStr, "%", 0, 100, 0);
      configParam(SHIFT_PARAM+i, -5.f, 5.f, 0.f, "Shift threshold " + iStr, " V");
      configOutput(PULSE_OUTPUT+i, "Pulse " + iStr);
      configOutput(WAVE_OUTPUT+i, "Shifted wave " + iStr);
      configSwitch<FixedSwitchQuantity>(MUTE_PARAM+i, 0.f, 1.f, 0.f, "Mute " + iStr, {"Off", "On"});
    }

    configInput(WAVE_INPUT, "Wave");
    configInput(LEVEL_INPUT, "Shift depth CV");
    configParam(LEVEL_CV_PARAM, -1.f, 1.f, 0.f, "Level CV amount", "%", 0, 100, 0);
    configParam(LEVEL_PARAM, 0.f, 1.f, 1.f, "Level", "%", 0, 100, 0);
    configOutput(MIX_OUTPUT, "Shifted wave mix");

    configBypass(WAVE_INPUT, MIX_OUTPUT);
    
  }
  
/*
  void setOversample() override {
    if (oversample > 1) {
      for (int i=0; i<4; i++){
        preUpSample[i].setOversample(oversample, oversampleStages);
        stageUpSample[i].setOversample(oversample, oversampleStages);
        biasUpSample[i].setOversample(oversample, oversampleStages);
        upSample[i].setOversample(oversample, oversampleStages);
        downSample[i].setOversample(oversample, oversampleStages);
      }
    }
  }
*/  

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    
    //using float_4 = simd::float_4;
  }

/*
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "preAmpDisableOver", json_boolean(disableOver[PRE_INPUT]));
    json_object_set_new(rootJ, "preAmpBipolar", json_boolean(bipolar[PRE_INPUT]));
    json_object_set_new(rootJ, "stageAmpDisableOver", json_boolean(disableOver[STAGE_INPUT]));
    json_object_set_new(rootJ, "stageAmpBipolar", json_boolean(bipolar[STAGE_INPUT]));
    json_object_set_new(rootJ, "biasDisableOver", json_boolean(disableOver[BIAS_INPUT]));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "preAmpDisableOver"))) {
      disableOver[PRE_INPUT] = json_boolean_value(val);
    }
    if ((val = json_object_get(rootJ, "preAmpBipolar"))) {
      bipolar[PRE_INPUT] = json_boolean_value(val);
    }

    if ((val = json_object_get(rootJ, "stageAmpDisableOver"))) {
      disableOver[STAGE_INPUT] = json_boolean_value(val);
    }
    if ((val = json_object_get(rootJ, "stageAmpBipolar"))) {
      bipolar[STAGE_INPUT] = json_boolean_value(val);
    }

    if ((val = json_object_get(rootJ, "biasDisableOver"))) {
      disableOver[BIAS_INPUT] = json_boolean_value(val);
    }
  }
*/

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
    
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(17.5f,224.5f), module, WaveMultiplier::HPF_PARAM, WaveMultiplier::HPF_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(17.5f,257.5f), module, WaveMultiplier::LPF_PARAM, WaveMultiplier::LPF_LIGHT));
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
    addInput(createInputCentered<PolyPort>(Vec(67.f, 342.5f), module, WaveMultiplier::LEVEL_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(97.f, 342.5f), module, WaveMultiplier::LEVEL_CV_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(128.f, 342.5f), module, WaveMultiplier::LEVEL_PARAM));
    addOutput(createOutputCentered<PolyPort>(Vec(172.f, 342.5f), module, WaveMultiplier::MIX_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    if(this->module) {
      WaveMultiplier* mod = static_cast<WaveMultiplier*>(this->module);
      mod->lights[WaveMultiplier::HPF_LIGHT].setBrightness(mod->params[WaveMultiplier::HPF_PARAM].getValue());
      mod->lights[WaveMultiplier::LPF_LIGHT].setBrightness(mod->params[WaveMultiplier::LPF_PARAM].getValue());
      for (int i=0; i<4; i++)
        mod->lights[WaveMultiplier::MUTE_LIGHT+i].setBrightness(mod->params[WaveMultiplier::MUTE_PARAM+i].getValue());
    }
  }
};

Model* modelWaveMultiplier = createModel<WaveMultiplier, WaveMultiplierWidget>("WaveMultiplier");

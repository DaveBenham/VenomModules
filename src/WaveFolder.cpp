// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "math.hpp"
//#include <float.h>

struct WaveFolder : VenomModule {

  enum ParamId {
    STAGES_PARAM,
    OVER_PARAM,
    PRE_PARAM,
    STAGE_PARAM,
    BIAS_PARAM,
    PRE_AMT_PARAM,
    STAGE_AMT_PARAM,
    BIAS_AMT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    PRE_INPUT,
    STAGE_INPUT,
    BIAS_INPUT,
    POLY_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    POLY_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    PRE_VCA_LIGHT,
    STAGE_VCA_LIGHT,
    ENUMS(PRE_OVER_LIGHT,2),
    ENUMS(STAGE_OVER_LIGHT,2),
    ENUMS(BIAS_OVER_LIGHT,2),
    LIGHTS_LEN
  };
  
  int oversample = 0;
  int oversampleValues[6]{1,2,4,8,16,32};
  OversampleFilter_4 preUpSample[4]{}, stageUpSample[4]{}, biasUpSample[4]{}, upSample[4]{}, downSample[4]{};
  float stageRaw = -1.f;
  simd::float_4 stageParm{};
  bool disableOver[3]{}, bipolar[2]{};


  WaveFolder() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(STAGES_PARAM, 0.f, 4.f, 1.f, "Stages", {"2", "3", "4", "5", "6"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 2.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});

    configParam(PRE_PARAM, 0.f, 10.f, 1.f, "Pre-amp");
    configParam(STAGE_PARAM, -0.30103f, 1.f, 0.f, "Stage amp", "", 10, 1, 0);  // 0.5 - 10 range
    configParam(BIAS_PARAM, -5.f, 5.f, 0.f, "Bias", "V");
    
    configParam(PRE_AMT_PARAM, -1.f, 1.f, 0.f, "Pre-amp CV amount", "%", 0, 100, 0);
    configParam(STAGE_AMT_PARAM, -1.f, 1.f, 0.f, "Stage amp CV amount", "%", 0, 100, 0);
    configParam(BIAS_AMT_PARAM, -1.f, 1.f, 0.f, "Bias CV amount", "%", 0, 100, 0);

    configInput(PRE_INPUT, "Pre-amp CV");
    configLight(PRE_OVER_LIGHT, "Pre-amp CV oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";
    configLight(PRE_VCA_LIGHT, "Pre-amp bipolar VCA indicator");

    configInput(STAGE_INPUT, "Stage amp CV");
    configLight(STAGE_OVER_LIGHT, "Stage amp CV oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";
    configLight(STAGE_VCA_LIGHT, "Stage amp bipolar VCA indicator");

    configInput(BIAS_INPUT, "Bias CV");
    configLight(BIAS_OVER_LIGHT, "Bias CV oversample indicator")->description = "off = none, yellow = oversampled, red = disabled";

    configInput(POLY_INPUT, "Audio In");
    configOutput(POLY_OUTPUT, "Audio Out");

    configBypass(POLY_INPUT, POLY_OUTPUT);
    
    oversampleStages = 5;
  }
  
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

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    
    using float_4 = simd::float_4;
    float limit = 10.f / 6.f;
    if (oversample != oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())]) {
      oversample = oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())];
      setOversample();
    }
    
    if (stageRaw != params[STAGE_PARAM].getValue()) {
      stageRaw = params[STAGE_PARAM].getValue();
      stageParm = pow(10.f, stageRaw);
    }
    float preParm = params[PRE_PARAM].getValue(),
          preAmt = params[PRE_AMT_PARAM].getValue(),
          stageAmt = params[STAGE_AMT_PARAM].getValue(),
          biasParm = params[BIAS_PARAM].getValue(),
          biasAmt = params[BIAS_AMT_PARAM].getValue();
    bool preOver = inputs[PRE_INPUT].isConnected() && !disableOver[PRE_INPUT] && oversample>1,
         stageOver = inputs[STAGE_INPUT].isConnected() && !disableOver[STAGE_INPUT] && oversample>1,
         biasOver = inputs[BIAS_INPUT].isConnected() && !disableOver[BIAS_INPUT] && oversample>1;
    
    int stages = static_cast<int>(params[STAGES_PARAM].getValue())+2;
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++)
      channels = std::max({channels, inputs[i].getChannels()});
    
    float_4 in[4]{}, out[4]{}, pre[4]{}, stage[4]{}, bias[4]{};
    for (int o=0; o<oversample; o++) {
      for (int i=0, c=0; c<channels; i++, c+=4){
        if (!o) {
          pre[i] = inputs[PRE_INPUT].getPolyVoltageSimd<float_4>(c);
          stage[i] = inputs[STAGE_INPUT].getPolyVoltageSimd<float_4>(c);
          bias[i] = inputs[BIAS_INPUT].getPolyVoltageSimd<float_4>(c);
          in[i] = inputs[POLY_INPUT].getPolyVoltageSimd<float_4>(c) * oversample;
        }
        if (oversample > 1) {
          in[i] = upSample[i].process(o ? float_4::zero() : in[i]);
        if (preOver)
            pre[i] = preUpSample[i].process(o ? float_4::zero() : pre[i]*oversample);
        if (stageOver)
          stage[i] = stageUpSample[i].process(o ? float_4::zero() : stage[i]*oversample);
        if (biasOver)
          bias[i] = biasUpSample[i].process(o ? float_4::zero() : bias[i]*oversample);
        }
        if (!o || preOver) {
          pre[i] = pre[i] * preAmt + preParm;
          if (!bipolar[PRE_INPUT])
            pre[i] = ifelse(pre[i]<0.f, 0.f, pre[i]);
        }
        if (!o || stageOver) {
          stage[i] = stage[i] * stageAmt + stageParm;
          if (!bipolar[STAGE_INPUT])
            stage[i] = ifelse(stage[i]<0.f, 0.f, stage[i]);
        }
        if (!o || biasOver)
          bias[i] = bias[i] * biasAmt + biasParm;
        out[i] = (in[i] + bias[i]) * pre[i];
        for (int s=0; s<stages; s++)
          out[i] = simd::clamp( out[i] * stage[i], -5.f, 5.f) * 2.f - out[i];
        out[i] = softClip(out[i]*limit) / limit;
        if (oversample > 1)
          out[i] = downSample[i].process(out[i]);
      }
    }
    for (int i=0, c=0; c<channels; i++, c+=4)
      outputs[POLY_OUTPUT].setVoltageSimd(out[i], c);
    outputs[POLY_OUTPUT].setChannels(channels);
  }

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

};

struct WaveFolderWidget : VenomWidget {

  struct StagesSwitch : GlowingSvgSwitchLockable {
    StagesSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_2.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_3.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_4.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_5.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/cnt_6.svg")));
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

  struct CVPort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      WaveFolder* module = static_cast<WaveFolder*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createBoolPtrMenuItem("Disable oversampling", "", &module->disableOver[portId]));
      if (portId < WaveFolder::BIAS_INPUT){
        menu->addChild(createBoolPtrMenuItem("Bipolar VCA (ring mod)", "", &module->bipolar[portId]));
      }
      PolyPort::appendContextMenu(menu);
    }
  };

  template <class TCVPort>
  TCVPort* createCVInputCentered(math::Vec pos, engine::Module* module, int inputId) {
    TCVPort* o = createInputCentered<TCVPort>(pos, module, inputId);
    o->portId = inputId;
    return o;
  }

  WaveFolderWidget(WaveFolder* module) {
    setModule(module);
    setVenomPanel("WaveFolder");

    addParam(createLockableParamCentered<StagesSwitch>(Vec(24.f, 72.f), module, WaveFolder::STAGES_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(66.f, 72.f), module, WaveFolder::OVER_PARAM));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(19.f, 132.f), module, WaveFolder::PRE_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(71.f, 132.f), module, WaveFolder::STAGE_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(45.f, 166.f), module, WaveFolder::BIAS_PARAM));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(19.f, 198.f), module, WaveFolder::PRE_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(71.f, 198.f), module, WaveFolder::STAGE_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(45.f, 228.f), module, WaveFolder::BIAS_AMT_PARAM));

    addInput(createCVInputCentered<CVPort>(Vec(19.f, 257.5f), module, WaveFolder::PRE_INPUT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(5.5f,246.f), module, WaveFolder::PRE_VCA_LIGHT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(32.5f,246.f), module, WaveFolder::PRE_OVER_LIGHT));

    addInput(createCVInputCentered<CVPort>(Vec(71.f, 257.5f), module, WaveFolder::STAGE_INPUT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(57.5f,246.f), module, WaveFolder::STAGE_VCA_LIGHT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(84.5f,246.f), module, WaveFolder::STAGE_OVER_LIGHT));

    addInput(createCVInputCentered<CVPort>(Vec(45.f, 287.5f), module, WaveFolder::BIAS_INPUT));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(58.5f,276.f), module, WaveFolder::BIAS_OVER_LIGHT));
    
    addInput(createInputCentered<PolyPort>(Vec(24.f, 335.5f), module, WaveFolder::POLY_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(66.f, 335.5f), module, WaveFolder::POLY_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    if(this->module) {
      WaveFolder* mod = static_cast<WaveFolder*>(this->module);
      bool over = mod->params[WaveFolder::OVER_PARAM].getValue();

      mod->lights[WaveFolder::PRE_OVER_LIGHT].setBrightness(over && !(mod->disableOver[WaveFolder::PRE_INPUT]) && mod->inputs[WaveFolder::PRE_INPUT].isConnected());
      mod->lights[WaveFolder::PRE_OVER_LIGHT+1].setBrightness(over && (mod->disableOver[WaveFolder::PRE_INPUT]) && mod->inputs[WaveFolder::PRE_INPUT].isConnected());
      mod->lights[WaveFolder::PRE_VCA_LIGHT].setBrightness(mod->bipolar[WaveFolder::PRE_INPUT]);

      mod->lights[WaveFolder::STAGE_OVER_LIGHT].setBrightness(over && !(mod->disableOver[WaveFolder::STAGE_INPUT]) && mod->inputs[WaveFolder::STAGE_INPUT].isConnected());
      mod->lights[WaveFolder::STAGE_OVER_LIGHT+1].setBrightness(over && (mod->disableOver[WaveFolder::STAGE_INPUT]) && mod->inputs[WaveFolder::STAGE_INPUT].isConnected());
      mod->lights[WaveFolder::STAGE_VCA_LIGHT].setBrightness(mod->bipolar[WaveFolder::STAGE_INPUT]);

      mod->lights[WaveFolder::BIAS_OVER_LIGHT].setBrightness(over && !(mod->disableOver[WaveFolder::BIAS_INPUT]) && mod->inputs[WaveFolder::BIAS_INPUT].isConnected());
      mod->lights[WaveFolder::BIAS_OVER_LIGHT+1].setBrightness(over && (mod->disableOver[WaveFolder::BIAS_INPUT]) && mod->inputs[WaveFolder::BIAS_INPUT].isConnected());
    }
  }
};

Model* modelWaveFolder = createModel<WaveFolder, WaveFolderWidget>("WaveFolder");

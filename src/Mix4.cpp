// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "dsp/math.hpp"
#include "OversampleFilter.hpp"
#include "ThemeStrings.hpp"

#define MODULE_NAME Mix4
static const std::string moduleName = "Mix4";

struct Mix4 : Module {
  enum ParamId {
    ENUMS(LEVEL_PARAMS, 4),
    MIX_LEVEL_PARAM,
    MODE_PARAM,
    CLIP_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(INPUTS, 4),
    INPUTS_LEN
  };
  enum OutputId {
    MIX_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  int mode = -1;
  bool connected[4] = {false, false, false, false};
  float normal = 0.f;
  float scale = 1.f;
  float offset = 0.f;
  float oversample = 4.f;
  OversampleFilter_4 outUpSample[4], outDownSample[4];

  #include "ThemeModVars.hpp"

  void appendMenu(Menu* menu, int parmId) {
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolMenuItem("Lock parameter", "",
      [=]() {
        return paramLocks[parmId].locked;
      },
      [=](bool val){
        setLock(val, parmId);
      }
    ));
  }

  struct ParamLock {
    bool locked;
    bool initLocked;
    float min, max, dflt;
    ParamLock(){
      locked = false;
      initLocked = false;
    }
  };

  void setLock(bool val, int id) {
    if (paramLocks[id].locked != val){
      paramLocks[id].locked = val;
      ParamQuantity* q = paramQuantities[id];
      if (val){
        paramLocks[id].min = q->minValue;
        paramLocks[id].max = q->maxValue;
        paramLocks[id].dflt = q->defaultValue;
        q->name += " (locked)";
        q->minValue = q->maxValue = q->defaultValue = q->getValue();
      }
      else {
        q->name.erase(q->name.length()-9);
        q->minValue = paramLocks[id].min;
        q->maxValue = paramLocks[id].max;
        q->defaultValue = paramLocks[id].dflt;
      }
    }
  }

  void setLockAll(bool val){
    for (int i=0; i<PARAMS_LEN; i++)
      setLock(val, i);
  }

  ParamLock paramLocks[PARAMS_LEN];
  bool paramLocksInitialized = false;

  Mix4() {
    struct FixedSwitchQuantity : SwitchQuantity {
      std::string getDisplayValueString() override {
        return labels[getValue()];
      }
    };
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i < 4; i++){
      configParam(LEVEL_PARAMS+i, 0.f, 2.f, 1.f, string::f("Channel %d level", i + 1), " dB", -10.f, 20.f);
      configInput(INPUTS+i, string::f("Channel %d", i + 1));
    }
    configParam(MIX_LEVEL_PARAM, 0.f, 2.f, 1.f, "Mix level", " dB", -10.f, 20.f);
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 4.f, 0.f, "Level Mode", {"Unipolar audio dB", "Unipolar audio dB poly sum", "Bipolar CV%", "Bipolar CV x2", "Bipolar CV x10"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 3.f, 0.f, "Clipping", {"Off", "Hard CV clipping", "Soft audio clipping", "Soft oversampled audio clipping"});
    configOutput(MIX_OUTPUT, "Mix");
    initOversample();
  }

  void initOversample(){
    for (int i=0; i<4; i++){
      outUpSample[i].setOversample(oversample);
      outDownSample[i].setOversample(oversample);
    }
  }

  void onReset(const ResetEvent& e) override {
    mode = -1;
    initOversample();
    Module::onReset(e);
  }

  void process(const ProcessArgs& args) override {
    if (!paramLocksInitialized){
      paramLocksInitialized = true;
      for (int i=0; i<PARAMS_LEN; i++){
        setLock(paramLocks[i].initLocked, i);
      }
    }
    if( static_cast<int>(params[MODE_PARAM].getValue()) != mode ||
        connected[0] != inputs[INPUTS + 0].isConnected() ||
        connected[1] != inputs[INPUTS + 1].isConnected() ||
        connected[2] != inputs[INPUTS + 2].isConnected() ||
        connected[3] != inputs[INPUTS + 3].isConnected()
    ){
      mode = static_cast<int>(params[MODE_PARAM].getValue());
      ParamQuantity* q;
      for (int i=0; i<4; i++) {
        connected[i] = inputs[INPUTS + i].isConnected();
        q = paramQuantities[LEVEL_PARAMS + i];
        q->unit = mode <= 1 ? " dB" : !connected[i] ? " V" : mode == 2 ? "%" : "x";
        q->displayBase = mode <= 1 ? -10.f : 0.f;
        q->displayMultiplier = mode <= 1 ? 20.f : (mode == 2 && connected[i]) ? 100.f : (mode == 3 && connected[i]) ? 2.f : 10.f;
        q->displayOffset = mode <= 1 ? 0.f : (mode == 2 && connected[i]) ? -100.f : (mode == 3 && connected[i]) ? -2.f : -10.f;
      }
      q = paramQuantities[MIX_LEVEL_PARAM];
      q->unit = mode <= 1 ? " dB" : mode == 2 ? "%" : "x";
      q->displayBase = mode <= 1 ? -10.f : 0.f;
      q->displayMultiplier = mode <= 1 ? 20.f : mode == 2 ? 100.f : mode == 3 ? 2.f : 10.f;
      q->displayOffset = mode <= 1 ? 0.f : mode == 2 ? -100.f : mode == 3 ? -2.f : -10.f;
      q->defaultValue = mode <= 1 ? 1.f : mode == 2 ? 2.f : mode == 3 ? 1.5f : 1.1f;
      normal = mode <= 1 ? 0.f : mode == 2 ? 10.f : mode == 3 ? 5.f : 1.f;
      scale = mode == 4 ? 10.f : mode == 3 ? 2.f : 1.f;
      offset = mode <= 1 ? 0.f : -1.f;
    }
    int clip = static_cast<int>(params[CLIP_PARAM].getValue());

    int channels = mode == 1 ? 1 : std::max({1, inputs[INPUTS].getChannels(), inputs[INPUTS+1].getChannels(), inputs[INPUTS+2].getChannels(), inputs[INPUTS+3].getChannels()});
    simd::float_4 out;
    for (int c=0; c<channels; c+=4){
      out = mode == 1 ?
            inputs[INPUTS+0].getVoltageSum() * (params[LEVEL_PARAMS+0].getValue()+offset)*scale
          + inputs[INPUTS+1].getVoltageSum() * (params[LEVEL_PARAMS+1].getValue()+offset)*scale
          + inputs[INPUTS+2].getVoltageSum() * (params[LEVEL_PARAMS+2].getValue()+offset)*scale
          + inputs[INPUTS+3].getVoltageSum() * (params[LEVEL_PARAMS+3].getValue()+offset)*scale
        :
            inputs[INPUTS+0].getNormalPolyVoltageSimd<simd::float_4>(normal, c) * (params[LEVEL_PARAMS+0].getValue()+offset)*scale
          + inputs[INPUTS+1].getNormalPolyVoltageSimd<simd::float_4>(normal, c) * (params[LEVEL_PARAMS+1].getValue()+offset)*scale
          + inputs[INPUTS+2].getNormalPolyVoltageSimd<simd::float_4>(normal, c) * (params[LEVEL_PARAMS+2].getValue()+offset)*scale
          + inputs[INPUTS+3].getNormalPolyVoltageSimd<simd::float_4>(normal, c) * (params[LEVEL_PARAMS+3].getValue()+offset)*scale;
      out *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale;
      if (clip == 1)
        out = clamp(out, -10.f, 10.f);
      if (clip == 2)
        out = 10.f * tanh_rational5(out/9.5f);
      if (clip == 3){
        for (int i=0; i<oversample; i++){
          out = outUpSample[c/4].process(i ? simd::float_4::zero() : out*oversample);
          out = 10.f * tanh_rational5(out/9.5f);
          out = outDownSample[c/4].process(out);
        }
      }
      outputs[MIX_OUTPUT].setVoltageSimd(out, c);
    }
    outputs[MIX_OUTPUT].setChannels(channels);
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    for (int i=0; i<PARAMS_LEN; i++){
      std::string nm = "paramLock"+std::to_string(i);
      json_object_set_new(rootJ, nm.c_str(), json_boolean(paramLocks[i].locked));
    }
    #include "ThemeToJson.hpp"
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* val;
    for (int i=0; i<PARAMS_LEN; i++){
      std::string nm = "paramLock"+std::to_string(i);
      if ((val = json_object_get(rootJ, nm.c_str())))
        paramLocks[i].initLocked = json_boolean_value(val);
    }
    #include "ThemeFromJson.hpp"
  }
};

struct Mix4Widget : ModuleWidget {

  struct GlowingSvgSwitch : app::SvgSwitch {
    void drawLayer(const DrawArgs& args, int layer) override {
      if (layer==1) {
        if (module && !module->isBypassed()) {
          draw(args);
        }
      }
      app::SvgSwitch::drawLayer(args, layer);
    }

    void appendContextMenu(Menu* menu) override {
      if (module)
        dynamic_cast<Mix4*>(this->module)->appendMenu(menu, this->paramId);
    }
  };

  struct ModeSwitch : GlowingSvgSwitch {
    ModeSwitch() {
      shadow->opacity = 0.0;
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPinkButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  struct ClipSwitch : GlowingSvgSwitch {
    ClipSwitch() {
      shadow->opacity = 0.0;
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
    }
  };

  struct RoundSmallBlackKnobCustom : RoundSmallBlackKnob {
    void appendContextMenu(Menu* menu) override {
      if (module)
        dynamic_cast<Mix4*>(this->module)->appendMenu(menu, this->paramId);
    }
  };

  struct RoundBlackKnobCustom : RoundBlackKnob {
    void appendContextMenu(Menu* menu) override {
      if (module)
        dynamic_cast<Mix4*>(this->module)->appendMenu(menu, this->paramId);
    }
  };

  Mix4Widget(Mix4* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, faceplatePath(moduleName, module ? module->currentThemeStr() : themes[getDefaultTheme()]))));

    addParam(createParamCentered<RoundSmallBlackKnobCustom>(Vec(22.337, 34.295), module, Mix4::LEVEL_PARAMS+0));
    addParam(createParamCentered<RoundSmallBlackKnobCustom>(Vec(22.337, 66.535), module, Mix4::LEVEL_PARAMS+1));
    addParam(createParamCentered<RoundSmallBlackKnobCustom>(Vec(22.337, 98.775), module, Mix4::LEVEL_PARAMS+2));
    addParam(createParamCentered<RoundSmallBlackKnobCustom>(Vec(22.337,131.014), module, Mix4::LEVEL_PARAMS+3));
    addParam(createParamCentered<RoundBlackKnobCustom>(Vec(22.337,168.254), module, Mix4::MIX_LEVEL_PARAM));
    addParam(createParamCentered<ModeSwitch>(Vec(36.674,50.415), module, Mix4::MODE_PARAM));
    addParam(createParamCentered<ClipSwitch>(Vec(36.674,82.655), module, Mix4::CLIP_PARAM));

    addInput(createInputCentered<PJ301MPort>(Vec(22.337,201.993), module, Mix4::INPUTS+0));
    addInput(createInputCentered<PJ301MPort>(Vec(22.337,235.233), module, Mix4::INPUTS+1));
    addInput(createInputCentered<PJ301MPort>(Vec(22.337,268.473), module, Mix4::INPUTS+2));
    addInput(createInputCentered<PJ301MPort>(Vec(22.337,301.712), module, Mix4::INPUTS+3));
    addOutput(createOutputCentered<PJ301MPort>(Vec(22.337,340.434), module, Mix4::MIX_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    Mix4* module = dynamic_cast<Mix4*>(this->module);
    assert(module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Lock all parameters", "",
      [=]() {
        module->setLockAll(true);
      }
    ));
    menu->addChild(createMenuItem("Unlock all parameters", "",
      [=]() {
        module->setLockAll(false);
      }
    ));
    #include "ThemeMenu.hpp"
  }

  #include "ThemeStep.hpp"
};

Model* modelMix4 = createModel<Mix4, Mix4Widget>("Mix4");

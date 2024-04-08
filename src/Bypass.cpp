// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "TaskWorker.hpp"

struct Bypass : VenomModule {
  enum ParamId {
    TRIG_PARAM,
    GATE_PARAM,
    ENUMS(INPUT_MODE_PARAM,3),
    ENUMS(OUTPUT_MODE_PARAM,3),
    PARAMS_LEN
  };
  enum InputId {
    TRIG_INPUT,
    ENUMS(BYPASS_INPUT,3),
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(BYPASS_OUTPUT,3),
    OUTPUTS_LEN
  };
  enum LightId {
    TRIG_LIGHT,
    LIGHTS_LEN
  };

  float buttonVal = 0.f;
  bool bypassed = false;
  bool working = false;
  dsp::TSchmittTrigger<float> trig;
  TaskWorker taskWorker;

  Bypass() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(TRIG_PARAM, 0.f, 1.f, 0.f, "Trigger");
    configSwitch<FixedSwitchQuantity>(GATE_PARAM, 0.f, 2.f, 0.f, "Gate Mode", {"Off","On","Invert"});
    configInput(TRIG_INPUT, "Trigger");
    for (int i=0; i<3; i++){
      std::string id = std::to_string(i+1);
      configSwitch<FixedSwitchQuantity>(INPUT_MODE_PARAM+i, 0.f, 4.f, 0.f, "Input "+id+" bypass mode", {
        "Off",
        "Source",
        "Source and left neighbors",
        "Source and right neighbors",
        "Source and all neighbors",
      });
      configInput(BYPASS_INPUT+i, "Bypass "+id);
      configOutput(BYPASS_OUTPUT+i, "Bypass "+id);
      configSwitch<FixedSwitchQuantity>(OUTPUT_MODE_PARAM+i, 0.f, 4.f, 1.f, "Output "+id+" bypass mode", {
        "Off",
        "Target",
        "Target and left neighbors",
        "Target and right neighbors",
        "Target and all neighbors",
      });
      configBypass(BYPASS_INPUT+i, BYPASS_OUTPUT+i);
    }
  }
  
  struct BypassGroup {
    std::vector<Module*> mods;
    int scope = 0;
  };

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (working) return;
    int event = 0;
    int buttonEvent = 0;
    BypassGroup bypassGroup[6]{};
    int groups = 0;
    if (params[TRIG_PARAM].getValue() != buttonVal){
      buttonVal = !buttonVal;
      if (buttonVal){
        bypassed = !bypassed;
        buttonEvent = 1;
      }
    }
    switch (static_cast<int>(params[GATE_PARAM].getValue())) {
      case 0: // Trigger mode
        if ((event = trig.process(inputs[TRIG_INPUT].getVoltage(), 0.1f, 2.f)))
          bypassed = !bypassed;
        break;
      case 1: // Gate mode
        if ((event = trig.processEvent(inputs[TRIG_INPUT].getVoltage(), 0.1f, 2.f)))
          bypassed = event > 0;
        break;
      case 2: // Inverted gate mode
        if ((event = trig.processEvent(inputs[TRIG_INPUT].getVoltage(), 0.1f, 2.f)))
          bypassed = event < 0;
        break;
    }
    std::vector<Module*> inMods[INPUTS_LEN]{};
    std::vector<Module*> outMods[OUTPUTS_LEN]{};
    if (event || buttonEvent){
      for (int64_t cableId : APP->engine->getCableIds()){
        Cable* cable = APP->engine->getCable(cableId);
        if (cable->inputModule == this) inMods[cable->inputId].push_back(cable->outputModule);
        if (cable->outputModule == this) outMods[cable->outputId].push_back(cable->inputModule);
      }
    }
    for (int i=0; i<3; i++){
      outputs[BYPASS_OUTPUT+i].setChannels(inputs[BYPASS_INPUT+i].getChannels());
      if (inputs[BYPASS_INPUT+i].isConnected()) outputs[BYPASS_OUTPUT+i].writeVoltages(inputs[BYPASS_INPUT+i].getVoltages());
      else outputs[BYPASS_OUTPUT+i].channels = 0;
      if (event || buttonEvent) {
        if (inMods[BYPASS_INPUT+i].size() && (bypassGroup[groups].scope = params[INPUT_MODE_PARAM+i].getValue())){
          bypassGroup[groups++].mods = inMods[BYPASS_INPUT+i];
        }
        if (outMods[BYPASS_OUTPUT+i].size() && (bypassGroup[groups].scope = params[OUTPUT_MODE_PARAM+i].getValue())){
          bypassGroup[groups++].mods = outMods[BYPASS_OUTPUT+i];
        }
      }
    }
    if (groups) {
      working = true;
      taskWorker.work([=](){ processBypassGroups(bypassed, bypassGroup, groups); });
    }
  }
  
  void processBypassGroups(bool bypassed, const BypassGroup* bypassGroup, int groups){
    for (int i=0; i<groups; i++){
      bool bypassLeft = bypassGroup[i].scope == 2 || bypassGroup[i].scope == 4;
      bool bypassRight = bypassGroup[i].scope == 3 || bypassGroup[i].scope == 4;
      for (Module* mod : bypassGroup[i].mods){
        if (bypassLeft) {
          for (Module* neighbor = mod->getLeftExpander().module; neighbor && neighbor->model != this->model; neighbor = neighbor->getLeftExpander().module){
            APP->engine->bypassModule(neighbor, bypassed);
          }
        }
        if (bypassRight) {
          for (Module* neighbor = mod->getRightExpander().module; neighbor && neighbor->model != this->model; neighbor = neighbor->getRightExpander().module){
            APP->engine->bypassModule(neighbor, bypassed);
          }
        }
        APP->engine->bypassModule(mod, bypassed);
      }
    }
    working = false;
  }  
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "bypassed", json_boolean(bypassed));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "bypassed"))){
      bypassed = json_boolean_value(val);
    }
  }

};

struct BypassWidget : VenomWidget {
  
  struct BypassSwitch : GlowingSvgSwitchLockable {
    BypassSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };

  struct GateSwitch : GlowingSvgSwitchLockable {
    GateSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
    }
  };

  BypassWidget(Bypass* module) {
    setModule(module);
    setVenomPanel("Bypass");
    addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<RedLight>>>(Vec(22.5f, 39.5), module, Bypass::TRIG_PARAM, Bypass::TRIG_LIGHT));
    addParam(createLockableParamCentered<GateSwitch>(Vec(37.f,53.5f), module, Bypass::GATE_PARAM));
    addInput(createInputCentered<MonoPort>(Vec(22.5f,67.5f), module, Bypass::TRIG_INPUT));
    float delta=0.f;
    for (int i=0; i<3; i++, delta+=86.f) {
      addParam(createLockableParamCentered<BypassSwitch>(Vec(22.5f,110.f+delta), module, Bypass::INPUT_MODE_PARAM+i));
      addInput(createInputCentered<PolyPort>(Vec(22.5f,128.5f+delta), module, Bypass::BYPASS_INPUT+i));
      addOutput(createOutputCentered<PolyPort>(Vec(22.5f,155.5f+delta), module, Bypass::BYPASS_OUTPUT+i));
      addParam(createLockableParamCentered<BypassSwitch>(Vec(22.5f,174.f+delta), module, Bypass::OUTPUT_MODE_PARAM+i));
    }
  }

  void step() override {
    VenomWidget::step();
    Bypass* mod = dynamic_cast<Bypass*>(this->module);
    if (mod) mod->lights[Bypass::TRIG_LIGHT].setBrightness(mod->bypassed);
  }

};

Model* modelBypass = createModel<Bypass, BypassWidget>("Bypass");

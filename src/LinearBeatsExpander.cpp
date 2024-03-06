// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

#define LIGHT_OFF 0.02f
#define LIGHT_ON  1.f

struct LinearBeatsExpander : VenomModule {
  #include "LinearBeatsExpander.hpp"

  bool left = false;

  LinearBeatsExpander() {
    venomConfig(EXP_PARAMS_LEN, EXP_INPUTS_LEN, EXP_OUTPUTS_LEN, EXP_LIGHTS_LEN);
    for (int i=0; i<9; i++) {
      configInput(MUTE_INPUT+i, label[i]+" mute CV");
      configSwitch<FixedSwitchQuantity>(MUTE_PARAM+i, 0.f, 1.f, 0.f, label[i]+" mute", {"Unmuted", "Muted"});
    }
    configInput(BYPASS_INPUT, "Disable linear beats CV");
    configSwitch<FixedSwitchQuantity>(BYPASS_PARAM, 0.f, 1.f, 0.f, "Linear beats", {"Enabled", "Disabled"});
    configLight(LEFT_LIGHT, "Left connection indicator");
    configLight(RIGHT_LIGHT, "Right connection indicator");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
  
  void setLabels(std::string str){
    for (int i=0; i<9; i++) {
      paramQuantities[MUTE_PARAM+i]->name = label[i]+str;
      inputInfos[MUTE_INPUT+i]->name = label[i]+str+" CV";
    }
  }
  
  void setConnectionLight(){
    Module* mod = getRightExpander().module;
    if (mod && mod->model == modelLinearBeats) {
      lights[RIGHT_LIGHT].setBrightness(1.f);
      lights[LEFT_LIGHT].setBrightness(0.f);
      setLabels(" input mute");
      left = false;
    }  
    else {
      lights[RIGHT_LIGHT].setBrightness(0.f);
      mod = getLeftExpander().module;
      if (mod && mod->model == modelLinearBeats) {
        lights[LEFT_LIGHT].setBrightness(1.f);
        setLabels(" output mute");
        left = true;
      }
      else {
        lights[LEFT_LIGHT].setBrightness(0.f);
        setLabels(" mute");
        left = false;
      }  
    }  
  }
  
  void onExpanderChange(const ExpanderChangeEvent& e) override {
    setConnectionLight();
  }  

};

struct LinearBeatsExpanderWidget : VenomWidget {
  int venomDelCnt = 0;

  LinearBeatsExpanderWidget(LinearBeatsExpander* module) {
    setModule(module);
    setVenomPanel("LinearBeatsExpander");
    float y=56.5f;
    for(int i=0; i<9; i++){
      addInput(createInputCentered<MonoPort>(Vec(20.5f,y), module, LinearBeatsExpander::MUTE_INPUT+i));
      addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(55.5f,y), module, LinearBeatsExpander::MUTE_PARAM+i, LinearBeatsExpander::MUTE_LIGHT+i));
      y+=31.556f;
    }  
    addInput(createInputCentered<MonoPort>(Vec(20.5f,344.85f), module, LinearBeatsExpander::BYPASS_INPUT));
    addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(55.5f,344.85f), module, LinearBeatsExpander::BYPASS_PARAM, LinearBeatsExpander::BYPASS_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(6.f, 21.3f), module, LinearBeatsExpander::LEFT_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(69.f, 21.3f), module, LinearBeatsExpander::RIGHT_LIGHT));
  }

  void step() override {
    VenomWidget::step();
    if(this->module) {
      for (int i=0; i<9; i++){
        this->module->lights[LinearBeatsExpander::MUTE_LIGHT+i].setBrightness(this->module->params[LinearBeatsExpander::MUTE_PARAM+i].getValue() ? LIGHT_ON : LIGHT_OFF);
      }
      this->module->lights[LinearBeatsExpander::BYPASS_LIGHT].setBrightness(this->module->params[LinearBeatsExpander::BYPASS_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      // fix for VCV bug in onExpanderChange (not triggered by module deletion)
      if (venomDelCnt != getVenomDelCnt()){
        static_cast<LinearBeatsExpander*>(this->module)->setConnectionLight();
        venomDelCnt = getVenomDelCnt();
      }
    }  
  }

};

Model* modelLinearBeatsExpander = createModel<LinearBeatsExpander, LinearBeatsExpanderWidget>("LinearBeatsExpander");

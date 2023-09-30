// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

#define LIGHT_OFF 0.02f
#define LIGHT_ON  1.f

struct LinearMute : VenomModule {
  #include "LinearMute.hpp"

  bool left = false;

  LinearMute() {
    venomConfig(MUTE_PARAMS_LEN, MUTE_INPUTS_LEN, MUTE_OUTPUTS_LEN, MUTE_LIGHTS_LEN);
    for (int i=0; i<9; i++) {
      configSwitch(MUTE_PARAM+i, 0.f, 1.f, 0.f, label[i]+" Mute", {"Unmuted", "Muted"});
    }  
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
  
  void setLabels(std::string str){
    for (int i=0; i<9; i++)
      paramQuantities[MUTE_PARAM+i]->name = label[i]+str;
  }  
  
  void onExpanderChange(const ExpanderChangeEvent& e) override {
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

};

struct LinearMuteWidget : VenomWidget {

  LinearMuteWidget(LinearMute* module) {
    setModule(module);
    setVenomPanel("LinearMute");
    float y=56.5f;
    for(int i=0; i<9; i++){
      addParam(createLockableLightParamCentered<VCVLightBezelLatchLockable<MediumSimpleLight<RedLight>>>(Vec(22.5f,y), module, LinearMute::MUTE_PARAM+i, LinearMute::MUTE_LIGHT+i));
      y+=31.556f;
    }  
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(7.5f, 35.f), module, LinearMute::LEFT_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(37.5f, 35.f), module, LinearMute::RIGHT_LIGHT));
  }

  void step() override {
    VenomWidget::step();
    if(this->module) {
      for (int i=0; i<9; i++){
        this->module->lights[LinearMute::MUTE_LIGHT+i].setBrightness(this->module->params[LinearMute::MUTE_PARAM+i].getValue() ? LIGHT_ON : LIGHT_OFF);
      }
    }  
  }

};

Model* modelLinearMute = createModel<LinearMute, LinearMuteWidget>("LinearMute");

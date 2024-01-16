// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

struct Push5 : VenomModule {
  enum ParamId {
    ENUMS(BUTTON_PARAM,5),
    PARAMS_LEN
  };
  enum InputId {
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(OUTPUT,5),
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(BUTTON_LIGHT,5*3),
    LIGHTS_LEN
  };
  
  struct ButtonExtension {
    int mode;
    int onColor;
    int offColor;
    int onVal;
    int offVal;
    bool lightOn;
    bool buttonOn;
    dsp::PulseGenerator trigGenerator;
    ButtonExtension(){
      mode=1; // gate
      onColor=6;
      offColor=13;
      onVal=0;
      offVal=3;
      lightOn=false;
      buttonOn=false;
    }
  };

  ButtonExtension buttonExtension[5]{};
  
  float buttonVals[7]{10.f,5.f,1.f,0.f,-1.f,-5.f,-10.f};
  float buttonColors[15][3]{
    {1.f,0.f,0.f}, //red
    {1.f,1.f,0.f}, //yellow
    {0.f,0.f,1.f}, //blue
    {0.f,1.f,0.f}, //green
    {0.5f,0.f,1.f}, //purple
    {1.f,0.5f,0.f}, //orange
    {1.f,1.f,1.f}, //white
    {0.02f,0.f,0.f}, //darkRed
    {0.02f,0.02f,0.f}, //darkYellow
    {0.f,0.f,0.02f}, //darkBlue
    {0.f,0.02f,0.f}, //darkGreen
    {0.01f,0.f,0.02f}, //darkPurple
    {0.02f,0.01f,0.f}, //darkOrange
    {0.02f,0.02f,0.02f}, //gray
    {0.f,0.f,0.f} //off
  };

  void appendCustomParamMenu(Menu *menu, int paramId){
    ButtonExtension *e = buttonExtension + paramId;
    menu->addChild(createIndexSubmenuItem(
      "Button mode",
      {"Trigger","Gate","Toggle"},
      [=]() {
        return e->mode;
      },
      [=](int mode) {
        if (mode) e->trigGenerator.reset();
        e->mode = mode;
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "On value",
      {"10 V","5 V","1 V","0 V","-1 V","-5 V","-10 V"},
      [=]() {
        return e->onVal;
      },
      [=](int val) {
        e->onVal = val;
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Off value",
      {"10 V","5 V","1 V","0 V","-1 V","-5 V","-10 V"},
      [=]() {
        return e->offVal;
      },
      [=](int val) {
        e->offVal = val;
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "On Color",
      {"Red","Yellow","Blue","Green","Purple","Orange","White",
       "Dim Red","Dim Yellow","Dim Blue","Dim Green","Dim Purple","Dim Orange","Dim Gray","Off"},
      [=]() {
        return e->onColor;
      },
      [=](int val) {
        e->onColor = val;
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Off Color",
      {"Red","Yellow","Blue","Green","Purple","Orange","White",
       "Dim Red","Dim Yellow","Dim Blue","Dim Green","Dim Purple","Dim Orange","Dim Gray","Off"},
      [=]() {
        return e->offColor;
      },
      [=](int val) {
        e->offColor = val;
      }
    ));
  }  
  
  Push5() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<5; i++) {
      std::string nm = "Button " + std::to_string(i+1);
      configParam(BUTTON_PARAM+i, 0.f, 1.f, 0.f, nm);
      configOutput(OUTPUT+i, nm);
    }  
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    for (int i=0; i<5; i++){
      ButtonExtension *e = buttonExtension+i;
      float val = params[i].getValue();
      switch (e->mode){
        case 0: //trigger
          if (val>0 && !e->lightOn && e->trigGenerator.remaining<=0.f){
            e->trigGenerator.trigger();
          }
          e->lightOn = val;
          outputs[i].setVoltage(e->trigGenerator.process(args.sampleTime) ? buttonVals[e->onVal] : buttonVals[e->offVal]);
          break;
        case 1: //gate
          e->lightOn = val;
          outputs[i].setVoltage(val ? buttonVals[e->onVal] : buttonVals[e->offVal]);
          break;
        case 2: //toggle
          if (val && !e->buttonOn){
            e->lightOn = !e->lightOn;
            outputs[i].setVoltage(e->lightOn ? buttonVals[e->onVal] : buttonVals[e->offVal]);
          }
          break;
      }
      e->buttonOn = val;
    }
  }
  
};

struct Push5Widget : VenomWidget {

  template <typename TLightBase>
  struct Button : VCVLightBezelLockable<TLightBase> {
    void appendContextMenu(Menu* menu) override {
      if (this->module) {
        VCVLightBezelLockable<TLightBase>::appendContextMenu(menu);
        static_cast<Push5*>(this->module)->appendCustomParamMenu(menu, this->paramId);
      }
    }
  };

  Push5Widget(Push5* module) {
    setModule(module);
    setVenomPanel("Push5");
    float y=42.5f;
    for (int i=0; i<5; i++, y+=31.f){
      addParam(createLockableLightParamCentered<Button<MediumSimpleLight<RedGreenBlueLight>>>(Vec(22.5f, y), module, Push5::BUTTON_PARAM+i, Push5::BUTTON_LIGHT+i*3));
    }
    y = 209.5f;
    for (int i=0; i<5; i++, y+=32.f){
      addOutput(createOutputCentered<MonoPort>(Vec(22.5f, y), module, Push5::OUTPUT+i));
    }
  }

  void step() override {
    VenomWidget::step();
    Push5* mod = dynamic_cast<Push5*>(this->module);
    if(mod) {
      for (int i=0, l=0; i<5; i++, l+=3){
        Push5::ButtonExtension *e = mod->buttonExtension+i;
        mod->lights[Push5::BUTTON_LIGHT+l+0].setBrightness(e->lightOn ? mod->buttonColors[e->onColor][0] : mod->buttonColors[e->offColor][0]);
        mod->lights[Push5::BUTTON_LIGHT+l+1].setBrightness(e->lightOn ? mod->buttonColors[e->onColor][1] : mod->buttonColors[e->offColor][1]);
        mod->lights[Push5::BUTTON_LIGHT+l+2].setBrightness(e->lightOn ? mod->buttonColors[e->onColor][2] : mod->buttonColors[e->offColor][2]);
      }
    }
  }

};

Model* modelPush5 = createModel<Push5, Push5Widget>("Push5");

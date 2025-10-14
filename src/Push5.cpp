// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

struct Push5 : VenomModule {
  enum ParamId {
    ENUMS(BUTTON_PARAM,5),
    ENUMS(ON_PARAM,5),
    ENUMS(OFF_PARAM,5),
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
    int poly;
    float lightOn;
    bool buttonOn;
    dsp::PulseGenerator trigGenerator;
    ButtonExtension(){
      mode=1; // gate
      onColor=6;
      offColor=13;
      onVal=0;
      offVal=3;
      poly=1;
      lightOn=0.f;
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
    ParamQuantity* onq = paramQuantities[ON_PARAM+paramId];
    ParamQuantity* offq = paramQuantities[OFF_PARAM+paramId];
    menu->addChild(new MenuSeparator);
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
      {"10 V","5 V","1 V","0 V","-1 V","-5 V","-10 V","Custom"},
      [=]() {
        return e->onVal;
      },
      [=](int val) {
        e->onVal = val;
        if (val<7)
          onq->setValue(buttonVals[val]);
      }
    ));
    if (e->onVal == 7){
      menu->addChild(createSubmenuItem("Custom ON value", onq->getDisplayValueString(),
        [=](Menu *menu){
          MenuTextField *editField = new MenuTextField();
          editField->box.size.x = 150;
          editField->setText(onq->getDisplayValueString());
          editField->changeHandler = [=](std::string text) {
            onq->setDisplayValueString(text);
          };
          menu->addChild(editField);
        }
      ));
    }
    menu->addChild(createIndexSubmenuItem(
      "Off value",
      {"10 V","5 V","1 V","0 V","-1 V","-5 V","-10 V","Custom"},
      [=]() {
        return e->offVal;
      },
      [=](int val) {
        e->offVal = val;
        if (val<7)
          offq->setValue(buttonVals[val]);
      }
    ));
    if (e->offVal == 7){
      menu->addChild(createSubmenuItem("Custom OFF value", offq->getDisplayValueString(),
        [=](Menu *menu){
          MenuTextField *editField = new MenuTextField();
          editField->box.size.x = 150;
          editField->setText(offq->getDisplayValueString());
          editField->changeHandler = [=](std::string text) {
            offq->setDisplayValueString(text);
          };
          menu->addChild(editField);
        }
      ));
    }
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
    menu->addChild(createIndexSubmenuItem(
      "Polyphony channels",
      {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
      [=]() {
        return e->poly-1;
      },
      [=](int val) {
        e->poly = val+1;
      }
    ));
  }  
  
  Push5() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    ParamQuantity *q;
    for (int i=0; i<5; i++) {
      std::string istr = std::to_string(i+1);
      std::string nm = "Button " + istr;
      configParam(BUTTON_PARAM+i, 0.f, 1.f, 0.f, nm);
      configOutput(OUTPUT+i, nm);
      nm = "On " + istr;
      q=configParam(ON_PARAM+i, -100.f, 100.f, 10.f, nm);
      q->resetEnabled = false;
      q->randomizeEnabled = false;
      nm = "Off " + istr;
      q=configParam(OFF_PARAM+i, -100.f, 100.f, 0.f, nm);
      q->resetEnabled = false;
      q->randomizeEnabled = false;
      paramExtensions[i].inputLink = false;
      paramExtensions[i].nameLink = i;
      outputExtensions[i].nameLink = i;
    }  
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    for (int i=0; i<5; i++){
      ButtonExtension *e = buttonExtension+i;
      float val = params[i].getValue();
      float out = 0.f;
      bool stus;
      switch (e->mode){
        case 0: //trigger
          if (val>0 && !e->buttonOn && !e->lightOn && e->trigGenerator.remaining<=0.f){
            e->trigGenerator.trigger();
          }
          stus = e->trigGenerator.process(args.sampleTime);
          out = params[(stus ? ON_PARAM : OFF_PARAM)+i].getValue();
          e->lightOn = math::clamp(stus ? 1.f : e->lightOn - args.sampleTime/0.1f);
          break;
        case 1: //gate
          e->lightOn = val;
          out = params[(val ? ON_PARAM : OFF_PARAM)+i].getValue();
          break;
        case 2: //toggle
          if (val && !e->buttonOn){
            e->lightOn = !e->lightOn;
          }
          out = params[(e->lightOn ? ON_PARAM : OFF_PARAM)+i].getValue();
          break;
      }
      e->buttonOn = val;
      for (int c=0; c<e->poly; c++)
        outputs[i].setVoltage(out,c);
      outputs[i].setChannels(e->poly);
    }
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    for (int i=0; i<5; i++){
      ButtonExtension *e = buttonExtension+i;
      std::string iStr = std::to_string(i);
      std::string nm = "mode"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(e->mode));
      nm = "onVal"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(e->onVal));
      nm = "offVal"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(e->offVal));
      nm = "onColor"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(e->onColor));
      nm = "offColor"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(e->offColor));
      nm = "lightOn"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_boolean(e->mode==2 ? e->lightOn : false));
      nm = "poly"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(e->poly));
    }
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    for (int i=0; i<5; i++){
      ButtonExtension *e = buttonExtension+i;
      std::string iStr = std::to_string(i);
      std::string nm = "mode"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        e->mode = json_integer_value(val);
        if (e->mode) e->trigGenerator.reset();
      }
      nm = "onVal"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        e->onVal = json_integer_value(val);
      }
      nm = "offVal"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        e->offVal = json_integer_value(val);
      }
      nm = "onColor"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        e->onColor = json_integer_value(val);
      }
      nm = "offColor"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        e->offColor = json_integer_value(val);
      }
      nm = "lightOn"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        e->lightOn = json_boolean_value(val);
      }
      nm = "poly"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        e->poly = json_integer_value(val);
      }
      if (e->onVal<7)
        params[ON_PARAM+i].setValue(buttonVals[e->onVal]);
      if (e->offVal<7)
        params[OFF_PARAM+i].setValue(buttonVals[e->offVal]);
    }
  }

};

struct Push5Widget : VenomWidget {

  template <typename TLightBase>
  struct Button : VCVLightBezelLockable<TLightBase> {
    void appendContextMenu(Menu* menu) override {
      if (this->module) {
        static_cast<Push5*>(this->module)->appendCustomParamMenu(menu, this->paramId);
        VCVLightBezelLockable<TLightBase>::appendContextMenu(menu);
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
      addOutput(createOutputCentered<PolyPort>(Vec(22.5f, y), module, Push5::OUTPUT+i));
    }
  }

  void appendContextMenu(Menu *menu) override {
    Push5 *module = static_cast<Push5*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Configure all buttons:"));
    menu->addChild(createIndexSubmenuItem(
      "Button mode",
      {"Trigger","Gate","Toggle"},
      [=]() {
        int current = module->buttonExtension[0].mode;
        for (int i=1; i<5; i++){
          if (module->buttonExtension[i].mode != current)
            current = 3;
        }
        return current;
      },
      [=](int mode) {
        if (mode<3){
          for (int i=0; i<5; i++){
            if (mode) module->buttonExtension[i].trigGenerator.reset();
            module->buttonExtension[i].mode = mode;
          }
        }
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "On value",
      {"10 V","5 V","1 V","0 V","-1 V","-5 V","-10 V"},
      [=]() {
        int current = module->buttonExtension[0].onVal;
        for (int i=1; i<5; i++){
          if (module->buttonExtension[i].onVal != current)
            current = 7;
        }
        return current;
      },
      [=](int val) {
        if (val<7){
          for (int i=0; i<5; i++){
            module->buttonExtension[i].onVal = val;
            module->params[Push5::ON_PARAM+i].setValue(module->buttonVals[val]);
          }
        }
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Off value",
      {"10 V","5 V","1 V","0 V","-1 V","-5 V","-10 V"},
      [=]() {
        int current = module->buttonExtension[0].offVal;
        for (int i=1; i<5; i++){
          if (module->buttonExtension[i].offVal != current)
            current = 7;
        }
        return current;
      },
      [=](int val) {
        if (val<7){
          for (int i=0; i<5; i++){
            module->buttonExtension[i].offVal = val;
            module->params[Push5::OFF_PARAM+i].setValue(module->buttonVals[val]);
          }
        }
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "On Color",
      {"Red","Yellow","Blue","Green","Purple","Orange","White",
       "Dim Red","Dim Yellow","Dim Blue","Dim Green","Dim Purple","Dim Orange","Dim Gray","Off"},
      [=]() {
        int current = module->buttonExtension[0].onColor;
        for (int i=1; i<5; i++){
          if (module->buttonExtension[i].onColor != current)
            current = 15;
        }
        return current;
      },
      [=](int val) {
        if (val<15){
          for (int i=0; i<5; i++){
            module->buttonExtension[i].onColor = val;
          }
        }
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Off Color",
      {"Red","Yellow","Blue","Green","Purple","Orange","White",
       "Dim Red","Dim Yellow","Dim Blue","Dim Green","Dim Purple","Dim Orange","Dim Gray","Off"},
      [=]() {
        int current = module->buttonExtension[0].offColor;
        for (int i=1; i<5; i++){
          if (module->buttonExtension[i].offColor != current)
            current = 15;
        }
        return current;
      },
      [=](int val) {
        if (val<15){
          for (int i=0; i<5; i++){
            module->buttonExtension[i].offColor = val;
          }
        }
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Polyphony channels",
      {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
      [=]() {
        int current = module->buttonExtension[0].poly;
        for (int i=1; i<5; i++){
          if (module->buttonExtension[i].poly != current)
            current = 17;
        }
        return current-1;
      },
      [=](int val) {
        if (val<16){
          for (int i=0; i<5; i++){
            module->buttonExtension[i].poly = val+1;
          }
        }
      }
    ));
    VenomWidget::appendContextMenu(menu);
  }  

  void step() override {
    VenomWidget::step();
    Push5* mod = dynamic_cast<Push5*>(this->module);
    if(mod) {
      for (int i=0, l=0; i<5; i++, l+=3){
        Push5::ButtonExtension *e = mod->buttonExtension+i;
        mod->lights[Push5::BUTTON_LIGHT+l+0].setBrightness(e->lightOn*mod->buttonColors[e->onColor][0] + (1-e->lightOn)*mod->buttonColors[e->offColor][0]);
        mod->lights[Push5::BUTTON_LIGHT+l+1].setBrightness(e->lightOn*mod->buttonColors[e->onColor][1] + (1-e->lightOn)*mod->buttonColors[e->offColor][1]);
        mod->lights[Push5::BUTTON_LIGHT+l+2].setBrightness(e->lightOn*mod->buttonColors[e->onColor][2] + (1-e->lightOn)*mod->buttonColors[e->offColor][2]);
      }
    }
  }

};

Model* modelPush5 = createModel<Push5, Push5Widget>("Push5");

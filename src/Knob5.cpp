// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

struct Knob5 : VenomModule {
  enum ParamId {
    ENUMS(KNOB_PARAM,5),
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
    LIGHTS_LEN
  };
  
  int knobRange[5]{7,7,7,7,7};
  int poly[5]{1,1,1,1,1};
  
  void setRange(int paramId, int range){
    ParamQuantity *q = paramQuantities[paramId];
    knobRange[paramId] = range;
    switch (range){
      case 0:
        q->defaultValue = 0.f;
        q->displayMultiplier = 1.f;
        q->displayOffset = 0.f;
        break;;
      case 1:
        q->defaultValue = 0.f;
        q->displayMultiplier = 2.f;
        q->displayOffset = 0.f;
        break;
      case 2:
        q->defaultValue = 0.f;
        q->displayMultiplier = 5.f;
        q->displayOffset = 0.f;
        break;
      case 3:
        q->defaultValue = 0.f;
        q->displayMultiplier = 10.f;
        q->displayOffset = 0.f;
        break;
      case 4:
        q->defaultValue = 0.5f;
        q->displayMultiplier = 2.f;
        q->displayOffset = -1.f;
        break;
      case 5:
        q->defaultValue = 0.5f;
        q->displayMultiplier = 4.f;
        q->displayOffset = -2.f;
        break;
      case 6:
        q->defaultValue = 0.5f;
        q->displayMultiplier = 10.f;
        q->displayOffset = -5.f;
        break;
      case 7:
        q->defaultValue = 0.5f;
        q->displayMultiplier = 20.f;
        q->displayOffset = -10.f;
        break;
    }
  }

  void appendCustomParamMenu(Menu *menu, int paramId){
    menu->addChild(createIndexSubmenuItem(
      "Knob range",
      {"0-1 V","0-2 V","0-5 V","0-10 V","+/- 1 V","+/- 2 V","+/- 5 V","+/- 10 V"},
      [=]() {
        return knobRange[paramId];
      },
      [=](int range) {
        setRange(paramId, range);
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Polyphony channels",
      {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
      [=]() {
        return poly[paramId]-1;
      },
      [=](int val) {
        poly[paramId] = val+1;
      }
    ));
  }  
  
  Knob5() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<5; i++) {
      std::string nm = "Knob " + std::to_string(i+1);
      configParam(KNOB_PARAM+i, 0.f, 1.f, 0.5f, nm, " V", 0.f, 20.f, -10.f);
      configOutput(OUTPUT+i, nm);
      paramExtensions[i].inputLink = false;
      paramExtensions[i].nameLink = i;
      outputExtensions[i].nameLink = i;
    }  
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    for (int i=0; i<5; i++){
      ParamQuantity *q = paramQuantities[i];
      float out = params[i].getValue() * q->displayMultiplier + q->displayOffset;
      for (int c=0; c<poly[i]; c++)
        outputs[i].setVoltage(out,c);
      outputs[i].setChannels(poly[i]);
    }
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    for (int i=0; i<5; i++){
      std::string iStr = std::to_string(i);
      std::string nm = "knobRange"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(knobRange[i]));
      nm = "poly"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(poly[i]));
    }
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    for (int i=0; i<5; i++){
      std::string iStr = std::to_string(i);
      std::string nm = "knobRange"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        setRange(i, json_integer_value(val));
      }
      nm = "poly"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        poly[i] = json_integer_value(val);
      }
    }
  }

};

struct Knob5Widget : VenomWidget {
  
  struct Knob : RoundSmallBlackKnobLockable {
    void appendContextMenu(Menu* menu) override {
      if (this->module) {
        RoundSmallBlackKnobLockable::appendContextMenu(menu);
        static_cast<Knob5*>(this->module)->appendCustomParamMenu(menu, this->paramId);
      }
    }
  };

  Knob5Widget(Knob5* module) {
    setModule(module);
    setVenomPanel("Knob5");
    float y=42.5f;
    for (int i=0; i<5; i++, y+=31.f){
      addParam(createLockableParamCentered<Knob>(Vec(22.5f, y), module, Knob5::KNOB_PARAM+i));
    }
    y = 209.5f;
    for (int i=0; i<5; i++, y+=32.f){
      addOutput(createOutputCentered<PolyPort>(Vec(22.5f, y), module, Knob5::OUTPUT+i));
    }
  }

  void appendContextMenu(Menu* menu) override {
    Knob5* module = dynamic_cast<Knob5*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexSubmenuItem(
      "Configure all knob ranges",
      {"0-1 V","0-2 V","0-5 V","0-10 V","+/- 1 V","+/- 2 V","+/- 5 V","+/- 10 V"},
      [=]() {
        int current = module->knobRange[0];
        for (int i=1; i<5; i++){
          if (current != module->knobRange[i])
            current = 8;
        }
        return current;
      },
      [=](int range) {
        if (range<8) {
          for (int i=0; i<5; i++){
            module->setRange(i, range);
          }
        }
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Polyphony channels",
      {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
      [=]() {
        int current = module->poly[0];
        for (int i=1; i<5; i++){
          if (module->poly[i] != current)
            current = 17;
        }
        return current-1;
      },
      [=](int val) {
        if (val<16){
          for (int i=0; i<5; i++){
            module->poly[i] = val+1;
          }
        }
      }
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelKnob5 = createModel<Knob5, Knob5Widget>("Knob5");

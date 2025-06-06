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
  int quant[5]{0,0,0,0,0};
  int unit[5]{0,0,0,0,0};
  int poly[5]{1,1,1,1,1};
  dsp::ClockDivider clockDivider;
  
  void configQuantity(int paramId){
    ParamQuantity *q = paramQuantities[paramId];
    switch (knobRange[paramId]){
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
    paramExtensions[paramId].factoryDflt = q->defaultValue;
    q->unit = unit[paramId] ? " \u00A2" : " V";
  }

  void appendCustomParamMenu(Menu *menu, int paramId){
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexSubmenuItem(
      "Knob range",
      {"0-1 V","0-2 V","0-5 V","0-10 V","+/- 1 V","+/- 2 V","+/- 5 V","+/- 10 V"},
      [=]() {
        return knobRange[paramId];
      },
      [=](int range) {
        knobRange[paramId] = range;
        configQuantity(paramId);
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Quantize",
      {"Off (Continuous)","Integers (Octaves)","1/12V (Semitones)"},
      [=]() {
        return quant[paramId];
      },
      [=](int val) {
        quant[paramId] = val;
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Display unit", 
      {"Volts (V)","Cents (\u00A2)"},
      [=]() {
        return unit[paramId];
      },
      [=](int val) {
        unit[paramId] = val;
        configQuantity(paramId);
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
  
  struct Knob5Quantity : ParamQuantity {
    float getDisplayValue() override {
      Knob5* mod = reinterpret_cast<Knob5*>(module);
      float val = ParamQuantity::getDisplayValue(); // Continuous
      switch (mod->quant[paramId]) {
        case 1: // Integer
          val = round(val);
          break;
        case 2: // Semitone
          val = round(val*12.f)/12.f;
      }
      if (mod->unit[paramId]) val *= 1200.f;
      return val;
    }
    void setDisplayValue(float val) override {
      Knob5* mod = reinterpret_cast<Knob5*>(module);
      ParamQuantity::setDisplayValue( mod->unit[paramId] ? val/1200.f : val);
    }
  };

  Knob5() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<5; i++) {
      std::string nm = "Knob " + std::to_string(i+1);
      configParam<Knob5Quantity>(KNOB_PARAM+i, 0.f, 1.f, 0.5f, nm, " V", 0.f, 20.f, -10.f);
      configOutput(OUTPUT+i, nm);
      paramExtensions[i].inputLink = false;
      paramExtensions[i].nameLink = i;
      outputExtensions[i].nameLink = i;
    }  
    clockDivider.setDivision(32);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (clockDivider.process()) {
      for (int i=0; i<5; i++){
        ParamQuantity *q = paramQuantities[i];
//        float out = q->getValue() * q->displayMultiplier + q->displayOffset;
        float out = params[i].getValue() * q->displayMultiplier + q->displayOffset;
        switch (quant[i]){
          case 1:  // integer
            out = round(out);
            break;
          case 2:  // semitone
            out = round(out*12.f)/12.f;
            break;
        }
        for (int c=0; c<poly[i]; c++)
          outputs[i].setVoltage(out,c);
        outputs[i].setChannels(poly[i]);
      } 
    }
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    for (int i=0; i<5; i++){
      std::string iStr = std::to_string(i);
      std::string nm = "knobRange"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(knobRange[i]));
      nm = "quant"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(quant[i]));
      nm = "unit"+iStr;
      json_object_set_new(rootJ, nm.c_str(), json_integer(unit[i]));
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
        knobRange[i] = json_integer_value(val);
      }
      nm = "quant"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        quant[i] = json_integer_value(val);
      }
      nm = "unit"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        unit[i] = json_integer_value(val);
      }
      nm = "poly"+iStr;
      if ((val = json_object_get(rootJ, nm.c_str()))){
        poly[i] = json_integer_value(val);
      }
      configQuantity(i);
    }
  }

};

struct Knob5Widget : VenomWidget {
  
  struct Knob : RoundSmallBlackKnobLockable {
    void appendContextMenu(Menu* menu) override {
      if (this->module) {
        static_cast<Knob5*>(this->module)->appendCustomParamMenu(menu, this->paramId);
        RoundSmallBlackKnobLockable::appendContextMenu(menu);
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
    Knob5* module = reinterpret_cast<Knob5*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Configure all knobs:"));
    menu->addChild(createIndexSubmenuItem(
      "Knob range",
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
            module->knobRange[i] = range;
            module->configQuantity(i);
          }
        }
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Quantize",
      {"Off (Continuous)","Integers (Octaves)","1/12V (Semitones)"},
      [=]() {
        int current = module->quant[0];
        for (int i=1; i<5; i++){
          if (current != module->quant[i])
            current = 3;
        }
        return current;
      },
      [=](int val) {
        if (val<3) {
          for (int i=0; i<5; i++){
            module->quant[i]=val;
          }
        }
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "Display unit",
      {"Volts (V)","Cents (\u00A2)"},
      [=]() {
        int current = module->unit[0];
        for (int i=1; i<5; i++){
          if (current != module->unit[i])
            current = 2;
        }
        return current;
      },
      [=](int val) {
        if (val<2) {
          for (int i=0; i<5; i++){
            module->unit[i]=val;
            module->configQuantity(i);
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

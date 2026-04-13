// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct NullCable : VenomModule {

  enum ParamId {
    ENUMS(GATE_PARAM,3),
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(GATE_INPUT,3),
    ENUMS(SIGNAL_INPUT,3),
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(SIGNAL_OUTPUT,3),
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(GATE_LIGHT,6),
    LIGHTS_LEN
  };
  
  bool buttonVal[3]{},
       cvVal[3]{},
       state[3]{true,true,true};
       
  int mode = 0;
  #define BLOCK 0
  #define PASS  1
  #define TOGGLE 2
  
  struct GateQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
      return static_cast<NullCable*>(this->module)->state[paramId] ? "Pass" : "Nullify";
    }
  };

  NullCable() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    
    for (int i=0; i<3; i++) {
      std::string istr = std::to_string(i+1);
      std::string nm = "Gate " + istr;
      configParam<GateQuantity>(GATE_PARAM+i, 0.f, 1.f, 0.f, nm);
      configInput(GATE_INPUT+i, nm);
      nm = "Signal " + istr;
      configInput(SIGNAL_INPUT+i, nm);
      configOutput(SIGNAL_OUTPUT+i, nm);
      configBypass(SIGNAL_INPUT+i, SIGNAL_OUTPUT+i);
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    for (int i=0; i<3; i++) {
      bool newButtonVal = params[GATE_PARAM+i].getValue();
      if (inputs[GATE_INPUT+i].isConnected()) {
        bool newCvVal = cvVal[i];
        if (inputs[GATE_INPUT+i].getVoltage()>=2.0f)
          newCvVal = true;
        if (inputs[GATE_INPUT+i].getVoltage()<=0.2f)
          newCvVal = false;
        switch (mode) {
          case BLOCK:
            state[i] = newButtonVal ? newCvVal : !newCvVal;
            break;
          case PASS:
            state[i] = newButtonVal ? !newCvVal : newCvVal;
            break;
          case TOGGLE:
            if (newCvVal != cvVal[i] && newCvVal)
              state[i] = !state[i];
            if (newButtonVal != buttonVal[i] && newButtonVal)
              state[i] = !state[i];
            break;
        }
        cvVal[i] = newCvVal;
      } else {
        cvVal[i] = false;
        if (newButtonVal != buttonVal[i] && newButtonVal)
          state[i] = !state[i];
      }
      buttonVal[i] = newButtonVal;
      if (state[i]) {
        outputs[SIGNAL_OUTPUT+i].channels = 16;
        int cnt = inputs[SIGNAL_INPUT+i].getChannels();
        for (int c=0; c<cnt; c++)
          outputs[SIGNAL_OUTPUT+i].setVoltage(inputs[SIGNAL_INPUT+i].getVoltage(c),c);
        outputs[SIGNAL_OUTPUT+i].setChannels(cnt);
      }
      else {
        outputs[SIGNAL_OUTPUT+i].channels = 0;
      }
    }
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "mode", json_integer(mode));
    json_t* jArray = json_array();
    for (int i=0; i<3; i++)
      json_array_append_new(jArray, json_boolean(state[i]));
    json_object_set_new(rootJ, "state", jArray);
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t *arrayVal, *val;
    size_t i;
    if ((val = json_object_get(rootJ, "mode")))
      mode = json_integer_value(val);
    if ((arrayVal = json_object_get(rootJ, "state"))){
      json_array_foreach(arrayVal, i, val) {
        state[i] = json_boolean_value(val);
      }
    }
  }

};

struct NullCableWidget : VenomWidget {

  NullCableWidget(NullCable* module) {
    setModule(module);
    setVenomPanel("NullCable");

    for (int i=0; i<3; i++) {
      float delta = i * 111.f;
      addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteRedLight<>>>>(Vec(22.5f, 38.f+delta), module, NullCable::GATE_PARAM+i, NullCable::GATE_LIGHT+i*2));
      addInput(createInputCentered<MonoPort>(Vec(22.5f, 65.f+delta), module, NullCable::GATE_INPUT+i));
      addInput(createInputCentered<PolyPort>(Vec(22.5f, 93.f+delta), module, NullCable::SIGNAL_INPUT+i));
      addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 121.5f+delta), module, NullCable::SIGNAL_OUTPUT+i));
    }
  }
  
  void appendContextMenu(Menu *menu) override {
    NullCable *module = static_cast<NullCable*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexPtrSubmenuItem("Gate mode", {"Nullify","Pass","Toggle"}, &module->mode));
    VenomWidget::appendContextMenu(menu);
  }  

  void step() override {
    VenomWidget::step();
    NullCable* module = dynamic_cast<NullCable*>(this->module);
    if(module) {
      for (int i=0; i<3; i++){
        module->lights[i*2+0].setBrightness(module->state[i]);
        module->lights[i*2+1].setBrightness(!module->state[i]);
      }
    }
  }
};

}

Model* modelVenomNullCable = createModel<Venom::NullCable, Venom::NullCableWidget>("NullCable");

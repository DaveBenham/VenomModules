// Venom Modules (c) 2023, 2024, 2025, 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct Split4x2 : VenomModule {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(POLY_INPUT,2),
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(POLY_OUTPUT,8),
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(DROP_LIGHT,2),
    LIGHTS_LEN
  };
   
  int outChannels[8]{};
  std::string outLabels[16]{"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"};
  
  void setOutputDescription(int id){
    outputInfos[id]->description = "Channels: "+outLabels[outChannels[id]];
  }
  
  Split4x2() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(0, "Port 1");
    configInput(1, "Port 2");
    configLight(0, "Input port 1 dropped channel(s) indicator");
    configLight(1, "Input port 2 dropped channel(s) indicator");
    for (int i=0; i<8; i++) {
      std::string name = "Port " + std::to_string(i+1);
      configOutput(i, name);
      setOutputDescription(i);
    }  
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    Input *activeInput = &inputs[0];
    int activeLight = 0,
        activeChannel = 0,
        inChannels = std::max(activeInput->getChannels(), 1);
    for (int i=0; i<4; i++) {
      for (int y=0; y<=outChannels[i]; y++){
        outputs[i].setVoltage(activeChannel<inChannels ? activeInput->getVoltage(activeChannel++) : 0.f, y);
      }
      outputs[i].setChannels(outChannels[i]+1);
    }
    if (inputs[1].isConnected()){
      lights[activeLight++].setBrightness(activeChannel<inChannels);
      activeInput = &inputs[1];
      activeChannel = 0;
      inChannels = std::max(activeInput->getChannels(), 1);
    } else lights[1].setBrightness(0);
    for (int i=4; i<8; i++) {
      for (int y=0; y<=outChannels[i]; y++){
        outputs[i].setVoltage(activeChannel<inChannels ? activeInput->getVoltage(activeChannel++) : 0.f, y);
      }
      outputs[i].setChannels(outChannels[i]+1);
    }
    lights[activeLight].setBrightness(activeChannel<inChannels);
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_t* array = json_array();
    for (int i=0; i<8; i++){
      json_array_append_new(array, json_integer(outChannels[i]));
    }
    json_object_set_new(rootJ, "outChannels", array);
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* array = json_object_get(rootJ, "outChannels");
    if (array){
      size_t i;
      json_t* val;
      json_array_foreach(array, i, val) {
        outChannels[i] = json_integer_value(val);
        setOutputDescription(i);
      }
    }
  }
  
};

struct Split4x2Widget : VenomWidget {

  struct OutPort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      Split4x2* module = static_cast<Split4x2*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createIndexSubmenuItem(
        "Channels",
        {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
        [=]() {
          return module->outChannels[portId];
        },
        [=](int cnt) {
          module->outChannels[portId] = cnt;
          module->setOutputDescription(portId);
        }
      ));    
      PolyPort::appendContextMenu(menu);
    }
  };

  template <class TWidget>
  TWidget* createConfigChannelsOutputCentered(math::Vec pos, engine::Module* module, int outputId) {
    TWidget* o = createOutputCentered<TWidget>(pos, module, outputId);
    o->portId = outputId;
    return o;
  }

  Split4x2Widget(Split4x2* module) {
    setModule(module);
    setVenomPanel("Split4x2");

    addInput(createInputCentered<PolyPort>(Vec(22.5f, 51.5f), module, 0));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 219.5f), module, 1));
    addChild(createLightCentered<SmallLight<RedLight>>(Vec(35.f, 41.f), module, 0));
    addChild(createLightCentered<SmallLight<RedLight>>(Vec(35.f, 209.f), module, 1));
    for (int i=0; i<4; i++){
      addOutput(createConfigChannelsOutputCentered<OutPort>(Vec(22.5f, 88.5f+28.f*i), module, i));
      addOutput(createConfigChannelsOutputCentered<OutPort>(Vec(22.5f, 256.5f+28.f*i), module, 4+i));
    }
  }

  void appendContextMenu(Menu* menu) override {
    Split4x2* module = static_cast<Split4x2*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Restore mono outputs", "",
      [=]() {
        for (int i=0; i<8; i++) {
          module->outChannels[i]=0;
          module->setOutputDescription(i);
        }
      }
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

}

Model* modelVenomSplit4x2 = createModel<Venom::Split4x2, Venom::Split4x2Widget>("Split4x2");

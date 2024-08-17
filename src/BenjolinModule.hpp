#include <bitset>

struct BenjolinModule : VenomModule {
  
  // expander only
  bool connected = false;
  
  enum GatesParamId {
    GATES_MODE_PARAM,
    GATES_POLARITY_PARAM,
    GATES_PARAMS_LEN
  };
  enum GatesInputId {
    GATES_INPUTS_LEN
  };
  enum GatesOutputId {
    ENUMS(GATE_OUTPUT,8),
    GATES_OUTPUTS_LEN
  };
  enum GatesLightId {
    GATES_EXP_LIGHT,
    ENUMS(GATE_LIGHT,8),
    GATES_LIGHTS_LEN
  };
 
  BenjolinModule* leftExpander = NULL;
  BenjolinModule* rightExpander = NULL;

  void onExpanderChange(const ExpanderChangeEvent& e) override {
    if (e.side)
      rightExpander = dynamic_cast<BenjolinModule*>(getRightExpander().module);
    else
      leftExpander = dynamic_cast<BenjolinModule*>(getLeftExpander().module);
  }

  inline int setCount(unsigned char num) {
    return static_cast<int>(std::bitset<8>(num).count());
  }

};

struct BenjolinGatesExpander : BenjolinModule {

  enum GateLogic{
    AND,
    OR,
    XOR
  };
  unsigned char gateBits[8]{1,2,4,8,16,32,64,128};
  int gateLogic[8]{};
  unsigned char oldVal[8]{};
  dsp::PulseGenerator trigGenerator[8];

  BenjolinGatesExpander() {
    venomConfig(GATES_PARAMS_LEN, GATES_INPUTS_LEN, GATES_OUTPUTS_LEN, GATES_LIGHTS_LEN);
    configLight(0, "Left connection indicator");
    configSwitch<FixedSwitchQuantity>(GATES_MODE_PARAM, 0.f, 6.f, 0.f, "Gate mode", {"Gate", "Clock gate", "Inverse clock gate",
                                                                                     "Trigger", "Clock rise trigger", "Clock fall trigger", "Clock edge trigger"});
    configSwitch<FixedSwitchQuantity>(GATES_POLARITY_PARAM, 0.f, 1.f, 0.f, "Polarity", {"Unipolar", "Bipolar"});
    for (int i=0; i<8; i++) {
      std::string label = string::f("%d", i + 1);
      configOutput(GATE_OUTPUT+i, label);
      configLight(GATE_LIGHT+i, label+" indicator");
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_t* array = json_array();
    for (int i=0; i<8; i++)
      json_array_append_new(array, json_integer(gateBits[i]));
    json_object_set_new(rootJ, "gateBits", array);
    array = json_array();
    for (int i=0; i<8; i++){
      json_array_append_new(array, json_integer(gateLogic[i]));
    }
    json_object_set_new(rootJ, "gateLogic", array);
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* array;
    json_t* val;
    size_t index;
    if ((array = json_object_get(rootJ, "gateBits"))) {
      json_array_foreach(array, index, val){
        gateBits[index] = json_integer_value(val);
      }
    }
    if ((array = json_object_get(rootJ, "gateLogic"))) {
      json_array_foreach(array, index, val){
        gateLogic[index] = json_integer_value(val);
      }
    }
    setPortNames();
  }
  
  void setPortName(int portId){
    std::string txt="";
    std::string op="";
    switch (gateLogic[portId]) {
      case AND:
        op="&";
        break;
      case OR:
        op="|";
        break;
      default:
        op="^";
    }
    std::string delim="";
    for (unsigned char bit=1, i=1; i<=8; bit<<=1, i++){
      if (gateBits[portId]&bit) {
        txt = string::f("%s%s%d", txt.c_str(), delim.c_str(), i);
        delim=op;
      }
    }
    PortInfo* pi = outputInfos[portId];
    PortExtension* e = &outputExtensions[portId];
    if (pi->name == e->factoryName)
      pi->name = txt;
    e->factoryName = txt;
  }
  
  void setPortNames(){
    for (int i=0; i<8; i++)
      setPortName(i);
  }
  
  void gateOutputMenu( Menu* menu, int portId ) {
    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Gate bits","",
      [=](Menu* menu) {
        for (unsigned char bit=1, id=1; id<=8; bit<<=1, id++){
          menu->addChild(createBoolMenuItem(std::to_string(id),"",
            [=]() { return gateBits[portId]&bit; },
            [=](bool val) {
              if (val && setCount(gateBits[portId])<4) gateBits[portId] |= bit;
              else gateBits[portId] &= ~bit;
              setPortName(portId);
            }
          ));
        }
      }
    ));
    menu->addChild(createIndexSubmenuItem("Gate logic",
      {"AND", "OR", "XOR"},
      [=]() {
        return gateLogic[portId];
      },
      [=](int val) {
        gateLogic[portId] = val;
        setPortName(portId);
      }
    ));
  }  
};


struct BenjolinExpanderWidget : VenomWidget {
  void step() override {
    bool connected = false;
    BenjolinModule* thisMod = static_cast<BenjolinModule*>(this->module);
    BenjolinModule* leftMod = thisMod ? thisMod->leftExpander : NULL;
    while(leftMod) {
      if (leftMod->model == modelBenjolinOsc) {
        connected = true;
        break;
      }
      leftMod = leftMod->leftExpander;
    }
    if(thisMod && thisMod->connected != connected) {
      thisMod->connected = connected;
      thisMod->lights[0].setBrightness(connected);
      if (!connected){
        for (int i=0; i<thisMod->getNumOutputs(); i++) {
          thisMod->outputs[i].setVoltage(0.f);
          thisMod->outputs[i].setChannels(1);
          if (thisMod->model == modelBenjolinGatesExpander)
            thisMod->lights[BenjolinModule::GATE_LIGHT+i].setBrightness(0);
        }
      }
    }
    
    VenomWidget::step();
  }  
};

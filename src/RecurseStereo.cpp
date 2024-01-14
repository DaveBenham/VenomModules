// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

struct RecurseStereo : VenomModule {
  enum ParamId {
    COUNT_PARAM,
    SCALE_PARAM,
    OFFSET_PARAM,
    TIMING_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    SCALE_INPUT,
    OFFSET_INPUT,
    RETURN_L_INPUT,
    RETURN_R_INPUT,
    IN_L_INPUT,
    IN_R_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    SEND_L_OUTPUT,
    SEND_R_OUTPUT,
    OUT_L_OUTPUT,
    OUT_R_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    SCALE_LIGHT,
    OFFSET_LIGHT,
    LIGHTS_LEN
  };

  int recurCount = 1;
  bool recurCountErr = false;
  int order = 0;
  int oldOrder = -1;

  enum ModTiming {
    PRE_START_1,
    PRE_START_N,
    POST_RETURN_N,
    POST_RETURN_1
  };

  RecurseStereo() {
    struct TimingQuantity : ParamQuantity {
      std::string getDisplayValueString() override {
        RecurseStereo* module = reinterpret_cast<RecurseStereo*>(this->module);
        int val = static_cast<int>(module->params[RecurseStereo::TIMING_PARAM].getValue());
        switch (val) {
          case 0: return "0 = Before 1st send";
          case 1: return "1 = Before all sends";
          case 2: return "2 = After all returns";
          case 3: return "3 = After last return";
          default: return "Error";
        };
      }
    };
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(COUNT_PARAM, 1.f, 16.f, 1.f, "Recursion Count", "");
    configParam(SCALE_PARAM, -10.f, 10.f, 1.f, "Scale", "");
    configParam(OFFSET_PARAM, -10.f, 10.f, 0.f, "Offset", " V");
    configParam<TimingQuantity>(TIMING_PARAM, 0.f, 3.f, 0.f, "Modulation Timing", "");
    configInput(SCALE_INPUT, "Scale");
    configInput(OFFSET_INPUT, "Offset");
    configInput(RETURN_L_INPUT, "Left Return");
    configInput(RETURN_R_INPUT, "Right Return");
    configInput(IN_L_INPUT, "Left Signal");
    configInput(IN_R_INPUT, "Right Signal");
    configOutput(SEND_L_OUTPUT, "Left Send");
    configOutput(SEND_R_OUTPUT, "Right Send");
    configOutput(OUT_L_OUTPUT, "Left Signal");
    configOutput(OUT_R_OUTPUT, "Right Signal");
    configBypass(IN_L_INPUT, OUT_L_OUTPUT);
    configBypass(inputs[IN_R_INPUT].isConnected() ? IN_R_INPUT : IN_L_INPUT, OUT_R_OUTPUT);
  }

  void onPortChange(const PortChangeEvent& e) override {
    if (e.type == Port::INPUT && e.portId == IN_R_INPUT)
      bypassRoutes[1].inputId = e.connecting ? IN_R_INPUT : IN_L_INPUT;
  }

  void onReset() override {
    recurCount = 1;
    recurCountErr = false;
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    recurCount = static_cast<int>(params[COUNT_PARAM].getValue());
    int inChannels = std::max({1, inputs[IN_L_INPUT].getChannels()});
    recurCountErr = (inChannels > 16 / recurCount);
    int outChannels = recurCountErr ? 16 / recurCount : inChannels;
    ModTiming timing = static_cast<ModTiming>(params[TIMING_PARAM].getValue());
    float scaleParam = params[SCALE_PARAM].getValue();
    float offsetParam = params[OFFSET_PARAM].getValue();
    bool sendConnectedL = outputs[SEND_L_OUTPUT].isConnected();
    bool sendConnectedR = outputs[SEND_R_OUTPUT].isConnected();
    bool returnConnectedL = inputs[RETURN_L_INPUT].isConnected();
    bool returnConnectedR = inputs[RETURN_R_INPUT].isConnected();
    bool mod = inputs[SCALE_INPUT].isConnected() || inputs[OFFSET_INPUT].isConnected() || scaleParam != 1.0f || offsetParam != 0.0f;
    bool right = sendConnectedR || outputs[OUT_R_OUTPUT].isConnected();
    float scale = 0.f;
    float offset = 0.f;
    float rtnL = 0.f;
    float rtnR = 0.f;
    for (int c=0; c<outChannels; c++) {
      rtnL = inputs[IN_L_INPUT].getVoltage(c);
      if(right)
        rtnR = inputs[IN_R_INPUT].getNormalPolyVoltage(rtnL, c);
      if (mod) {
        scale = inputs[SCALE_INPUT].getNormalPolyVoltage(1.0f, c) * scaleParam;
        offset = inputs[OFFSET_INPUT].getNormalPolyVoltage(0.0f, c) + offsetParam;
        if (timing == PRE_START_1){
          rtnL = order==0 ? rtnL * scale + offset : (rtnL + offset) * scale;
          if (right)
            rtnR = order==0 ? rtnR * scale + offset : (rtnR + offset) * scale;
        }
      }
      for (int o=c*recurCount, end=o+recurCount; o<end; o++) {
        if (mod && timing == PRE_START_N) {
          rtnL = order==0 ? rtnL * scale + offset : (rtnL + offset) * scale;
          if (right)
            rtnR = order==0 ? rtnR * scale + offset : (rtnR + offset) * scale;
        }
        if (sendConnectedL)
          outputs[SEND_L_OUTPUT].setVoltage(rtnL, o);
        if (sendConnectedR)
          outputs[SEND_R_OUTPUT].setVoltage(rtnR, o);
        if (returnConnectedL)
          rtnL = inputs[RETURN_L_INPUT].getVoltage(o);
        if (returnConnectedR)
          rtnR = inputs[RETURN_R_INPUT].getVoltage(o);
        if (mod && timing == POST_RETURN_N){
          rtnL = order==0 ? rtnL * scale + offset : (rtnL + offset) * scale;
          if (right)
            rtnR = order==0 ? rtnR * scale + offset : (rtnR + offset) * scale;
        }
      }
      if (mod && timing == POST_RETURN_1){
        rtnL = order==0 ? rtnL * scale + offset : (rtnL + offset) * scale;
        if (right)
          rtnR = order==0 ? rtnR * scale + offset : (rtnR + offset) * scale;
      }
      outputs[OUT_L_OUTPUT].setVoltage(rtnL, c);
      if (right)
        outputs[OUT_R_OUTPUT].setVoltage(rtnR, c);
    }
    for (int c=outChannels; c<inChannels; c++){
      outputs[OUT_L_OUTPUT].setVoltage(0.0f, c);
      if (right)
        outputs[OUT_R_OUTPUT].setVoltage(0.0f, c);
    }
    outputs[SEND_L_OUTPUT].setChannels(outChannels * recurCount);
    outputs[OUT_L_OUTPUT].setChannels(outChannels);
    if (right) {
      outputs[SEND_R_OUTPUT].setChannels(outChannels * recurCount);
      outputs[OUT_R_OUTPUT].setChannels(outChannels);
    }

    if (order != oldOrder) {
      oldOrder = order;
      lights[SCALE_LIGHT].setBrightness(order==0);
      lights[OFFSET_LIGHT].setBrightness(order==1);
    }

  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "orderOp", json_integer(order));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val = json_object_get(rootJ, "orderOp");
    if (val)
      order = json_integer_value(val);
  }

};


struct RecurseStereoWidget : VenomWidget {

  struct CountDisplay : DigitalDisplay18 {
    void step() override {
      if (module) {
        RecurseStereo* mod = dynamic_cast<RecurseStereo*>(module);
        text = string::f("%d", mod->recurCount);
        fgColor = mod->recurCountErr ? SCHEME_RED : SCHEME_YELLOW;
      } else {
        text = "16";
        fgColor = SCHEME_YELLOW;
      }
    }
  };

  RecurseStereoWidget(RecurseStereo* module) {
    setModule(module);
    setVenomPanel("RecurseStereo");
    CountDisplay* countDisplay = createWidget<CountDisplay>(mm2px(Vec(3.5, 10.9)));
    countDisplay->module = module;
    addChild(countDisplay);
    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(mm2px(Vec(18.134, 14.97)), module, RecurseStereo::COUNT_PARAM));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(7.299, 29.4)), module, RecurseStereo::SCALE_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(18.136, 29.4)), module, RecurseStereo::SCALE_PARAM));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(7.297, 43.85)), module, RecurseStereo::OFFSET_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(18.134, 43.85)), module, RecurseStereo::OFFSET_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(mm2px(Vec(12.7155, 55.60)), module, RecurseStereo::TIMING_PARAM));

    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(7.297, 72.65)), module, RecurseStereo::SEND_L_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(18.134, 72.65)), module, RecurseStereo::SEND_R_OUTPUT));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(7.297, 87.1)), module, RecurseStereo::RETURN_L_INPUT));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(18.134, 87.1)), module, RecurseStereo::RETURN_R_INPUT));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(7.297, 101.55)), module, RecurseStereo::IN_L_INPUT));
    addInput(createInputCentered<PolyPort>(mm2px(Vec(18.134, 101.55)), module, RecurseStereo::IN_R_INPUT));
    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(7.297, 116.0)), module, RecurseStereo::OUT_L_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(mm2px(Vec(18.134, 116.0)), module, RecurseStereo::OUT_R_OUTPUT));

    addChild(createLightCentered<TinyLight<YlwLight<>>>(mm2px(Vec(12.7115, 29.4)), module, RecurseStereo::SCALE_LIGHT));
    addChild(createLightCentered<TinyLight<YlwLight<>>>(mm2px(Vec(12.7115, 43.85)), module, RecurseStereo::OFFSET_LIGHT));
  }

  void appendContextMenu(Menu* menu) override {
    RecurseStereo* module = dynamic_cast<RecurseStereo*>(this->module);
    assert(module);
    menu->addChild(new MenuSeparator);
    std::vector<std::string> orderLabels;
    orderLabels.push_back("Scale before offset");
    orderLabels.push_back("Offset before scale");
    menu->addChild(createIndexSubmenuItem("Order of operation", orderLabels,
      [=]() {return module->order;},
      [=](int i) {module->order = i;}
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelRecurseStereo = createModel<RecurseStereo, RecurseStereoWidget>("RecurseStereo");

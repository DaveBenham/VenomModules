// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

struct PolyScale : VenomModule {
  enum ParamId {
    ENUMS(LEVEL_PARAM,16),
    PARAMS_LEN
  };
  enum InputId {
    POLY_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    POLY_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  struct Range {
    float scale;
    float offset;
    float dflt;
  };
  Range ranges[8]{
    // 0-1          0-2            0-5            0-10
    {1.f,0.f,1.f}, {2.f,0.f,0.5f}, {5.f,0.f,0.2f}, {10.f,0.f,0.1f},
    // +/- 1          +/- 2            +/- 5             +/- 10
    {2.f,-1.f,1.f}, {4.f,-2.f,0.75f}, {10.f,-5.f,0.6f}, {20.f,-10.f,0.55f}
  };
  int rangeId = 0;
  int channels = 0;
  
  void setRange(int val) {
    rangeId = val;
    for (int i=0; i<16; i++){
      ParamQuantity *q = paramQuantities[LEVEL_PARAM+i];
      Range* r = &ranges[rangeId];
      q->defaultValue = r->dflt;
      q->displayMultiplier = r->scale;
      q->displayOffset = r->offset;
      paramExtensions[LEVEL_PARAM+i].factoryDflt = r->dflt;
    }
  }
  
  PolyScale() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<16; i++) {
      std::string nm = "Level " + std::to_string(i+1);
      configParam(LEVEL_PARAM+i, 0.f, 1.f, 1.f, nm, "x");
    }
    configInput(POLY_INPUT,"Poly");
    configOutput(POLY_OUTPUT,"Poly");
    configBypass(POLY_INPUT, POLY_OUTPUT);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    using float_4 = simd::float_4;
    float_4 scale{};
    int cnt = channels ? channels : inputs[POLY_INPUT].getChannels();
    for (int i=0; i<cnt; i+=4){
      for (int j=0; j<4; j++)
        scale[j] = params[LEVEL_PARAM+i+j].getValue();
      outputs[POLY_OUTPUT].setVoltageSimd(inputs[POLY_INPUT].getPolyVoltageSimd<float_4>(i) * (scale * ranges[rangeId].scale + ranges[rangeId].offset), i);
    }
    outputs[POLY_OUTPUT].setChannels(cnt);
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "rangeId", json_integer(rangeId));
    json_object_set_new(rootJ, "channels", json_integer(channels));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "rangeId")))
      setRange(json_integer_value(val));
    if ((val = json_object_get(rootJ, "channels")))
      channels = json_integer_value(val);
  }

};

struct PolyScaleWidget : VenomWidget {
  
  struct PCCountDisplay : DigitalDisplay18 {
    void step() override {
      if (module) {
        PolyScale* mod = static_cast<PolyScale*>(module);
        if (mod->channels){
          text = string::f("%d", mod->channels);
          fgColor = mod->inputs[PolyScale::POLY_INPUT].getChannels() > mod->channels ? SCHEME_RED : SCHEME_YELLOW;
        } else {
          text = string::f("%d", mod->inputs[PolyScale::POLY_INPUT].getChannels());
          fgColor = SCHEME_YELLOW;
        }
      } else {
        text = "16";
        fgColor = SCHEME_YELLOW;
      }
    }
  };

  PolyScaleWidget(PolyScale* module) {
    setModule(module);
    setVenomPanel("PolyScale");
    float y=64.5f;
    for (int i=0; i<8; i++, y+=24.f){
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.f, y), module, PolyScale::LEVEL_PARAM+i));
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(33.f, y), module, PolyScale::LEVEL_PARAM+i+8));
    }

    PCCountDisplay* countDisplay = createWidget<PCCountDisplay>(Vec(10.316, 252.431));
    countDisplay->module = module;
    countDisplay->fgColor = SCHEME_YELLOW;
    addChild(countDisplay);

    addInput(createInputCentered<PolyPort>(Vec(22.5,300.5), module, PolyScale::POLY_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5,339.5), module, PolyScale::POLY_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    PolyScale* module = dynamic_cast<PolyScale*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexSubmenuItem(
      "Level range",
      {"0-1x","0-2x","0-5x","0-10x","+/- 1x","+/- 2x","+/- 5x","+/- 10x"},
      [=]() {
        return module->rangeId;
      },
      [=](int rangeId) {
        module->setRange(rangeId);
      }
    ));
    menu->addChild(createIndexPtrSubmenuItem(
      "Polyphony channels",
      {"Auto","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
      &module->channels
    ));
    VenomWidget::appendContextMenu(menu);
  }
};

Model* modelPolyScale = createModel<PolyScale, PolyScaleWidget>("PolyScale");

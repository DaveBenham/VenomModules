// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct PolyOffset : VenomModule {
  enum ParamId {
    ENUMS(OFFSET_PARAM,16),
    MIN_PARAM,
    MAX_PARAM,
    QUANT_PARAM,
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
  Range ranges[9]{
    // 0-1          0-2            0-5            0-10
    {1.f,0.f,0.f}, {2.f,0.f,0.f}, {5.f,0.f,0.f}, {10.f,0.f,0.f},
    // +/- 1          +/- 2            +/- 5             +/- 10             custom
    {2.f,-1.f,0.5f}, {4.f,-2.f,0.5f}, {10.f,-5.f,0.5f}, {20.f,-10.f,0.5f}, {20.f,-10.f,0.5f}
  };
  int rangeId = 7;
  int quant = 0;
  int unit = 0;
  int channels = 0;
  
  void setRange(int val) {
    rangeId = val;
    Range* r = &ranges[val];
    for (int i=0; i<16; i++){
      ParamQuantity *q = paramQuantities[OFFSET_PARAM+i];
      q->defaultValue = r->dflt;
      q->displayMultiplier = r->scale;
      q->displayOffset = r->offset;
      paramExtensions[OFFSET_PARAM+i].factoryDflt = r->dflt;
    }
  }
  
  void setUnit(int val) {
    unit = val;
    std::string unitStr = unit ? " \u00A2" : " V";
    for (int i=0; i<16; i++){
      paramQuantities[OFFSET_PARAM+i]->unit = unitStr;
    }
  }
  
  struct OffsetQuantity : ParamQuantity {
    float getDisplayValue() override {
      PolyOffset* mod = reinterpret_cast<PolyOffset*>(module);
      float val = ParamQuantity::getDisplayValue(); // Continuous
      switch (mod->quant) {
        case 4: // Integer
          val = round(val);
          break;
        case 5: // Semitone
          val = round(val*12.f)/12.f;
          break;
        case 6: // Custom
          val = round(val/mod->params[QUANT_PARAM].getValue())*mod->params[QUANT_PARAM].getValue();
          break;
      }
      if (mod->unit) val *= 1200.f;
      return val;
    }
    void setDisplayValue(float val) override {
      PolyOffset* mod = reinterpret_cast<PolyOffset*>(module);
      ParamQuantity::setDisplayValue( mod->unit ? val/1200.f : val);
    }
  };

  PolyOffset() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<16; i++) {
      std::string nm = "Offset " + std::to_string(i+1);
      configParam<OffsetQuantity>(OFFSET_PARAM+i, 0.f, 1.f, 0.5f, nm, " V", 0.f, 20.f, -10.f);
    }
    ParamQuantity *q=configParam(MIN_PARAM, -100.f, 100.f, -10.f, "Custom min");
    q->resetEnabled = false;
    q->randomizeEnabled = false;
    q=configParam(MAX_PARAM, -100.f, 100.f, 10.f, "Custom max");
    q->resetEnabled = false;
    q->randomizeEnabled = false;
    q=configParam(QUANT_PARAM, 0.f, 100.f, 1.f, "Custom quant");
    q->resetEnabled = false;
    q->randomizeEnabled = false;
    configInput(POLY_INPUT,"Poly");
    configOutput(POLY_OUTPUT,"Poly");
    configBypass(POLY_INPUT, POLY_OUTPUT);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    using float_4 = simd::float_4;
    float_4 offset{};
    float_4 voltage{};
    int cnt = channels ? channels : inputs[POLY_INPUT].getChannels();
    for (int i=0; i<cnt; i+=4){
      for (int j=0; j<4; j++)
        offset[j] = params[OFFSET_PARAM+i+j].getValue();
      voltage = (offset * ranges[rangeId].scale) + ranges[rangeId].offset;
      switch (quant) {
        case 4: // Quantize offset to Integer
          voltage = simd::round(voltage);
          break;
        case 5: // Quantize offset to Semitone
          voltage = simd::round(voltage*12.f)/12.f;
          break;
        case 6: // Quantize offset to custom
          voltage = simd::round(voltage/params[QUANT_PARAM].getValue())*params[QUANT_PARAM].getValue();
      }
      voltage += inputs[POLY_INPUT].getPolyVoltageSimd<float_4>(i);
      switch (quant) {
        case 1: // Quantize output to Integer
          voltage = simd::round(voltage);
          break;
        case 2: // Quantize output to Semitone
          voltage = simd::round(voltage*12.f)/12.f;
          break;
        case 3: // Quantize output to custom
          voltage = simd::round(voltage/params[QUANT_PARAM].getValue())*params[QUANT_PARAM].getValue();
      }
      outputs[POLY_OUTPUT].setVoltageSimd(voltage, i);
    }
    outputs[POLY_OUTPUT].setChannels(cnt);
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "rangeId", json_integer(rangeId));
    json_object_set_new(rootJ, "quantV2", json_integer(quant));
    json_object_set_new(rootJ, "unit", json_integer(unit));
    json_object_set_new(rootJ, "channels", json_integer(channels));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "rangeId")))
      setRange(json_integer_value(val));
    if ((val = json_object_get(rootJ, "quantV2")))
      quant = json_integer_value(val);
    if ((val = json_object_get(rootJ, "quant"))){
      quant = json_integer_value(val);
      if (quant>2)
        quant++;
    }
    if ((val = json_object_get(rootJ, "unit")))
      setUnit(json_integer_value(val));
    if ((val = json_object_get(rootJ, "channels")))
      channels = json_integer_value(val);
  }

};

struct PolyOffsetWidget : VenomWidget {
  
  struct PCCountDisplay : DigitalDisplay18 {
    void step() override {
      if (module) {
        PolyOffset* mod = static_cast<PolyOffset*>(module);
        if (mod->channels){
          text = string::f("%d", mod->channels);
          fgColor = mod->inputs[PolyOffset::POLY_INPUT].getChannels() > mod->channels ? SCHEME_RED : SCHEME_YELLOW;
        } else {
          text = string::f("%d", mod->inputs[PolyOffset::POLY_INPUT].getChannels());
          fgColor = SCHEME_YELLOW;
        }
      } else {
        text = "16";
        fgColor = SCHEME_YELLOW;
      }
    }
  };

  PolyOffsetWidget(PolyOffset* module) {
    setModule(module);
    setVenomPanel("PolyOffset");
    float y=64.5f;
    for (int i=0; i<8; i++, y+=24.f){
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.f, y), module, PolyOffset::OFFSET_PARAM+i));
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(33.f, y), module, PolyOffset::OFFSET_PARAM+i+8));
    }

    PCCountDisplay* countDisplay = createWidget<PCCountDisplay>(Vec(10.316, 252.431));
    countDisplay->module = module;
    addChild(countDisplay);

    addInput(createInputCentered<PolyPort>(Vec(22.5,300.5), module, PolyOffset::POLY_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5,339.5), module, PolyOffset::POLY_OUTPUT));
  };

  void appendContextMenu(Menu* menu) override {
    PolyOffset* module = dynamic_cast<PolyOffset*>(this->module);
    ParamQuantity* minq = module->paramQuantities[PolyOffset::MIN_PARAM];
    ParamQuantity* maxq = module->paramQuantities[PolyOffset::MAX_PARAM];
    ParamQuantity* quantq = module->paramQuantities[PolyOffset::QUANT_PARAM];
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexSubmenuItem(
      "Offset range",
      {"0-1 V","0-2 V","0-5 V","0-10 V","+/- 1 V","+/- 2 V","+/- 5 V","+/- 10 V","Custom"},
      [=]() {
        return module->rangeId;
      },
      [=](int rangeId) {
        module->setRange(rangeId);
      }
    ));
    if (module->rangeId == 8) {
      menu->addChild(createSubmenuItem("Custom min", minq->getDisplayValueString(),
        [=](Menu *menu){
          MenuTextField *editField = new MenuTextField();
          editField->box.size.x = 150;
          editField->setText(minq->getDisplayValueString());
          editField->changeHandler = [=](std::string text) {
            minq->setDisplayValueString(text);
            module->ranges[8].offset = minq->getValue();
            module->ranges[8].scale  = maxq->getValue() - minq->getValue();
            module->setRange(8);
          };
          menu->addChild(editField);
        }
      ));
      menu->addChild(createSubmenuItem("Custom max", maxq->getDisplayValueString(),
        [=](Menu *menu){
          MenuTextField *editField = new MenuTextField();
          editField->box.size.x = 150;
          editField->setText(maxq->getDisplayValueString());
          editField->changeHandler = [=](std::string text) {
            maxq->setDisplayValueString(text);
            module->ranges[8].offset = minq->getValue();
            module->ranges[8].scale  = maxq->getValue() - minq->getValue();
            module->setRange(8);
          };
          menu->addChild(editField);
        }
      ));
    }
    menu->addChild(createIndexPtrSubmenuItem(
      "Quantize",
      {"Off (Continuous)","Output to Integers (Octaves)","Output to 1/12V (Semitones)","Output to custom interval","Offset to Integers (Octaves)","Offset to 1/12V (Semitones)","Offset to custom interval"},
      &module->quant
    ));
    if (module->quant == 3 || module->quant == 6) {
      menu->addChild(createSubmenuItem("Custom quantize interval", quantq->getDisplayValueString(),
        [=](Menu *menu){
          MenuTextField *editField = new MenuTextField();
          editField->box.size.x = 150;
          editField->setText(quantq->getDisplayValueString());
          editField->changeHandler = [=](std::string text) {
            quantq->setDisplayValueString(text);
            if (quantq->getValue() == 0.f)
              quantq->setValue(1.f);
          };
          menu->addChild(editField);
        }
      ));
    }
    menu->addChild(createIndexSubmenuItem(
      "Display unit", 
      {"Volts (V)","Cents (\u00A2)"},
      [=]() {
        return module->unit;
      },
      [=](int val) {
        module->setUnit(val);
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

}

Model* modelVenomPolyOffset = createModel<Venom::PolyOffset, Venom::PolyOffsetWidget>("PolyOffset");

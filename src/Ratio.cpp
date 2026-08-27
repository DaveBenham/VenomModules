// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include <float.h>

namespace Venom {

struct Ratio : VenomModule {
  enum ParamId {
    NUM_PARAM,
    NUM_CV_PARAM,
    NUM_QUANT_PARAM,
    DEN_PARAM,
    DEN_CV_PARAM,
    DEN_QUANT_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    NUM_CV_INPUT,
    DEN_CV_INPUT,
    ROOT_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    POLY_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  using float_4 = simd::float_4;

  int numVal = 1,
      denVal = 1,
      monitor = 0;

  Ratio() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(NUM_PARAM, 1.f, 128.f, 1.f, "Numerator");
    configParam(NUM_CV_PARAM, -1.f, 1.f, 0.f, "Numerator CV", "%", 0.f, 100.f, 0.f);
    configParam(NUM_QUANT_PARAM, 1.f, 8.f, 1.f, "Numerator CV step");
    configInput(NUM_CV_INPUT, "Numerator CV");
    configParam(DEN_PARAM, 1.f, 128.f, 1.f, "Denominator");
    configParam(DEN_CV_PARAM, -1.f, 1.f, 0.f, "Denominator CV", "%", 0.f, 100.f, 0.f);
    configParam(DEN_QUANT_PARAM, 1.f, 8.f, 1.f, "Denominator CV step");
    configInput(DEN_CV_INPUT, "Denominator CV");
    configInput(ROOT_INPUT, "Root");
    configOutput(POLY_OUTPUT, "V/Oct");
    configBypass(ROOT_INPUT, POLY_OUTPUT);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++){
      if(inputs[i].getChannels() > channels)
        channels = inputs[i].getChannels();
    }
    float numParam = params[NUM_PARAM].getValue(),
          denParam = params[DEN_PARAM].getValue(),
          numAmt = params[NUM_CV_PARAM].getValue(),
          denAmt = params[DEN_CV_PARAM].getValue(),
          numQuant = params[NUM_QUANT_PARAM].getValue(),
          denQuant = params[DEN_QUANT_PARAM].getValue();

    for (int s=0, c=0; c<channels; s++, c+=4) {
      float_4 num = clamp(numParam + round(inputs[NUM_CV_INPUT].getPolyVoltageSimd<float_4>(c) * numAmt * 10.f / numQuant + 0.5f) * numQuant, 1.f, 199.f),
              den = clamp(denParam + round(inputs[DEN_CV_INPUT].getPolyVoltageSimd<float_4>(c) * denAmt * 10.f / denQuant + 0.5f) * denQuant, 1.f, 199.f);
      if (s == monitor / 4) {
        numVal = num[monitor % 4];
        denVal = den[monitor % 4];
      }
      outputs[POLY_OUTPUT].setVoltageSimd(inputs[ROOT_INPUT].getPolyVoltageSimd<float_4>(c) + log2(num/den), c);
    }
    outputs[POLY_OUTPUT].setChannels(channels);
    if (monitor >= channels) {
      numVal = 0;
      denVal = 0;
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "monitor", json_integer(monitor));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if((val = json_object_get(rootJ, "monitor")))
      monitor = json_integer_value(val);
  }

};


struct RatioWidget : VenomWidget {

  struct IntDisplay : DigitalDisplay188 {
    int dflt = 1;
    int *val = &dflt;
    void step() override {
      text = *val ? string::f("%d", *val) : "";
    }
  };

  RatioWidget(Ratio* module) {
    setModule(module);
    setVenomPanel("Ratio");
    IntDisplay *numDisplay = createWidget<IntDisplay>(Vec(4.975f, 119.7f)),
               *denDisplay = createWidget<IntDisplay>(Vec(4.975f, 152.735f));
    numDisplay->module = denDisplay->module = module;
    if (module) {
      numDisplay->val = &module->numVal;
      denDisplay->val = &module->denVal;
    }  
    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(Vec(22.5f, 47.f), module, Ratio::NUM_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.f, 74.5f), module, Ratio::NUM_CV_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundTinyBlackKnobLockable>>(Vec(33.f, 74.4f), module, Ratio::NUM_QUANT_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 102.f), module, Ratio::NUM_CV_INPUT));
    addChild(numDisplay);
    addChild(denDisplay);
    addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(Vec(22.5f, 202.5f), module, Ratio::DEN_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.f, 230.f), module, Ratio::DEN_CV_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundTinyBlackKnobLockable>>(Vec(33.f, 230.f), module, Ratio::DEN_QUANT_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 257.5f), module, Ratio::DEN_CV_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 298.5f), module, Ratio::ROOT_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 339.5f), module, Ratio::POLY_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    Ratio* module = static_cast<Ratio*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexSubmenuItem("Monitor channel",
      {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
      [=]() {return module->monitor;},
      [=](int i) {module->monitor = i;}
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

}

Model* modelVenomRatio = createModel<Venom::Ratio, Venom::RatioWidget>("Ratio");

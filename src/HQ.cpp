// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include <float.h>

namespace Venom {

struct HQ : VenomModule {
  enum ParamId {
    PARTIAL_PARAM,
    SERIES_PARAM,
    CV_PARAM,
    DETUNE_AMT_PARAM,
    DETUNE_COMP_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    CV_INPUT,
    ROOT_INPUT,
    IN_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUT_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  enum SeriesLabels {
    ALL,
    ODD,
    EVEN
  };

  struct PartialRec {
    int partial;
    float voct;
    float allMin;
    float allMax;
    float oddMin;
    float oddMax;
    float evenMin;
    float evenMax;
  };

  PartialRec partials[128] = {
    #include "HQ.data"
  };

  int monitor = 0;
  int monitorVal = 0;
  enum monitorEnum {MONITOR_OFF=999};
  int range = 0;
  int oldRange = -1;
  int ranges[12][2] = {
    {0,15}, {0,31}, {0,63}, {0,127},
    {-15,0}, {-31,0}, {-63,0}, {-127,0},
    {-15,15}, {-31,31}, {-63,63}, {-127,127}
  };
  int allScales[12][2] /*{offset,scale}*/ = {
    {0,15}, {0,31}, {0,63}, {0,127},
    {-15,15}, {-31,31}, {-63,63}, {-127,127},
    {-15,30}, {-31,62}, {-63,126}, {-127,254}
  };
  int oddScales[12][2] = {
    {0,7}, {0,15}, {0,31}, {0,63},
    {-7,7}, {-15,15}, {-31,31}, {-63,63},
    {-7,14}, {-15,30}, {-31,62}, {-63,126}
  };
  int evenScales[12][2] = {
    {0,8}, {0,16}, {0,32}, {0,64},
    {-8,8}, {-16,16}, {-32,32}, {-64,64},
    {-8,16}, {-16,32}, {-32,64}, {-64,128}
  };

  int partialParamGetValue() {
    float val = params[PARTIAL_PARAM].getValue();
    int rtn;
    switch (static_cast<int>(params[HQ::SERIES_PARAM].getValue())) {
      case ALL:
        return static_cast<int>(round(val * allScales[range][1] + allScales[range][0]));
      case ODD:
        return static_cast<int>(round(val * oddScales[range][1] + oddScales[range][0])) * 2;
      case EVEN:
      default:
        rtn = static_cast<int>(round(val * evenScales[range][1] + evenScales[range][0])) * 2;
        return rtn + (rtn > 0 ? -1 : rtn < 0 ? 1 : 0);
    }
  }

  struct PartialQuantity : ParamQuantity {
    float getDisplayValue() override {
      HQ* module = reinterpret_cast<HQ*>(this->module);
      int val = module->partialParamGetValue();
      return val + (val<0 ? -1 : 1);
    }
    void setDisplayValue(float v) override {
      HQ* module = reinterpret_cast<HQ*>(this->module);
      int val = static_cast<int>(v);
      int range = module->range;
      switch (static_cast<int>(module->params[HQ::SERIES_PARAM].getValue())) {
        case HQ::ALL:
          v = static_cast<float>(val - module->allScales[range][0] - (val > 0 ? 1 : val < 0 ? -1 : 0)) / static_cast<float>(module->allScales[range][1]);
          break;
        case HQ::ODD:
          v = static_cast<float>((val - (val > 0 ? 1 : val < 0 ? -1 : 0))/2 - module->oddScales[range][0]) / static_cast<float>(module->oddScales[range][1]);
          break;
        case EVEN:
          v = static_cast<float>(val/2 - module->evenScales[range][0]) / static_cast<float>(module->evenScales[range][1]);
          break;
      }
      setValue(clamp(v,0.f,1.f));
    }
  };
  
  simd::float_4 out[4]{};

  HQ() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch<FixedSwitchQuantity>(SERIES_PARAM, 0, 2, 0, "Harmonic Series", {"All", "Odd", "Even"});
    configParam<PartialQuantity>(PARTIAL_PARAM, 0.f, 1.f, 0.f, "Partial", "");
    configParam(CV_PARAM, -1.f, 1.f, 0.f, "CV", "%", 0.f, 100.f, 0.f);
    configInput(CV_INPUT, "CV");
    configParam(DETUNE_AMT_PARAM, -1.f, 1.f, 0.f, "Detune amount");
    configParam(DETUNE_COMP_PARAM, 1.f, 2.f, 2.f, "Detune frequency compensation", "", 0.f, 1.f, -1.f);
    configInput(ROOT_INPUT, "Root");
    configInput(IN_INPUT, "V/Oct");
    configOutput(OUT_OUTPUT, "V/Oct");
    configBypass(IN_INPUT, OUT_OUTPUT);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (oldRange != range) {
      paramQuantities[PARTIAL_PARAM]->defaultValue = range < 4 ? 0.f : range < 8 ? 1.f : 0.5f;
      oldRange = range;
    }
    int channels = std::max({ 1,
      inputs[CV_INPUT].getChannels(),
      inputs[ROOT_INPUT].getChannels(),
      inputs[IN_INPUT].getChannels()
    });
    bool inConnected = inputs[IN_INPUT].isConnected();
    int series = static_cast<int>(params[SERIES_PARAM].getValue());
    float scale=0.f, pfloat=0.f, root;
    int partial=0, pround=0;
    if (!inConnected) {
      scale = params[CV_PARAM].getValue() * 10.f;
      partial = partialParamGetValue();
    }
    float detune = params[DETUNE_AMT_PARAM].getValue();
    float comp = params[DETUNE_COMP_PARAM].getValue();
    for (int c=0; c<channels; c++){
      int ci = c/4;
      root = inputs[ROOT_INPUT].getPolyVoltage(c);
      if (inConnected) {
        bool inv = false;
        pfloat = inputs[IN_INPUT].getPolyVoltage(c) - root;
        if (pfloat < 0.f) {
          pfloat = -pfloat;
          inv = true;
        }
        PartialRec* rec;
        if (series == ODD) {
          rec = pfloat >= partials[112].oddMin ? &partials[112] :
                pfloat >= partials[96].oddMin ? &partials[96] :
                pfloat >= partials[80].oddMin ? &partials[80] :
                pfloat >= partials[64].oddMin ? &partials[64] :
                pfloat >= partials[48].oddMin ? &partials[48] :
                pfloat >= partials[32].oddMin ? &partials[32] :
                pfloat >= partials[16].oddMin ? &partials[16] :
                partials;
          while (pfloat > rec->oddMax) rec+=2;
        }
        else if (series == EVEN) {
          rec = pfloat >= partials[111].evenMin ? &partials[111] :
                pfloat >= partials[95].evenMin ? &partials[95] :
                pfloat >= partials[79].evenMin ? &partials[79] :
                pfloat >= partials[63].evenMin ? &partials[63] :
                pfloat >= partials[47].evenMin ? &partials[47] :
                pfloat >= partials[31].evenMin ? &partials[31] :
                pfloat >= partials[15].evenMin ? &partials[15] :
                partials;
          while (pfloat > rec->evenMax) rec+= rec==partials ? 1 : 2;
        }
        else { // series == ALL
          rec = pfloat >= partials[111].allMin ? &partials[111] :
                pfloat >= partials[95].allMin ? &partials[95] :
                pfloat >= partials[79].allMin ? &partials[79] :
                pfloat >= partials[63].allMin ? &partials[63] :
                pfloat >= partials[47].allMin ? &partials[47] :
                pfloat >= partials[31].allMin ? &partials[31] :
                pfloat >= partials[15].allMin ? &partials[15] :
                partials;
          while (pfloat > rec->allMax) rec++;
        }
        if (c == monitor)
          monitorVal = inv ? -(rec->partial-1) : rec->partial-1;
        out[ci][c%4] = root + (inv ? -rec->voct : rec->voct);
      } else {
        int mn=ranges[range][0], mx=ranges[range][1];
        if (series==ODD) {
          if (mn<0) mn+=1;
          if (mx>0) mx-=1;
        }
        pfloat = inputs[CV_INPUT].getPolyVoltage(c) * scale + partial;
        pround = math::clamp( static_cast<int>(round(pfloat)), mn, mx);
        if (series==ODD && (pround%2)!=0)
          pround += pfloat > pround ? 1 : -1;
        else if (series==EVEN && pround!=0 && pround%2==0)
          pround += pfloat > pround ? 1 : -1;
        if (c == monitor)
          monitorVal = pround;
        out[ci][c%4] = root + (pround>=0.f ? partials[static_cast<int>(pround)].voct : -partials[-static_cast<int>(pround)].voct);
      }
    }
    for (int c=0, ci=0; c<channels; c+=4, ci++){
      if (detune) {
        if (comp==2.f)
          out[ci] += detune / dsp::exp2_taylor5(simd::ifelse(out[ci]<-4.f, simd::float_4::zero(), out[ci]+4.f));
        else if (comp>1.f)
          out[ci] += detune / simd::pow(comp, simd::ifelse(out[ci]<-4.f, simd::float_4::zero(), out[ci]+4.f));
        else
          out[ci] += detune;
      }
      outputs[OUT_OUTPUT].setVoltageSimd(out[ci], c);
    }
    outputs[OUT_OUTPUT].setChannels(channels);
    if (monitor >= channels)
      monitorVal = MONITOR_OFF;
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "range", json_integer(range));
    json_object_set_new(rootJ, "monitor", json_integer(monitor));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "range")))
      range = json_integer_value(val);
    if((val = json_object_get(rootJ, "monitor")))
      monitor = json_integer_value(val);
  }

};


struct HQWidget : VenomWidget {

  struct PartialDisplay : DigitalDisplay188 {
    void step() override {
      if (module) {
        HQ* mod = dynamic_cast<HQ*>(module);
        if (mod->monitorVal == HQ::MONITOR_OFF) {
          text = "";
        } else {
          text = string::f("%d", abs(mod->monitorVal)+1);
          fgColor = mod->monitorVal < 0 ? SCHEME_RED : SCHEME_YELLOW;
        }
      } else {
        text = "1";
        fgColor = SCHEME_YELLOW;
      }
    }
  };

  HQWidget(HQ* module) {
    setModule(module);
    setVenomPanel("HQ");
    PartialDisplay* partialDisplay = createWidget<PartialDisplay>(Vec(4.976f, 43.217f));
    partialDisplay->module = module;
    addChild(partialDisplay);
    addParam(createLockableParam<CKSSThreeHorizontalLockable>(Vec(8.812f, 74.f), module, HQ::SERIES_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5f, 121.f), module, HQ::PARTIAL_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 159.f), module, HQ::CV_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 188.5f), module, HQ::CV_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(12.f, 229.f), module, HQ::DETUNE_AMT_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(33.f, 229.f), module, HQ::DETUNE_COMP_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 264.5f), module, HQ::ROOT_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 300.5f), module, HQ::IN_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 339.5f), module, HQ::OUT_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    HQ* module = dynamic_cast<HQ*>(this->module);
    assert(module);
    menu->addChild(new MenuSeparator);

    std::vector<std::string> rangeLabels;
    rangeLabels.push_back("1 to 16");
    rangeLabels.push_back("1 to 32");
    rangeLabels.push_back("1 to 64");
    rangeLabels.push_back("1 to 128");
    rangeLabels.push_back("-16 to 1");
    rangeLabels.push_back("-32 to 1");
    rangeLabels.push_back("-64 to 1");
    rangeLabels.push_back("-128 to 1");
    rangeLabels.push_back("-16 to 16");
    rangeLabels.push_back("-32 to 32");
    rangeLabels.push_back("-64 to 64");
    rangeLabels.push_back("-128 to 128");
    menu->addChild(createIndexSubmenuItem("Partial Range", rangeLabels,
      [=]() {return module->range;},
      [=](int i) {
        module->range = i;
      }
    ));

    std::vector<std::string> monitorLabels;
    for (int i=1; i<=16; i++)
      monitorLabels.push_back(std::to_string(i));
    monitorLabels.push_back("Off");
    menu->addChild(createIndexSubmenuItem("Monitor channel", monitorLabels,
      [=]() {return module->monitor;},
      [=](int i) {module->monitor = i;}
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

}

Model* modelVenomHQ = createModel<Venom::HQ, Venom::HQWidget>("HQ");

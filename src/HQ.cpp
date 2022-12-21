#include "plugin.hpp"
#include <float.h>


struct HQ : Module {
  enum ParamId {
    PARTIAL_PARAM,
    SERIES_PARAM,
    CV_PARAM,
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

  int partialParamGetValue() {
    int val = static_cast<int>(params[PARTIAL_PARAM].getValue());
    switch (static_cast<int>(params[SERIES_PARAM].getValue())) {
      case ALL: return val;
      case ODD: return val*2;
      case EVEN: 
      default:  return val>0 ? val*2-1 : val<0 ? val*2+1 : 0;
    }
  }

  int monitor = 0;
  int monitorVal = 0;
  enum monitorEnum {MONITOR_OFF=999};
  int oldSeries = 0;
  int range = 0;
  int ranges[12][2] = {
    {0,15}, {0,31}, {0,63}, {0,127},
    {-15,0}, {-31,0}, {-63,0}, {-127,0},
    {-15,15}, {-31,31}, {-63,63}, {-127,127}
  };

  struct PartialQuantity : ParamQuantity {
    float getDisplayValue() override {
      HQ* module = reinterpret_cast<HQ*>(this->module);
      int val = module->partialParamGetValue();
      return val + (val<0 ? -1 : 1);
    }
    void setDisplayValue(float v) override {
      HQ* module = reinterpret_cast<HQ*>(this->module);
      int val = static_cast<int>(v);
      switch (static_cast<int>(module->params[HQ::SERIES_PARAM].getValue())) {
        case HQ::ALL:
          val += val>0 ? -1 : val<0 ? 1 : 0;
          break;
        case HQ::ODD:
          val = val>0 ? (val-1)/2 : val<0 ? (val+1)/2 : 0;
          break;
        case EVEN:
          val = val>-2 && val<2 ? 0 : val/2;
          break;
      }
      setValue(static_cast<float>(val));
    }
  };

  void partialParamSetRange() {
    ParamQuantity* q = getParamQuantity(HQ::PARTIAL_PARAM);
    switch(static_cast<int>(params[SERIES_PARAM].getValue())){
      case ALL:
        q->minValue = ranges[range][0];
        q->maxValue = ranges[range][1];
        break;
      case ODD:
        q->minValue = ranges[range][0]/2;
        q->maxValue = ranges[range][1]/2;
        break;
      case EVEN:
        q->minValue = (ranges[range][0]+(ranges[range][0]<0 ? -1 : 1))/2;
        q->maxValue = (ranges[range][1]+1)/2;
        break;
    }
  }

  HQ() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch(SERIES_PARAM, 0, 2, 0, "Harmonic Series", {"All", "Odd", "Even"});
    configParam<PartialQuantity>(PARTIAL_PARAM, ranges[range][0], ranges[range][1], 0.f, "Partial", "");
    configParam(CV_PARAM, -1.f, 1.f, 0.f, "CV", "%", 0.f, 100.f, 0.f);
    configInput(CV_INPUT, "CV");
    configInput(ROOT_INPUT, "Root");
    configInput(IN_INPUT, "V/Oct");
    configOutput(OUT_OUTPUT, "V/Oct");
    configBypass(IN_INPUT, OUT_OUTPUT);
  }

  void process(const ProcessArgs& args) override {
    int channels = std::max({ 1,
      inputs[CV_INPUT].getChannels(),
      inputs[ROOT_INPUT].getChannels(),
      inputs[IN_INPUT].getChannels()
    });
    bool inConnected = inputs[IN_INPUT].isConnected();
    int series = static_cast<int>(params[SERIES_PARAM].getValue());
    if (series != oldSeries){
      partialParamSetRange();
      oldSeries = series;
    }
    float scale, pfloat=0.f, out=0.f, root;
    int partial, pround=0;
    if (!inConnected) {
      scale = params[CV_PARAM].getValue() * 10.f;
      partial = partialParamGetValue();
    }
    for (int c=0; c<channels; c++){
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
        out = root + (inv ? -rec->voct : rec->voct);
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
        out = root + (pround>=0.f ? partials[static_cast<int>(pround)].voct : -partials[-static_cast<int>(pround)].voct);
      }
      outputs[OUT_OUTPUT].setVoltage(out, c);
    }
    if (monitor >= channels)
      monitorVal = MONITOR_OFF;
    outputs[OUT_OUTPUT].setChannels(channels);
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "range", json_integer(range));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* val = json_object_get(rootJ, "range");
    if (val)
      range = json_integer_value(val);
    partialParamSetRange();
  }

};

struct PartialDisplay : DigitalDisplay188 {
  HQ* module;
  void step() override {
    if (module) {
      if (module->monitorVal == HQ::MONITOR_OFF) {
        text = "";
      } else {
        text = string::f("%d", abs(module->monitorVal)+1);
        fgColor = module->monitorVal < 0 ? SCHEME_RED : SCHEME_YELLOW;
      }
    } else {
      text = "1";
      fgColor = SCHEME_YELLOW;
    }
  }
};

struct HQWidget : ModuleWidget {
  HQWidget(HQ* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HQ.svg")));
    PartialDisplay* partialDisplay = createWidget<PartialDisplay>(mm2px(Vec(1.75, 14.0)));
    partialDisplay->module = module;
    addChild(partialDisplay);
    addParam(createParam<CKSSThreeHorizontal>(mm2px(Vec(2.9, 25.5)), module, HQ::SERIES_PARAM));
    addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(7.62, 42.9)), module, HQ::PARTIAL_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62, 58)), module, HQ::CV_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 68)), module, HQ::CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 83)), module, HQ::ROOT_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 98)), module, HQ::IN_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 113)), module, HQ::OUT_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    HQ* module = dynamic_cast<HQ*>(this->module);
    assert(module);
    menu->addChild(new MenuSeparator);

    std::vector<std::string> rangeLabels;
    rangeLabels.push_back("1 - 16");
    rangeLabels.push_back("1 - 32");
    rangeLabels.push_back("1 - 64");
    rangeLabels.push_back("1 - 128");
    rangeLabels.push_back("-16 - 1");
    rangeLabels.push_back("-32 - 1");
    rangeLabels.push_back("-64 - 1");
    rangeLabels.push_back("-128 - 1");
    rangeLabels.push_back("-16 - 16");
    rangeLabels.push_back("-32 - 32");
    rangeLabels.push_back("-64 - 64");
    rangeLabels.push_back("-128 - 128");
    menu->addChild(createIndexSubmenuItem("Partial Range", rangeLabels,
      [=]() {return module->range;},
      [=](int i) {
        module->range = i;
        module->partialParamSetRange();
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

  }

};

Model* modelHQ = createModel<HQ, HQWidget>("HQ");

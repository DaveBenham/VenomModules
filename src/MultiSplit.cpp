// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#define xInEven   14.5f
#define xInOdd  30.5f
#define xOutEven  59.5f
#define xOutOdd 75.5f
#define yOrigin  43.5f
#define yDelta   20.f

struct MultiSplit : VenomModule {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(POLY_INPUT,16),
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(POLY_OUTPUT,16),
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(DROP_LIGHT,16),
    LIGHTS_LEN
  };
  
  int outChannels[16]{}; // Assumes 1st DROP_LIGHT is 0
  std::string outLabels[17]{"Auto","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"};
  
  void setOutputDescription(int id){
    outputInfos[id]->description = "Channels: "+outLabels[outChannels[id]];
  }
  
  MultiSplit() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<16; i++) {
      std::string name = "Poly " + std::to_string(i+1);
      configInput(POLY_INPUT+i, name);
      configLight(DROP_LIGHT+i, name+" dropped channel(s) indicator");
      configOutput(POLY_OUTPUT+i, name);
      setOutputDescription(i);
    }  
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    int currentIn = 0;
    int nextIn = 0;
    bool overflow[16]{};
    while (nextIn < 16){
      currentIn = nextIn;
      nextIn = 16;
      int inCnt = std::max(inputs[POLY_INPUT+currentIn].getChannels(), 1);
      int fixedCnt = 0;
      int freeCnt = 0;
      for (int i=currentIn; i<16; i++) {
        if (outChannels[i])
          fixedCnt += outChannels[i];
        else
          freeCnt++;
        if (i==15 || inputs[POLY_INPUT+i+1].isConnected()){
          nextIn = i+1;
          break;
        }
      }
      int baseCnt = 0;
      int remainCnt = 0;
      if (freeCnt && inCnt>=fixedCnt){
        baseCnt = (inCnt-fixedCnt) / freeCnt;
        remainCnt = (inCnt-fixedCnt) % freeCnt;
      }
      int cIn=0;
      for (int i=currentIn; i<nextIn; i++) {
        int endCnt = outChannels[i];
        if (!endCnt){
          endCnt = baseCnt;
          if (remainCnt){
            endCnt++;
            remainCnt--;
          }
        }
        for (int cOut=0; cOut<endCnt; cOut++){
          outputs[POLY_OUTPUT+i].setVoltage(cIn<inCnt ? inputs[POLY_INPUT+currentIn].getVoltage(cIn++) : 0.f, cOut);
        }
        outputs[POLY_OUTPUT+i].setChannels(endCnt);
      }
      overflow[currentIn] = cIn < inCnt;
    }
    for (int i=0; i<16; i++)
      lights[DROP_LIGHT+i].setBrightness(overflow[i]);
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_t* array = json_array();
    for (int i=0; i<16; i++){
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

struct MultiSplitWidget : VenomWidget {

  struct Linework : widget::Widget {
    MultiSplit* mod=NULL;

    void drawLine(const DrawArgs& args, float x0, float y0, float x1, float y1) {
      nvgBeginPath(args.vg);
      nvgMoveTo(args.vg, x0, y0);
      nvgLineTo(args.vg, x1, y1);
      nvgStroke(args.vg);
    }

    void draw(const DrawArgs& args) override {
      int theme = mod ? mod->currentTheme : 0;
      if (!theme) theme = settings::preferDarkPanels ? getDefaultDarkTheme()+1 : getDefaultTheme()+1;
      nvgStrokeColor(args.vg, theme==4 ? nvgRGBA(0xf2, 0xf2, 0xf2, 0xff) : nvgRGBA(0xff, 0x00, 0x00, 0xff));
      nvgFillColor(args.vg, nvgRGB(0x25, 0x25, 0x25));
      for (int i=0; i<16; i++){
        if ((mod && mod->inputs[MultiSplit::POLY_OUTPUT+i].isConnected()) || (i==0)){
          nvgStrokeWidth(args.vg, 7.f);
          drawLine(args, i%2 ? xInOdd : xInEven, yOrigin+yDelta*i, i%2 ? xOutOdd : xOutEven, yOrigin+yDelta*i);
        } else {
          nvgStrokeWidth(args.vg, 20.f);
          drawLine(args, i%2 ? xOutEven : xOutOdd, yOrigin+yDelta*(i-1), i%2 ? xOutOdd : xOutEven, yOrigin+yDelta*i);
        }
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, i%2 ? xInOdd-18.5f : xInEven+18.5f, yOrigin+yDelta*i, 3.5f);
        nvgFill(args.vg);
      }
    }
  };
  
  struct OutPort : PolyPort {
    int portId;
    void appendContextMenu(Menu* menu) override {
      MultiSplit* module = static_cast<MultiSplit*>(this->module);
      menu->addChild(new MenuSeparator);
      menu->addChild(createIndexSubmenuItem(
        "Channels",
        {"Auto","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
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

  MultiSplitWidget(MultiSplit* module) {
    setModule(module);
    setVenomPanel("MultiSplit");

    Linework* linework = createWidget<Linework>(Vec(0.f,0.f));
    linework->mod = module;
    linework->box.size = box.size;
    addChild(linework);

    for (int i=0; i<16; i++){
      addInput(createInputCentered<PolyPort>(Vec(i%2 ? xInOdd : xInEven, yOrigin+yDelta*i), module, MultiSplit::POLY_INPUT+i));
      addChild(createLightCentered<SmallSimpleLight<RedLight>>(Vec(i%2 ? xInOdd-18.5f : xInEven+18.5f, yOrigin+yDelta*i), module, MultiSplit::DROP_LIGHT+i));
      addOutput(createConfigChannelsOutputCentered<OutPort>(Vec(i%2 ? xOutOdd : xOutEven, yOrigin+yDelta*i), module, MultiSplit::POLY_OUTPUT+i));
    }
  }

  void appendContextMenu(Menu* menu) override {
    MultiSplit* module = static_cast<MultiSplit*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Configure all output ports:"));
    menu->addChild(createIndexSubmenuItem(
      "Channels",
      {"Auto","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},
      [=]() {
        int current = module->outChannels[0];
        for (int i=1; i<16; i++){
          if (module->outChannels[i] != current)
            current = 17;
        }
        return current;
      },
      [=](int cnt) {
        if (cnt<16){
          for (int i=0; i<16; i++){
            module->outChannels[i] = cnt;
            module->setOutputDescription(i);
          }
        }
      }
    ));
    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelMultiSplit = createModel<MultiSplit, MultiSplitWidget>("MultiSplit");

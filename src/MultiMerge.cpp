// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#define xInEven   14.5f
#define xInOdd  30.5f
#define xOutEven  59.5f
#define xOutOdd 75.5f
#define yOrigin  43.5f
#define yDelta   20.f

struct MultiMerge : VenomModule {
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
  
  MultiMerge() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<16; i++) {
      std::string name = "Poly " + std::to_string(i+1);
      configInput(POLY_INPUT+i, name);
      configLight(DROP_LIGHT+i, name+" dropped channel(s) indicator");
      configOutput(POLY_OUTPUT+i, name);
    }  
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    int out[16]{};
    int outNdx = -1;
    int current = out[0];
    for (int i=15, o=15; i>=0; i--){
      out[i] = outputs[POLY_OUTPUT+i].isConnected() ? (o=i) : o;
    }
    for (int i=0; i<16; i++) {
      if (current != out[i]){
        outputs[current].setChannels(std::min(outNdx+1,16));
        current = out[i];
        outNdx = -1;
      }
      int inCnt=std::max(inputs[POLY_INPUT+i].getChannels(),1);
      for (int c=0; c<inCnt; c++){
        if (++outNdx<16)
          outputs[current].setVoltage(inputs[POLY_INPUT+i].getVoltage(c), outNdx);
        else
          break;
      }
      lights[DROP_LIGHT+i].setBrightness(outNdx>=16);
    }
    outputs[current].setChannels(std::min(outNdx+1,16));
  }

};

struct MultiMergeWidget : VenomWidget {
  
  struct Linework : widget::Widget {
    MultiMerge* mod=NULL;

    void drawLine(const DrawArgs& args, float x0, float y0, float x1, float y1) {
      nvgBeginPath(args.vg);
      nvgMoveTo(args.vg, x0, y0);
      nvgLineTo(args.vg, x1, y1);
      nvgStroke(args.vg);
    }

    void draw(const DrawArgs& args) override {
      int theme = mod ? mod->currentTheme : 0;
      if (!theme) theme = settings::preferDarkPanels ? getDefaultDarkTheme()+1 : getDefaultTheme()+1;
      nvgStrokeColor(args.vg, theme==4 ? nvgRGB(0xf2, 0xf2, 0xf2) : nvgRGB(0xff, 0x00, 0x00));
      nvgFillColor(args.vg, nvgRGB(0x25, 0x25, 0x25));
      bool chain=false;
      for (int i=0; i<16; i++){
        if (chain){
          nvgStrokeWidth(args.vg, 20.f);
          drawLine(args, i%2 ? xInEven : xInOdd, yOrigin+yDelta*(i-1), i%2 ? xInOdd : xInEven, yOrigin+yDelta*i);
        }
        if ((mod && mod->outputs[MultiMerge::POLY_OUTPUT+i].isConnected()) || (i==15)){
          nvgStrokeWidth(args.vg, 7.f);
          drawLine(args, i%2 ? xInOdd : xInEven, yOrigin+yDelta*i, i%2 ? xOutOdd : xOutEven, yOrigin+yDelta*i);
          chain = false;
        } else {
          chain = true;
        }
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, i%2 ? xInOdd-18.5f : xInEven+18.5f, yOrigin+yDelta*i, 3.5f);
        nvgFill(args.vg);
      }
    }
  };

  MultiMergeWidget(MultiMerge* module) {
    setModule(module);
    setVenomPanel("MultiMerge");

    Linework* linework = createWidget<Linework>(Vec(0.f,0.f));
    linework->mod = module;
    linework->box.size = box.size;
    addChild(linework);

    for (int i=0; i<16; i++){
      addInput(createInputCentered<PolyPort>(Vec(i%2 ? xInOdd : xInEven, yOrigin+yDelta*i), module, MultiMerge::POLY_INPUT+i));
      addChild(createLightCentered<SmallSimpleLight<RedLight>>(Vec(i%2 ? xInOdd-18.5f : xInEven+18.5f, yOrigin+yDelta*i), module, MultiMerge::DROP_LIGHT+i));
      addOutput(createOutputCentered<PolyPort>(Vec(i%2 ? xOutOdd : xOutEven, yOrigin+yDelta*i), module, MultiMerge::POLY_OUTPUT+i));
    }
  }

};

Model* modelMultiMerge = createModel<MultiMerge, MultiMergeWidget>("MultiMerge");

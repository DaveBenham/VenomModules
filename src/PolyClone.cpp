#include "plugin.hpp"
#include "ThemeStrings.hpp"

#define MODULE_NAME PolyClone
static const std::string moduleName = "PolyClone";

struct PolyClone : Module {
  enum ParamId {
    CLONE_PARAM,
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
    ENUMS(CHANNEL_LIGHTS, 32),
    LIGHTS_LEN
  };
  
  int clones = 1;

  #include "ThemeModVars.hpp"

  dsp::ClockDivider lightDivider;

  PolyClone() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(CLONE_PARAM, 1.f, 16.f, 1.f, "Clone count");
//    configInput(POLY_INPUT, "Poly");
//    configOutput(POLY_OUTPUT, "Poly");
    lightDivider.setDivision(44);
  }

  void process(const ProcessArgs& args) override {

    clones = static_cast<int>(params[CLONE_PARAM].getValue());
    int maxCh = 16/clones;

    // Get number of channels
    int ch = inputs[POLY_INPUT].getChannels();
    int goodCh = ch > maxCh ? maxCh : ch;

    int c=0;
    for (int i=0; i<goodCh; i++) {
      float val = inputs[POLY_INPUT].getVoltage(i);
      for (int j=0; j<clones; j++)
        outputs[POLY_OUTPUT].setVoltage(val, c++);
    }
    outputs[POLY_OUTPUT].setChannels(goodCh * clones);

    if (lightDivider.process()) {
      for (int i=0; i<16; i++) {
        lights[CHANNEL_LIGHTS+i*2].setBrightness(i<goodCh);
        lights[CHANNEL_LIGHTS+i*2+1].setBrightness(i>=goodCh && i<ch);
      }
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    #include "ThemeToJson.hpp"
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    #include "ThemeFromJson.hpp"
  }
};

struct CountDisplay : DigitalDisplay18 {
  PolyClone* module;
  void step() override {
    if (module) {
      text = string::f("%d", module->clones);
      fgColor = SCHEME_YELLOW;
    } else {
      text = "16";
      fgColor = SCHEME_YELLOW;
    }
  }
};

struct PolyCloneWidget : ModuleWidget {
  PolyCloneWidget(PolyClone* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, faceplatePath(moduleName, module ? module->currentThemeStr() : themes[getDefaultTheme()]))));

    float x = 22.5f;
    float y = RACK_GRID_WIDTH * 5.5f + 3;
    float dy = RACK_GRID_WIDTH * 2.f;

    CountDisplay* countDisplay = createWidget<CountDisplay>(Vec(x-12, y-20));
    countDisplay->module = module;
    addChild(countDisplay);

    y+=dy*1.f;
    addParam(createParamCentered<RotarySwitch<RoundBlackKnob>>(Vec(x,y), module, PolyClone::CLONE_PARAM));

    y += dy*4.75f;
    int ly = y+2;
    for(int li = 0; li < 8; li++, ly-=dy/2.f){
      addChild(createLightCentered<MediumLight<YellowRedLight<>>>(Vec(x-dy/5.f,ly), module, (PolyClone::CHANNEL_LIGHTS+li)*2));
      addChild(createLightCentered<MediumLight<YellowRedLight<>>>(Vec(x+dy/5.f,ly), module, (PolyClone::CHANNEL_LIGHTS+li+8)*2));
    }

    y+=dy;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, PolyClone::POLY_INPUT));

    y+=dy*1.75f;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, PolyClone::POLY_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    PolyClone* module = dynamic_cast<PolyClone*>(this->module);
    assert(module);
    #include "ThemeMenu.hpp"
  }

  #include "ThemeStep.hpp"
};

Model* modelPolyClone = createModel<PolyClone, PolyCloneWidget>("PolyClone");

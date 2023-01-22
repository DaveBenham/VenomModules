#include "plugin.hpp"
#include "ThemeStrings.hpp"

using simd::float_4;

#define MODULE_NAME CloneMerge
static const std::string moduleName = "CloneMerge";

struct CloneMerge : Module {
  enum ParamId {
    CLONE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(MONO_INPUTS, 8),
    INPUTS_LEN
  };
  enum OutputId {
    POLY_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(MONO_LIGHTS, 16),
    LIGHTS_LEN
  };

  #include "ThemeModVars.hpp"

  dsp::ClockDivider lightDivider;

  CloneMerge() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(CLONE_PARAM, 1.f, 16.f, 1.f, "Clone count");
    for (int i=0; i < 8; i++)
      configInput(MONO_INPUTS+i, string::f("Mono %d", i + 1));
    configOutput(POLY_OUTPUT, "Poly");
    lightDivider.setDivision(44);
  }

  void process(const ProcessArgs& args) override {

    int clones = static_cast<int>(params[CLONE_PARAM].getValue());
    int maxIns = 16/clones;

    // Get number of inputs
    int ins = 1;
    for (int i=7; i>0; i--) {
      if (inputs[MONO_INPUTS+i].isConnected()) {
        ins = i+1;
        break;
      }
    }
    int goodIns = ins > maxIns ? maxIns : ins;

    int channel=0;
    for (int i=0; i<goodIns; i+=1) {
      float val = inputs[MONO_INPUTS + i].getVoltage();
      for (int c=0; c<clones; c++)
        outputs[POLY_OUTPUT].setVoltage(val, channel++);
    }
    outputs[POLY_OUTPUT].setChannels(channel);
    
    if (lightDivider.process()) {
      for (int i=0; i<8; i++) {
        lights[MONO_LIGHTS+i*2].setBrightness(i<goodIns);
        lights[MONO_LIGHTS+i*2+1].setBrightness(i>=goodIns && i<ins);
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

struct CloneMergeWidget : ModuleWidget {
  CloneMergeWidget(CloneMerge* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, faceplatePath(moduleName, module ? module->currentThemeStr() : themes[getDefaultTheme()]))));

    float x = 22.5;
    float y = RACK_GRID_WIDTH * 3.55f;
    float dy = RACK_GRID_WIDTH * 2.f;

    addParam(createParamCentered<RotarySwitch<RoundSmallBlackKnob>>(Vec(x,y), module, CloneMerge::CLONE_PARAM));

    y+=dy*1.25f;
    for (int i=0; i<8; i++, y+=dy) {
      addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, CloneMerge::MONO_INPUTS + i));
      addChild(createLightCentered<TinyLight<YellowRedLight<>>>(Vec(x+dy*0.35f, y-dy*0.35f), module, CloneMerge::MONO_LIGHTS + i*2));
    }
    y+=dy*0.33f;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, CloneMerge::POLY_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    CloneMerge* module = dynamic_cast<CloneMerge*>(this->module);
    assert(module);
    #include "ThemeMenu.hpp"
  }

  #include "ThemeStep.hpp"
};

Model* modelCloneMerge = createModel<CloneMerge, CloneMergeWidget>("CloneMerge");

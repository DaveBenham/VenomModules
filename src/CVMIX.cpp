#include "plugin.hpp"
#include "ThemeStrings.hpp"

using simd::float_4;

#define MODULE_NAME CVMix
static const std::string moduleName = "CVMix";

struct CVMix : Module {
  enum ParamId {
    ENUMS(LEVEL_PARAMS, 3),
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(CV_INPUTS, 3),
    INPUTS_LEN
  };
  enum OutputId {
    MIX_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  #include "ThemeModVars.hpp"

  dsp::ClockDivider paramDivider;

  CVMix() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i = 0; i < 3; i++)
      configParam(LEVEL_PARAMS + i, -1.f, 1.f, 0.f, string::f("Level %d", i + 1), "V", 0, 10);
    for (int i = 0; i < 3; i++)
      configInput(CV_INPUTS + i, string::f("CV %d", i + 1));
    configOutput(MIX_OUTPUT, "Mix");
    paramDivider.setDivision(2048);
  }

  void process(const ProcessArgs& args) override {

    if (paramDivider.process())
      refreshParamQuantities();

    if (!outputs[MIX_OUTPUT].isConnected())
      return;

    // Get number of channels
    int channels = 1;
    for (int i = 0; i < 3; i++)
      channels = std::max(channels, inputs[CV_INPUTS + i].getChannels());

    for (int c = 0; c < channels; c += 4) {
      // Sum CV inputs
      float_4 mix = 0.f;
      for (int i = 0; i < 3; i++) {
        // Normalize inputs to 10V
        float_4 cv = inputs[CV_INPUTS + i].getNormalPolyVoltageSimd<float_4>(10.f, c);

        // Apply gain
        cv *= params[LEVEL_PARAMS + i].getValue();
        mix += cv;
      }

      // Set mix output
      outputs[MIX_OUTPUT].setVoltageSimd(mix, c);
    }
    outputs[MIX_OUTPUT].setChannels(channels);
  }

  /** Set the level param units to either V or %, depending on whether a cable is connected. */
  void refreshParamQuantities() {
    for (int i = 0; i < 3; i++) {
      ParamQuantity* pq = paramQuantities[LEVEL_PARAMS + i];
      if (!pq)
        continue;
      if (inputs[CV_INPUTS + i].isConnected()) {
        pq->unit = "%";
        pq->displayMultiplier = 100.f;
      }
      else {
        pq->unit = "V";
        pq->displayMultiplier = 10.f;
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

struct CVMixWidget : ModuleWidget {
  CVMixWidget(CVMix* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, faceplatePath(moduleName, module ? module->currentThemeStr() : themes[getDefaultTheme()]))));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 24.723)), module, CVMix::LEVEL_PARAMS + 0));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 41.327)), module, CVMix::LEVEL_PARAMS + 1));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 57.922)), module, CVMix::LEVEL_PARAMS + 2));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 76.539)), module, CVMix::CV_INPUTS + 0));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 86.699)), module, CVMix::CV_INPUTS + 1));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 96.859)), module, CVMix::CV_INPUTS + 2));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 113.115)), module, CVMix::MIX_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    CVMix* module = dynamic_cast<CVMix*>(this->module);
    assert(module);
    #include "ThemeMenu.hpp"
  }

  #include "ThemeStep.hpp"
};

Model* modelCVMix = createModel<CVMix, CVMixWidget>("CVMix");

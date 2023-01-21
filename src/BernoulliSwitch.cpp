#include "plugin.hpp"
#include "ThemeStrings.hpp"

#define LIGHT_OFF 0.02f

#define MODULE_NAME BernoulliSwitch
static const std::string moduleName = "BernoulliSwitch";


struct BernoulliSwitch : Module {
  enum ParamId {
    PROB_PARAM,
    TRIG_PARAM,
    MODE_PARAM,
    RISE_PARAM,
    FALL_PARAM,
    OFFSET_A_PARAM,
    OFFSET_B_PARAM,
    SCALE_A_PARAM,
    SCALE_B_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    A_INPUT,
    B_INPUT,
    TRIG_INPUT,
    PROB_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    A_OUTPUT,
    B_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    NO_SWAP_LIGHT,
    SWAP_LIGHT,
    TRIG_LIGHT,
    LIGHTS_LEN
  };
  enum ProbMode {
    TOGGLE_MODE,
    SWAP_MODE,
    GATE_MODE
  };

  dsp::SchmittTrigger trig[PORT_MAX_CHANNELS];
  bool swap[PORT_MAX_CHANNELS];
  int oldChannels = 0;
  int lightChannel = 0;
  bool lightOff = false;
  bool inputPolyControl = false;

  #include "ThemeModVars.hpp"

  BernoulliSwitch() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(PROB_PARAM, 0.f, 1.f, 0.5f, "Probability", "%", 0.f, 100.f, 0.f);
    configButton(TRIG_PARAM, "Manual 10V Trigger");
    configSwitch(MODE_PARAM, 0, 2, 1, "Probability Mode", {"Toggle", "Swap", "Gate"});
    configParam(RISE_PARAM, -10.f, 10.f, 1.f, "Rise Threshold", " V");
    configParam(FALL_PARAM, -10.f, 10.f, 0.1f, "Fall Threshold", " V");
    configParam(OFFSET_A_PARAM, -10.f, 10.f, 0.f, "A Offset", " V");
    configParam(OFFSET_B_PARAM, -10.f, 10.f, 0.f, "B Offset", " V");
    configParam(SCALE_A_PARAM, -1.f, 1.f, 1.f, "A Scale", "");
    configParam(SCALE_B_PARAM, -1.f, 1.f, 1.f, "B Scale", "");
    configInput(A_INPUT, "A");
    configInput(B_INPUT, "B");
    configInput(TRIG_INPUT, "Trigger");
    configInput(PROB_INPUT, "Probability");
    configOutput(A_OUTPUT, "A");
    configOutput(B_OUTPUT, "B");
    lights[NO_SWAP_LIGHT].setBrightness(true);
    lights[SWAP_LIGHT].setBrightness(false);
  }

  void onReset() override {
    oldChannels = 0;
    lights[NO_SWAP_LIGHT].setBrightness(true);
    lights[SWAP_LIGHT].setBrightness(false);
  }

  void process(const ProcessArgs& args) override {
    float scaleA = params[SCALE_A_PARAM].getValue(),
          scaleB = params[SCALE_B_PARAM].getValue(),
          offA = params[OFFSET_A_PARAM].getValue(),
          offB = params[OFFSET_B_PARAM].getValue(),
          rise = params[RISE_PARAM].getValue(),
          fall = params[FALL_PARAM].getValue(),
          probOff = params[PROB_PARAM].getValue(),
          manual = params[TRIG_PARAM].getValue() > 0.f ? 10.f : 0.f;
    bool invTrig = rise < fall;
    int aChannels = inputs[A_INPUT].getChannels();
    int bChannels = inputs[B_INPUT].getChannels();
    int mode = static_cast<int>(params[MODE_PARAM].getValue());
    if (invTrig) {
      rise = -rise;
      fall = -fall;
    }
    int channels = inputPolyControl ?
      std::max({ 1, aChannels, bChannels,
        inputs[TRIG_INPUT].getChannels(), inputs[PROB_INPUT].getChannels()
      }) :
      std::max({ 1,
        inputs[TRIG_INPUT].getChannels(), inputs[PROB_INPUT].getChannels()
      });
    if (channels > oldChannels) {
      for (int c=oldChannels; c<channels; c++){
        trig[c].reset();
        swap[c] = false;
      }
      oldChannels = channels;
    }
    if (!lightOff && lightChannel >= channels) {
      lights[NO_SWAP_LIGHT].setBrightness(false);
      lights[SWAP_LIGHT].setBrightness(false);
      lightOff = true;
    }
    if (lightOff && lightChannel < channels) {
      lights[NO_SWAP_LIGHT].setBrightness(!swap[lightChannel]);
      lights[SWAP_LIGHT].setBrightness(swap[lightChannel]);
      lightOff = false;
    }
    for (int c=0; c<channels; c++){
      float prob = inputs[PROB_INPUT].getPolyVoltage(c)/10.f + probOff;
      float trigIn = inputs[TRIG_INPUT].getPolyVoltage(c) + manual;
      if (trig[c].process(invTrig ? -trigIn : trigIn, fall, rise)){
        bool toss = (prob == 1.0f || random::uniform() < prob);
        switch(mode) {
          case TOGGLE_MODE:
            if (toss) swap[c] = !swap[c];
            break;
          case SWAP_MODE:
            swap[c] = toss;
            break;
          case GATE_MODE:
            swap[c] = !toss;
            break;
        }
        if (c == lightChannel) {
          lights[NO_SWAP_LIGHT].setBrightness(!swap[c]);
          lights[SWAP_LIGHT].setBrightness(swap[c]);
        }
      }
      if (mode == GATE_MODE && !swap[c] && !trig[c].isHigh()) {
        swap[c] = true;
        if (c == lightChannel){
          lights[NO_SWAP_LIGHT].setBrightness(false);
          lights[SWAP_LIGHT].setBrightness(true);
        }
      }
      lights[TRIG_LIGHT].setBrightness(manual ? 1.f : LIGHT_OFF);
      if (channels == 1 && !inputPolyControl) {
        for (int i=0; i<aChannels; i++)
          outputs[swap[c] ? B_OUTPUT : A_OUTPUT].setVoltage(inputs[A_INPUT].getNormalPolyVoltage(trigIn, i) * scaleA + offA, i);
        for (int i=0; i<bChannels; i++)
          outputs[swap[c] ? A_OUTPUT : B_OUTPUT].setVoltage(inputs[B_INPUT].getPolyVoltage(i) * scaleB + offB, i);
      } else {
        outputs[swap[c] ? B_OUTPUT : A_OUTPUT].setVoltage(inputs[A_INPUT].getNormalPolyVoltage(trigIn, c) * scaleA + offA, c);
        outputs[swap[c] ? A_OUTPUT : B_OUTPUT].setVoltage(inputs[B_INPUT].getPolyVoltage(c) * scaleB + offB, c);
      }
    }
    outputs[A_OUTPUT].setChannels( (channels==1 && !inputPolyControl) ? (swap[0] ? bChannels : aChannels) : channels);
    outputs[B_OUTPUT].setChannels( (channels==1 && !inputPolyControl) ? (swap[0] ? aChannels : bChannels) : channels);
  }


  json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "monitorChannel", json_integer(lightChannel));
    json_object_set_new(rootJ, "inputPolyControl", json_boolean(inputPolyControl));
    #include "ThemeToJson.hpp"
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    json_t* val;
    if ((val = json_object_get(rootJ, "monitorChannel")))
      lightChannel = json_integer_value(val);
    if ((val = json_object_get(rootJ, "inputPolyControl")))
      inputPolyControl = json_boolean_value(val);
    #include "ThemeFromJson.hpp"
  }

};

struct BernoulliSwitchWidget : ModuleWidget {
  BernoulliSwitchWidget(BernoulliSwitch* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, faceplatePath(moduleName, module ? module->currentThemeStr() : themes[getDefaultTheme()]))));

    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(mm2px(Vec(5.0, 18.75)), module, BernoulliSwitch::NO_SWAP_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(mm2px(Vec(20.431, 18.75)), module, BernoulliSwitch::SWAP_LIGHT));

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12.7155, 18.75)), module, BernoulliSwitch::PROB_PARAM));
    addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(6.5, 31.5)), module, BernoulliSwitch::TRIG_PARAM, BernoulliSwitch::TRIG_LIGHT));
    addParam(createParam<CKSSThree>(mm2px(Vec(17.5, 25.0)), module, BernoulliSwitch::MODE_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.297, 43.87)), module, BernoulliSwitch::RISE_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.134, 43.87)), module, BernoulliSwitch::FALL_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.297, 58.3)), module, BernoulliSwitch::OFFSET_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.136, 58.3)), module, BernoulliSwitch::OFFSET_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.297, 72.75)), module, BernoulliSwitch::SCALE_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.136, 72.75)), module, BernoulliSwitch::SCALE_B_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.297, 87.10)), module, BernoulliSwitch::A_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.134, 87.10)), module, BernoulliSwitch::B_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.297, 101.55)), module, BernoulliSwitch::A_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.134, 101.55)), module, BernoulliSwitch::B_OUTPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.297, 116.0)), module, BernoulliSwitch::TRIG_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.134, 116.0)), module, BernoulliSwitch::PROB_INPUT));
  }

  void appendContextMenu(Menu* menu) override {
    BernoulliSwitch* module = dynamic_cast<BernoulliSwitch*>(this->module);
    assert(module);
    menu->addChild(new MenuSeparator);
    std::vector<std::string> lightChannelLabels;
    for (int i=1; i<=16; i++)
      lightChannelLabels.push_back(std::to_string(i));
    lightChannelLabels.push_back("Off");
    menu->addChild(createIndexPtrSubmenuItem(
      "Polyphony control",
      {"Trig and Prob only", "All inputs"},
      &module->inputPolyControl
    ));
    menu->addChild(createIndexSubmenuItem("Monitor channel", lightChannelLabels,
      [=]() {return module->lightChannel;},
      [=](int i) {
        module->lightChannel = i;
        module->lights[BernoulliSwitch::NO_SWAP_LIGHT].setBrightness(i > module->oldChannels ? false : !module->swap[i]);
        module->lights[BernoulliSwitch::SWAP_LIGHT].setBrightness(i > module->oldChannels ? false : module->swap[i]);
      }
    ));
    #include "ThemeMenu.hpp"
  }

  #include "ThemeStep.hpp"
};

Model* modelBernoulliSwitch = createModel<BernoulliSwitch, BernoulliSwitchWidget>("BernoulliSwitch");

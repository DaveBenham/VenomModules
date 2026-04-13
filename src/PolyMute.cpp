// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

#define LIGHT_OFF 0.02f
#define LIGHT_INACTIVE 0.f

namespace Venom {

struct PolyMute : VenomModule {

  enum ParamId {
    ENUMS(MUTE_PARAM,16),
    SOFT_PARAM,
    MODE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    GATES_INPUT,
    POLY1_INPUT,
    POLY2_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    POLY1_OUTPUT,
    POLY2_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(MUTE_LIGHT,32),
    LIGHTS_LEN
  };
  
  bool buttonVal[16]{},
       cvVal[16]{},
       state[16]{true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true};
       
  #define BLOCK 0
  #define PASS 1
  #define TOGGLE 2

  int channelsParam = 0,
      channels = 0;
  
  dsp::SlewLimiter fade[16];
  
  struct GateQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
      return static_cast<PolyMute*>(this->module)->state[paramId] ? "Pass" : "Mute";
    }
  };

  PolyMute() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    
    for (int i=0; i<16; i++) {
      std::string nm = "Channel " + std::to_string(i+1);
      configParam<GateQuantity>(MUTE_PARAM+i, 0.f, 1.f, 0.f, nm);
      fade[i].setRiseFall(100.f, 100.f);
    }
    configSwitch<FixedSwitchQuantity>(SOFT_PARAM, 0.f, 1.f, 0.f, "Soft switching", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 2.f, 0.f, "Gate mode", {"Mute", "Pass", "Toggle"});
    configInput(GATES_INPUT, "Channel gates");
    configInput(POLY1_INPUT, "Poly 1");
    configInput(POLY2_INPUT, "Poly 2");
    configOutput(POLY1_OUTPUT, "Poly 1");
    configOutput(POLY2_OUTPUT, "Poly 2");
    configBypass(POLY1_INPUT, POLY1_OUTPUT);
    configBypass(POLY2_INPUT, POLY2_OUTPUT);
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    channels = channelsParam;
    if (!channels) {
      if (inputs[POLY1_INPUT].getChannels() || inputs[POLY2_INPUT].getChannels())
        channels = static_cast<int>(std::max(inputs[POLY1_INPUT].getChannels(), inputs[POLY2_INPUT].getChannels()));
      else
        channels = 16;
    }
    int mode = params[MODE_PARAM].getValue();
    bool soft = params[SOFT_PARAM].getValue();
    for (int i=0; i<16; i++) {
      bool newButtonVal = params[MUTE_PARAM+i].getValue();
      if (inputs[GATES_INPUT].isConnected()) {
        bool newCvVal = cvVal[i];
        if (inputs[GATES_INPUT].getPolyVoltage(i)>=2.0f)
          newCvVal = true;
        if (inputs[GATES_INPUT].getPolyVoltage(i)<=0.2f)
          newCvVal = false;
        switch (mode) {
          case BLOCK:
            state[i] = newButtonVal ? newCvVal : !newCvVal;
            break;
          case PASS:
            state[i] = newButtonVal ? !newCvVal : newCvVal;
            break;
          case TOGGLE:
            if (newCvVal != cvVal[i] && newCvVal)
              state[i] = !state[i];
            if (newButtonVal != buttonVal[i] && newButtonVal)
              state[i] = !state[i];
            break;
        }
        cvVal[i] = newCvVal;
      } else {
        cvVal[i] = false;
        if (newButtonVal != buttonVal[i] && newButtonVal)
          state[i] = !state[i];
      }
      if (soft && i<channels)
        fade[i].process(args.sampleTime, state[i]);
      else if (i<channels)
        fade[i].out = state[i];
      else
        fade[i].out = 0.f;
      outputs[POLY1_OUTPUT].setVoltage( inputs[POLY1_INPUT].getNormalPolyVoltage(10.f, i)*fade[i].out, i);
      outputs[POLY2_OUTPUT].setVoltage( inputs[POLY2_INPUT].getNormalPolyVoltage(10.f, i)*fade[i].out, i);
      buttonVal[i] = newButtonVal;
    }
    outputs[POLY1_OUTPUT].setChannels(channels);
    outputs[POLY2_OUTPUT].setChannels(channels);
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "channelsParam", json_integer(channelsParam));
    json_t* jArray = json_array();
    for (int i=0; i<16; i++)
      json_array_append_new(jArray, json_boolean(state[i]));
    json_object_set_new(rootJ, "state", jArray);
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t *arrayVal, *val;
    size_t i;
    if ((arrayVal = json_object_get(rootJ, "state"))){
      json_array_foreach(arrayVal, i, val) {
        state[i] = json_boolean_value(val);
      }
    }
  }

};

struct PolyMuteWidget : VenomWidget {

  struct SoftSwitch : GlowingSvgSwitchLockable {
    SoftSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
    }
  };

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
    }
  };

  PolyMuteWidget(PolyMute* module) {
    setModule(module);
    setVenomPanel("PolyMute");

    for (int i=0; i<8; i++) {
      addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteRedLight<>>>>(Vec(20.5f, 44.5f+i*27.f), module, PolyMute::MUTE_PARAM+i, PolyMute::MUTE_LIGHT+i*2));
      addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteRedLight<>>>>(Vec(54.5f, 44.5f+i*27.f), module, PolyMute::MUTE_PARAM+i+8, PolyMute::MUTE_LIGHT+(i+8)*2));
    }
    addParam(createLockableParamCentered<SoftSwitch>(Vec(12.5f, 269.5f), module, PolyMute::SOFT_PARAM));
    addParam(createLockableParamCentered<ModeSwitch>(Vec(62.5f, 269.5f), module, PolyMute::MODE_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(37.5f, 269.5f), module, PolyMute::GATES_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(20.5f, 303.5f), module, PolyMute::POLY1_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(54.5f, 303.5f), module, PolyMute::POLY2_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(20.5f, 341.f), module, PolyMute::POLY1_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(54.5f, 341.f), module, PolyMute::POLY2_OUTPUT));
  }
  
  void appendContextMenu(Menu *menu) override {
    PolyMute *module = static_cast<PolyMute*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexPtrSubmenuItem("Polyphony channel count", {"Auto","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"}, &module->channelsParam));
    VenomWidget::appendContextMenu(menu);
  }  

  void step() override {
    VenomWidget::step();
    PolyMute* module = dynamic_cast<PolyMute*>(this->module);
    if(module) {
      for (int i=0; i<16; i++){
        module->lights[i*2+0].setBrightness(module->state[i] ? (i<module->channels ? 1.f : 0.02f) : 0.f);
        module->lights[i*2+1].setBrightness(!module->state[i] ? (i<module->channels ? 1.f : 0.02f) : 0.f);
      }
    }
  }
  
};

}

Model* modelVenomPolyMute = createModel<Venom::PolyMute, Venom::PolyMuteWidget>("PolyMute");

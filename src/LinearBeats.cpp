// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

struct LinearBeats : VenomModule {
  #include "LinearMute.hpp"

  enum ParamId {
    ENUMS(MODE_PARAM,10),
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(IN_INPUT,10),
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(OUT_OUTPUT,10),
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  LinearBeats() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<10; i++) {
      configInput(IN_INPUT+i, label[i]);
      configSwitch<FixedSwitchQuantity>(MODE_PARAM+i, 0.f, 3.f, 0.f, label[i]+" Mode", {"Linear", "All", "Non-blocking Linear", "New All"});
      configOutput(OUT_OUTPUT+i, label[i]);
      configBypass(IN_INPUT+i, OUT_OUTPUT+i);
    }  
  }

  struct Channel : dsp::TSchmittTrigger<> {
    bool outState = false;
    float proc(float inValue, bool& preState, int mode, bool inMute, bool outMute) {
      if (process(inValue, 0.1f, 1.f)){
        outState = mode==0 || mode==2 ? !preState && state : state;
      }
      if (inMute)
        outState = false;
      else
        preState = mode==2 ? preState : mode==3 ? state : preState || state;
      return outState && !outMute ? inValue : 0.f;
    }  
  };
  
  Channel channel[10][16];
  int oldCnt[10] = {0,0,0,0,0,0,0,0,0,0};

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    
    bool preState = false;
    for(int i=0; i<10; i++){
      int cnt = inputs[IN_INPUT+i].getChannels();
      for (int c=oldCnt[i]; c<cnt; c++)
        channel[i][c].outState = channel[i][c].state = false;
      for(int c=0; c<cnt; c++){
        outputs[OUT_OUTPUT+i].setVoltage(channel[i][c].proc(inputs[IN_INPUT+i].getVoltage(c), preState, params[MODE_PARAM+i].getValue(), false, false), c);
      }
      outputs[OUT_OUTPUT+i].setChannels(cnt);
      oldCnt[i] = cnt;
    }
  }
  
};

struct LinearBeatsWidget : VenomWidget {

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
    }
  };

  LinearBeatsWidget(LinearBeats* module) {
    setModule(module);
    setVenomPanel("LinearBeats");
    float y=56.5f;
    for(int i=0; i<10; i++){
      addInput(createInputCentered<PJ301MPort>(Vec(16.5f,y), module, LinearBeats::IN_INPUT+i));
      addParam(createLockableParamCentered<ModeSwitch>(Vec(37.5f,y), module, LinearBeats::MODE_PARAM+i));
      addOutput(createOutputCentered<PJ301MPort>(Vec(58.5f,y), module, LinearBeats::OUT_OUTPUT+i));
      y+=31.556f;
    }
  }

};

Model* modelLinearBeats = createModel<LinearBeats, LinearBeatsWidget>("LinearBeats");

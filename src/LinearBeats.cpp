// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

struct LinearBeats : VenomModule {
  #include "LinearMute.hpp"

  enum ParamId {
    ENUMS(MODE_PARAM,9),
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(IN_INPUT,9),
    CLOCK_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(OUT_OUTPUT,9),
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  Module* inMute = NULL;
  Module* outMute = NULL;

  LinearBeats() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<9; i++) {
      configInput(IN_INPUT+i, label[i]);
      configSwitch<FixedSwitchQuantity>(MODE_PARAM+i, 0.f, 3.f, 0.f, label[i]+" Mode", {"Linear", "All", "Non-blocking Linear", "New All"});
      configOutput(OUT_OUTPUT+i, label[i]);
      configBypass(IN_INPUT+i, OUT_OUTPUT+i);
    }  
    configInput(CLOCK_INPUT, "Clock");
  }
  
  dsp::SchmittTrigger clockTrigger;

  struct Channel : dsp::TSchmittTrigger<> {
    bool outState = false;
    float proc(bool trig, float inValue, bool& preState, int mode, bool inMute, bool outMute) {
      Event event;
      if (trig && (event = processEvent(inValue, 0.1f, 1.f))){
        if (TRIGGERED)
          outState = mode==0 || mode==2 ? !preState && state : state;
        else
          outState = false;
      }
      if (inMute)
        outState = false;
      else
        preState = mode==2 ? preState : mode==3 ? state : preState || state;
      return outState && !outMute ? 10.f : 0.f;
    }  
  };
  
  Channel channel[9][16];
  int oldCnt[9] = {0,0,0,0,0,0,0,0,0};

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    
    bool preState = false;
    bool trig = (!inputs[CLOCK_INPUT].isConnected()) || clockTrigger.process(inputs[CLOCK_INPUT].getVoltage(), 0.1f, 1.f);
    Module* finalOutMute = outMute && (!outMute->getRightExpander().module || outMute->getRightExpander().module->model != modelLinearBeats) ? outMute : NULL;
    for(int i=0; i<9; i++){
      int cnt = inputs[IN_INPUT+i].getChannels();
      for (int c=oldCnt[i]; c<cnt; c++)
        channel[i][c].outState = channel[i][c].state = false;
      for(int c=0; c<cnt; c++){
        outputs[OUT_OUTPUT+i].setVoltage(
          channel[i][c].proc(
            trig, 
            inputs[IN_INPUT+i].getVoltage(c), 
            preState,
            params[MODE_PARAM+i].getValue(), 
            inMute ? inMute->params[MUTE_PARAM+i].getValue() : false, 
            finalOutMute ? finalOutMute->params[MUTE_PARAM+i].getValue() : false
          ), 
          c
        );
      }
      outputs[OUT_OUTPUT+i].setChannels(cnt);
      oldCnt[i] = cnt;
    }
  }
  
  void onExpanderChange(const ExpanderChangeEvent& e) override {
    if (e.side) { // right
      outMute = getRightExpander().module;
      if (outMute && outMute->model != modelLinearMute)
        outMute = NULL;
    }
    else { // left
      inMute = getLeftExpander().module;
      if ( inMute && inMute->model != modelLinearMute)
        inMute = NULL;
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
    for(int i=0; i<9; i++){
      addInput(createInputCentered<PJ301MPort>(Vec(16.5f,y), module, LinearBeats::IN_INPUT+i));
      addParam(createLockableParamCentered<ModeSwitch>(Vec(37.5f,y), module, LinearBeats::MODE_PARAM+i));
      addOutput(createOutputCentered<PJ301MPort>(Vec(58.5f,y), module, LinearBeats::OUT_OUTPUT+i));
      y+=31.556f;
    }
    addInput(createInputCentered<PJ301MPort>(Vec(16.5f,y), module, LinearBeats::CLOCK_INPUT));
  }

};

Model* modelLinearBeats = createModel<LinearBeats, LinearBeatsWidget>("LinearBeats");

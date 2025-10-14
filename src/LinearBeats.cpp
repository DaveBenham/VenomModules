// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

struct LinearBeats : VenomModule {
  #include "LinearBeatsExpander.hpp"

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
  bool toggleCV = false;
  dsp::SchmittTrigger inMuteCV[9], outMuteCV[9], inDisableCV, outDisableCV;

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
        if (event == TRIGGERED) {
          outState = (mode==1 || mode==3 || !preState) && !inMute;
          preState = mode==2 ? preState : mode==3 ? outState : preState || outState;
        }
        else
          outState = false;
      }
      return outState && !outMute ? 10.f : 0.f;
    }  
  };
  
  Channel channel[9][16];
  int oldCnt[9] = {0,0,0,0,0,0,0,0,0};

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    bool preState = false;
    bool trig = (!inputs[CLOCK_INPUT].isConnected()) || clockTrigger.process(inputs[CLOCK_INPUT].getVoltage(), 0.1f, 1.f);
    Module* finalInMute = inMute && !inMute->isBypassed() ? inMute : NULL;
    Module* finalOutMute = outMute && !outMute->isBypassed() && (!outMute->getRightExpander().module || outMute->getRightExpander().module->model != modelVenomLinearBeats) ? outMute : NULL;
    if (finalInMute) {
      for (int i=0; i<9; i++) {
        int evnt = inMuteCV[i].processEvent(finalInMute->inputs[MUTE_INPUT+i].getVoltage(), 0.1f, 1.f);
        if (toggleCV && evnt>0)
          finalInMute->params[MUTE_PARAM+i].setValue(!finalInMute->params[MUTE_PARAM+i].getValue());
        if (!toggleCV && evnt)
          finalInMute->params[MUTE_PARAM+i].setValue(inMuteCV[i].isHigh());
      }
      int evnt = inDisableCV.processEvent(finalInMute->inputs[BYPASS_INPUT].getVoltage(), 0.1f, 1.f);
      if (toggleCV && evnt>0)
        finalInMute->params[BYPASS_PARAM].setValue(!finalInMute->params[BYPASS_PARAM].getValue());
      if (!toggleCV && evnt)
        finalInMute->params[BYPASS_PARAM].setValue(inDisableCV.isHigh());
    }  
    if (finalOutMute) {
      for (int i=0; i<9; i++) {
        int evnt = outMuteCV[i].processEvent(finalOutMute->inputs[MUTE_INPUT+i].getVoltage(), 0.1f, 1.f);
        if (toggleCV && evnt>0)
          finalOutMute->params[MUTE_PARAM+i].setValue(!finalOutMute->params[MUTE_PARAM+i].getValue());
        if (!toggleCV && evnt)
          finalOutMute->params[MUTE_PARAM+i].setValue(outMuteCV[i].isHigh());
      }
      int evnt = outDisableCV.processEvent(finalOutMute->inputs[BYPASS_INPUT].getVoltage(), 0.1f, 1.f);
      if (toggleCV && evnt>0)
        finalOutMute->params[BYPASS_PARAM].setValue(!finalOutMute->params[BYPASS_PARAM].getValue());
      if (!toggleCV && evnt)
        finalOutMute->params[BYPASS_PARAM].setValue(outDisableCV.isHigh());
    }
    if ((finalInMute && finalInMute->params[BYPASS_PARAM].getValue()) || (finalOutMute && finalOutMute->params[BYPASS_PARAM].getValue())) {
      for (int i=0; i<9; i++) {
        if ((finalInMute && finalInMute->params[MUTE_PARAM+i].getValue()) || (finalOutMute && finalOutMute->params[MUTE_PARAM+i].getValue())) {
          outputs[OUT_OUTPUT+i].setVoltage(0.f);
          outputs[OUT_OUTPUT+i].setChannels(0);
        }
        else {
          float v[16];
          inputs[IN_INPUT+i].readVoltages(&v[0]);
          outputs[OUT_OUTPUT+i].setChannels(inputs[IN_INPUT+i].getChannels());
          outputs[OUT_OUTPUT+i].writeVoltages(&v[0]);
        }
      }
    }
    else {
      for(int i=0; i<9; i++){
        int cnt = inputs[IN_INPUT+i].getChannels();
        for (int c=oldCnt[i]; c<cnt; c++)
          channel[i][c].outState = channel[i][c].state = false;
        int mode = params[MODE_PARAM+i].getValue();
        bool muteIn = finalInMute && inMute->params[MUTE_PARAM+i].getValue();
        bool muteOut = finalOutMute && finalOutMute->params[MUTE_PARAM+i].getValue();
        for(int c=0; c<cnt; c++)
          outputs[OUT_OUTPUT+i].setVoltage( channel[i][c].proc(trig,inputs[IN_INPUT+i].getVoltage(c), preState, mode, muteIn, muteOut), c);
        outputs[OUT_OUTPUT+i].setChannels(cnt);
        oldCnt[i] = cnt;
      }
    }
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "toggleCV", json_boolean(toggleCV));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "toggleCV")))
      toggleCV = json_boolean_value(val);
  }

  void onExpanderChange(const ExpanderChangeEvent& e) override {
    if (e.side) { // right
      outMute = getRightExpander().module;
      if (outMute && outMute->model != modelVenomLinearBeatsExpander)
        outMute = NULL;
    }
    else { // left
      inMute = getLeftExpander().module;
      if ( inMute && inMute->model != modelVenomLinearBeatsExpander)
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
      addInput(createInputCentered<PolyPort>(Vec(16.5f,y), module, LinearBeats::IN_INPUT+i));
      addParam(createLockableParamCentered<ModeSwitch>(Vec(37.5f,y), module, LinearBeats::MODE_PARAM+i));
      addOutput(createOutputCentered<PolyPort>(Vec(58.5f,y), module, LinearBeats::OUT_OUTPUT+i));
      y+=31.556f;
    }
    addInput(createInputCentered<MonoPort>(Vec(16.5f,y), module, LinearBeats::CLOCK_INPUT));
  }

  void appendContextMenu(Menu* menu) override {
    LinearBeats* module = static_cast<LinearBeats*>(this->module);
    menu->addChild(new MenuSeparator);
    if (module->inMute)
      menu->addChild(createMenuLabel("Left Linear Beats expander connected"));
    else
      menu->addChild(createMenuItem("Add left Linear Beats expander", "", [this](){addExpander(modelVenomLinearBeatsExpander,this,true);}));
    if (module->outMute)
      menu->addChild(createMenuLabel("Right Linear Beats expander connected"));
    else
      menu->addChild(createMenuItem("Add right Linear Beats expander", "", [this](){addExpander(modelVenomLinearBeatsExpander,this,false);}));
    if (module->inMute || module->outMute) {
      menu->addChild(createBoolMenuItem("Expander CV toggles button on/off", "",
        [=]() {
          return module->toggleCV;
        },
        [=](bool val) {
          module->toggleCV = val;
        }
      ));
    }
    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelVenomLinearBeats = createModel<LinearBeats, LinearBeatsWidget>("LinearBeats");

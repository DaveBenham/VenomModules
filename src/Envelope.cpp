// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "EnvelopeModule.hpp"

namespace Venom {

struct Envelope : EnvelopeModule {

  bool reset = false;  
  struct Stage {
    EnvelopeModule *mod = NULL;
    int action = -1;
  };
  Stage stages[20]{};
  int stageCnt = 4;

  Envelope() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(SLOW_PARAM, 0.f, 1.f, 0.f, "Slow (x10) times", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(FROM0_PARAM, 0.f, 1.f, 0.f, "Retrigger from 0", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(GATE_IN_PARAM, 0.f, 1.f, 0.f, "Manual gate", {"Low", "High"});
    configInput(GATE_INPUT, "Gate");
    configLight(IDLE_LIGHT, "Idle indicator");
    configOutput(IDLE_OUTPUT, "Idle gate");
    configOutput(INV_OUTPUT, "Inverse envelope");
    configOutput(ENV_OUTPUT, "Envelope");

    for (int i=0; i<4; i++) {
      std::string prefix = "Stage " + std::to_string(i);
      configLight(UP_LIGHT+i, "Stage 1 10V target indicator");
      configSwitch<FixedSwitchQuantity>(ACTION_PARAM+i, 0.f, 2.f, 0.f, prefix+" action", {"Move", "Hold", "Sustain"});
      configLight(DOWN_LIGHT+i, prefix+" 0V target indicator");
      configSwitch<FixedSwitchQuantity>(MODE_PARAM+i, 0.f, 2.f, 0.f, prefix+" mode", {"Full", "Retriggerable Full", "Gate"});
      configParam(A_PARAM+i, -3.f, 1.f, -1.f, prefix+" move time", " s", 10.f, 1.f, 0.f);
      configParam(A_CV_PARAM+i, -0.1f, 0.1f, 0.f, prefix+" move time CV", "%", 0.f, 1000.f, 0.f);
      configInput(A_CV_INPUT+i, prefix+" move time CV");
      configParam<BQuantity>(B_PARAM+i, -3.f, 1.f, -1.f, prefix+" move shape", "", 0.f, 0.5f, 0.5f);
      configParam(B_CV_PARAM+i, -0.1f, 0.1f, 0.f, prefix+" move shape CV", "%", 0.f, 1000.f, 0.f);
      configInput(B_CV_INPUT+i, prefix+" move shape CV");
      configLight(GATE_LIGHT+i, prefix+" gate indicator");
      configOutput(GATE_OUTPUT+i, prefix+" gate");
    }
  }
  
  void process(const ProcessArgs& args) override {
    EnvelopeModule::process(args);
    
    // get channel count
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++) {
      int c = inputs[i].getChannels();
      if (c>channels)
        channels = c;
    }
    
    bool hi = true;
    int nextAction = params[ACTION_PARAM].getValue();
    for (int i=0; i<4; i++) {
      if (stages[i].action != nextAction) {
        reset = true;
        stages[i].action = nextAction;
      }
      if (nextAction==2)
        params[MODE_PARAM+i].setValue(2);
      nextAction = i<3 ? params[ACTION_PARAM+i+1].getValue() : (expander ? expander->params[ACTION_EXP_PARAM].getValue() : 0);
      up[i] = (stages[i].action==0 && nextAction==0) ? hi : false;
      down[i] = (stages[i].action==0 && nextAction==0) ? !hi : false;
      hi = (stages[i].action==0 && nextAction==0) ? !hi : hi;
    }
    EnvelopeModule *curExpander = expander;
    int newStageCnt = 4;
    while(curExpander && newStageCnt<20) {
      if (stages[newStageCnt].mod != curExpander || stages[newStageCnt].action != nextAction) {
        reset = true;
        stages[newStageCnt].mod = curExpander;
        stages[newStageCnt].action = nextAction;
      }
      if (nextAction==2)
        curExpander->params[MODE_EXP_PARAM].setValue(2);
      nextAction = curExpander->expander ? curExpander->expander->params[ACTION_EXP_PARAM].getValue() : 0;
      curExpander->up[0] = (stages[newStageCnt].action==0 && nextAction==0) ? hi : false;
      curExpander->down[0] = (stages[newStageCnt].action==0 && nextAction==0) ? !hi : false;
      hi = (stages[newStageCnt].action==0 && nextAction==0) ? !hi : hi;
      curExpander = curExpander->expander;
      newStageCnt++;
    }
    if (newStageCnt<stageCnt) {
      for (int i=newStageCnt; i<stageCnt; i++) {
        stages[i].mod = NULL;
        stages[i].action = -1;
      }
    }
    stageCnt = newStageCnt;
    
    
    reset = false;
  }
  
};

struct EnvelopeWidget : EnvelopeModuleWidget {
  int action[4]{-1,-1,-1,-1};
  int slow=0;

  EnvelopeWidget(Envelope* module) {
    setModule(module);
    setVenomPanel("Envelope");
    
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(22.5f,108.f), module, EnvelopeModule::SLOW_PARAM, EnvelopeModule::SLOW_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(22.5f,150.f), module, EnvelopeModule::FROM0_PARAM, EnvelopeModule::FROM0_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteLight>>>(Vec(22.5,185.5f), module, EnvelopeModule::GATE_IN_PARAM, EnvelopeModule::GATE_IN_LIGHT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 218.5f), module, EnvelopeModule::GATE_INPUT));
    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(22.5f,241.f), module, EnvelopeModule::IDLE_LIGHT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5, 266.5f), module, EnvelopeModule::IDLE_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5, 304.5f), module, EnvelopeModule::INV_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5, 342.5f), module, EnvelopeModule::ENV_OUTPUT));
    for (int i=0; i<4; i++) {
      addChild((labelWidget[i] = new LabelWidget(Vec(45.f+i*45, 0.f))));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(67.5f+i*45, 23.5f), module, EnvelopeModule::UP_LIGHT+i));
      addParam(createLockableParamCentered<ActionSwitch>(Vec(67.5f+i*45, 40.f), module, EnvelopeModule::ACTION_PARAM+i));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(67.5f+i*45, 56.5f), module, EnvelopeModule::DOWN_LIGHT+i));
      addParam(createLockableParamCentered<ModeSwitch>(Vec(67.5f+i*45, 73.f), module, EnvelopeModule::MODE_PARAM+i));
      addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(67.5f+i*45, 114.5f), module, EnvelopeModule::A_PARAM+i));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(67.5f+i*45, 150.5f), module, EnvelopeModule::A_CV_PARAM+i));
      addInput(createInputCentered<PolyPort>(Vec(67.5f+i*45, 184.f), module, EnvelopeModule::A_CV_INPUT+i));
      addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(67.5f+i*45, 225.f), module, EnvelopeModule::B_PARAM+i));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(67.5f+i*45, 261.f), module, EnvelopeModule::B_CV_PARAM+i));
      addInput(createInputCentered<PolyPort>(Vec(67.5f+i*45, 294.5f), module, EnvelopeModule::B_CV_INPUT+i));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(67.5f+i*45, 317.f), module, EnvelopeModule::GATE_LIGHT+i));
      addOutput(createOutputCentered<PolyPort>(Vec(67.5f+i*45, 342.5f), module, EnvelopeModule::GATE_OUTPUT+i));
    }
  }

  void step() override {
    EnvelopeModuleWidget::step();
    if(this->module) {
      Envelope* mod = static_cast<Envelope*>(this->module);
      int newSlow = mod->params[EnvelopeModule::SLOW_PARAM].getValue();
      for (int i=0; i<4; i++) { // reconfigure A and B
        int newAction = mod->params[EnvelopeModule::ACTION_PARAM+i].getValue();
        if (newAction!=action[i] || newSlow!=slow) {
          action[i] = newAction;
          std::string prefix = "Stage " + std::to_string(i+1);
          ParamQuantity *aq = mod->paramQuantities[EnvelopeModule::A_PARAM+i];
          EnvelopeModule::BQuantity *bq = static_cast<EnvelopeModule::BQuantity*>(mod->paramQuantities[EnvelopeModule::B_PARAM+i]);
          bq->action = newAction;
          switch (newAction) {
            case 0: // MOVE
              mod->setParamFactoryName(EnvelopeModule::A_PARAM+i, prefix + " move time", true);
              mod->setParamFactoryName(EnvelopeModule::A_CV_PARAM+i, prefix + " move time CV", true);
              mod->setPortFactoryName(EnvelopeModule::A_CV_INPUT+i, prefix + " move time CV", false, true);
              mod->setParamFactoryName(EnvelopeModule::B_PARAM+i, prefix + " move shape", true);
              mod->setParamFactoryName(EnvelopeModule::B_CV_PARAM+i, prefix + " move shape CV", true);
              mod->setPortFactoryName(EnvelopeModule::B_CV_INPUT+i, prefix + " move shape CV", false, true);
              aq->unit = " s";
              aq->displayBase = 10.f;
              aq->displayMultiplier = newSlow ? 10.f : 1.f;
              aq->displayOffset = 0.f;
              bq->unit = "";
              bq->displayBase = 0.f;
              bq->displayMultiplier = 0.5f;
              bq->displayOffset = 0.5f;
              break;
            case 1: // HOLD
              mod->setParamFactoryName(EnvelopeModule::A_PARAM+i, prefix + " hold level", true);
              mod->setParamFactoryName(EnvelopeModule::A_CV_PARAM+i, prefix + " hold level CV", true);
              mod->setPortFactoryName(EnvelopeModule::A_CV_INPUT+i, prefix + " hold level CV", false, true);
              mod->setParamFactoryName(EnvelopeModule::B_PARAM+i, prefix + " hold time", true);
              mod->setParamFactoryName(EnvelopeModule::B_CV_PARAM+i, prefix + " hold time CV", true);
              mod->setPortFactoryName(EnvelopeModule::B_CV_INPUT+i, prefix + " hold time CV", false, true);
              aq->unit = "%";
              aq->displayBase = 0.f;
              aq->displayMultiplier = 25.f;
              aq->displayOffset = 75.f;
              bq->unit = " s";
              bq->displayBase = 10.f;
              bq->displayMultiplier = newSlow ? 10.f : 1.f;
              bq->displayOffset = 0.f;
              break;
            case 2: // SUST
              mod->setParamFactoryName(EnvelopeModule::A_PARAM+i, prefix + " sustain level", true);
              mod->setParamFactoryName(EnvelopeModule::A_CV_PARAM+i, prefix + " sustain level CV", true);
              mod->setPortFactoryName(EnvelopeModule::A_CV_INPUT+i, prefix + " sustain level CV", false, true);
              mod->setParamFactoryName(EnvelopeModule::B_PARAM+i, prefix + " sustain drift", true);
              mod->setParamFactoryName(EnvelopeModule::B_CV_PARAM+i, prefix + " sustain drift CV", true);
              mod->setPortFactoryName(EnvelopeModule::B_CV_INPUT+i, prefix + " sustain drift CV", false, true);
              aq->unit = "%";
              aq->displayBase = 0.f;
              aq->displayMultiplier = 25.f;
              aq->displayOffset = 75.f;
              bq->unit = " V/s";
              bq->displayBase = 10.f;
              bq->displayMultiplier = 0.1f;
              bq->displayOffset = 0.f;
              break;
          }
        }
        if (labelWidget[i])
          labelWidget[i]->setLabel(currentTheme, action[i]);
        mod->lights[Envelope::UP_LIGHT+i].setBrightness(mod->up[i]);
        mod->lights[Envelope::DOWN_LIGHT+i].setBrightness(mod->down[i]);
      }
      slow = newSlow;
      mod->lights[Envelope::SLOW_LIGHT].setBrightness(mod->params[Envelope::SLOW_PARAM].getValue());
      mod->lights[Envelope::FROM0_LIGHT].setBrightness(mod->params[Envelope::FROM0_PARAM].getValue());
      mod->lights[Envelope::GATE_IN_LIGHT].setBrightness(mod->params[Envelope::GATE_IN_PARAM].getValue());
    }
    else for (int i=0; i<4; i++) {
      if (labelWidget[i])
        labelWidget[i]->setLabel(settings::preferDarkPanels ? darkTheme : lightTheme, 0);
    }
  }

};

}

Model* modelVenomEnvelope = createModel<Venom::Envelope, Venom::EnvelopeWidget>("Envelope");

// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "EnvelopeModule.hpp"

namespace Venom {

struct EnvelopeExpander : EnvelopeModule {
  
  EnvelopeExpander() {
    venomConfig(EXP_PARAMS_LEN, EXP_INPUTS_LEN, EXP_OUTPUTS_LEN, EXP_LIGHTS_LEN);
    configLight(EXPAND_EXP_LIGHT, "Left connection indicator");
    configLight(UP_EXP_LIGHT, "Stage n 10V target indicator");
    configSwitch<FixedSwitchQuantity>(ACTION_EXP_PARAM, 0.f, 2.f, 0.f, "Stage n action", {"Move", "Hold", "Sustain"});
    configLight(DOWN_EXP_LIGHT, "Stage n 0V target indicator");
    configSwitch<FixedSwitchQuantity>(MODE_EXP_PARAM, 0.f, 2.f, 0.f, "Stage n mode", {"Full", "Retriggerable Full", "Gate"});
    configParam(A_EXP_PARAM, -3.f, 1.f, -1.f, "Stage n move time", " s", 10.f, 1.f, 0.f);
    configParam(A_CV_EXP_PARAM, -0.1f, 0.1f, 0.f, "Stage n move time CV", "%", 0.f, 1000.f, 0.f);
    configInput(A_CV_EXP_INPUT, "Stage n move time CV");
    configParam<BQuantity>(B_EXP_PARAM, -3.f, 1.f, -1.f, "Stage n move shape", "", 0.f, 0.5f, 0.5f);
    configParam(B_CV_EXP_PARAM, -0.1f, 0.1f, 0.f, "Stage n move shape CV", "%", 0.f, 1000.f, 0.f);
    configInput(B_CV_EXP_INPUT, "Stage n move shape CV");
    configLight(GATE_EXP_LIGHT, "Stage n gate indicator");
    configOutput(GATE_EXP_OUTPUT, "Stage n gate");
  }
  

  void process(const ProcessArgs& args) override {
    EnvelopeModule::process(args);

  }
  
};

struct EnvelopeExpanderWidget : EnvelopeModuleWidget {
  int stage = 0,
      action = -1,
      slow = 0;
  bool connected = false;

  EnvelopeExpanderWidget(EnvelopeExpander* module) {
    setModule(module);
    setVenomPanel("EnvelopeExpander");

    addChild((labelWidget[0] = new LabelWidget(Vec(0.f, 0.f))));
    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(5.f, 6.5f), module, EnvelopeExpander::EXPAND_EXP_LIGHT));
    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(22.5f, 23.5f), module, EnvelopeExpander::UP_EXP_LIGHT));
    addParam(createLockableParamCentered<ActionSwitch>(Vec(22.5f, 40.f), module, EnvelopeExpander::ACTION_EXP_PARAM));
    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(22.5f, 56.5f), module, EnvelopeExpander::DOWN_EXP_LIGHT));
    addParam(createLockableParamCentered<ModeSwitch>(Vec(22.5f, 73.f), module, EnvelopeExpander::MODE_EXP_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5f, 114.5f), module, EnvelopeExpander::A_EXP_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 150.5f), module, EnvelopeExpander::A_CV_EXP_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 184.f), module, EnvelopeExpander::A_CV_EXP_INPUT));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5f, 225.f), module, EnvelopeExpander::B_EXP_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 261.f), module, EnvelopeExpander::B_CV_EXP_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 294.5f), module, EnvelopeExpander::B_CV_EXP_INPUT));
    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(22.5f, 317.f), module, EnvelopeExpander::GATE_EXP_LIGHT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 342.5f), module, EnvelopeExpander::GATE_EXP_OUTPUT));
  }

  void step() override {
    EnvelopeModuleWidget::step();
    
    if (this->module){
      EnvelopeExpander* mod = static_cast<EnvelopeExpander*>(this->module);
      int newSlow = 0;

      // test connection
      bool newConnected = false;
      int newStage = 5;
      for (Module *leftMod = this->module->getLeftExpander().module; leftMod && newStage<=20; leftMod = leftMod->getLeftExpander().module, newStage++) {
        if (leftMod->model == modelVenomEnvelope){
          newSlow = leftMod->params[EnvelopeModule::SLOW_PARAM].getValue();
          newConnected = true;
          stage = newStage;
          break;
        }
        if (leftMod->model != modelVenomEnvelopeExpander) {
          newStage = 21;
          break;
        }
      }

      // reconfigure A and B names
      if (action != mod->params[EnvelopeModule::ACTION_EXP_PARAM].getValue() || newStage != stage  || connected != newConnected || newSlow != slow) {
        action = mod->params[EnvelopeModule::ACTION_EXP_PARAM].getValue();
        stage = newStage;
        connected = newConnected;
        slow = newSlow;
        std::string prefix = connected ? "Stage " + std::to_string(stage) : "Stage n";
        mod->lights[EnvelopeModule::EXPAND_EXP_LIGHT].setBrightness(connected);  
        mod->lightInfos[EnvelopeModule::UP_EXP_LIGHT]->name = prefix + " 10V target indicator";
        mod->lightInfos[EnvelopeModule::DOWN_EXP_LIGHT]->name = prefix + " 0V target indicator";
        mod->setParamFactoryName(EnvelopeModule::ACTION_EXP_PARAM, prefix + " action", true);
        mod->setParamFactoryName(EnvelopeModule::MODE_EXP_PARAM, prefix + " mode", true);
        ParamQuantity *aq = mod->paramQuantities[EnvelopeModule::A_EXP_PARAM];
        EnvelopeModule::BQuantity *bq = static_cast<EnvelopeModule::BQuantity*>(mod->paramQuantities[EnvelopeModule::B_EXP_PARAM]);
        bq->action = action;
        switch (action) {
          case 0: // MOVE
            mod->setParamFactoryName(EnvelopeModule::A_EXP_PARAM, prefix + " move time", true);
            mod->setParamFactoryName(EnvelopeModule::A_CV_EXP_PARAM, prefix + " move time CV", true);
            mod->setPortFactoryName(EnvelopeModule::A_CV_EXP_INPUT, prefix + " move time CV", false, true);
            mod->setParamFactoryName(EnvelopeModule::B_EXP_PARAM, prefix + " move shape", true);
            mod->setParamFactoryName(EnvelopeModule::B_CV_EXP_PARAM, prefix + " move shape CV", true);
            mod->setPortFactoryName(EnvelopeModule::B_CV_EXP_INPUT, prefix + " move shape CV", false, true);
            aq->unit = " s";
            aq->displayBase = 10.f;
            aq->displayMultiplier = slow==2 ? 100.f : slow ? 10.f : 1.f;
            aq->displayOffset = 0.f;
            bq->unit = "";
            bq->displayBase = 0.f;
            bq->displayMultiplier = 0.5f;
            bq->displayOffset = 0.5f;
            break;
          case 1: // HOLD
            mod->setParamFactoryName(EnvelopeModule::A_EXP_PARAM, prefix + " hold level", true);
            mod->setParamFactoryName(EnvelopeModule::A_CV_EXP_PARAM, prefix + " hold level CV", true);
            mod->setPortFactoryName(EnvelopeModule::A_CV_EXP_INPUT, prefix + " hold level CV", false, true);
            mod->setParamFactoryName(EnvelopeModule::B_EXP_PARAM, prefix + " hold time", true);
            mod->setParamFactoryName(EnvelopeModule::B_CV_EXP_PARAM, prefix + " hold time CV", true);
            mod->setPortFactoryName(EnvelopeModule::B_CV_EXP_INPUT, prefix + " hold time CV", false, true);
            aq->unit = "%";
            aq->displayBase = 0.f;
            aq->displayMultiplier = 25.f;
            aq->displayOffset = 75.f;
            bq->unit = " s";
            bq->displayBase = 10.f;
            bq->displayMultiplier = slow==2 ? 100.f : slow ? 10.f : 1.f;
            bq->displayOffset = 0.f;
            break;
          case 2: // SUST
            mod->setParamFactoryName(EnvelopeModule::A_EXP_PARAM, prefix + " sustain level", true);
            mod->setParamFactoryName(EnvelopeModule::A_CV_EXP_PARAM, prefix + " sustain level CV", true);
            mod->setPortFactoryName(EnvelopeModule::A_CV_EXP_INPUT, prefix + " sustain level CV", false, true);
            mod->setParamFactoryName(EnvelopeModule::B_EXP_PARAM, prefix + " sustain drift", true);
            mod->setParamFactoryName(EnvelopeModule::B_CV_EXP_PARAM, prefix + " sustain drift CV", true);
            mod->setPortFactoryName(EnvelopeModule::B_CV_EXP_INPUT, prefix + " sustain drift CV", false, true);
            aq->unit = "%";
            aq->displayBase = 0.f;
            aq->displayMultiplier = 25.f;
            aq->displayOffset = 75.f;
            bq->unit = " V/s";
            bq->displayBase = 10.f;
            bq->displayMultiplier = 10.f;
            bq->displayOffset = 0.f;
            break;
        }
        mod->lightInfos[EnvelopeModule::GATE_EXP_LIGHT]->name = prefix + " gate indicator";
        mod->setPortFactoryName(EnvelopeModule::GATE_EXP_OUTPUT, prefix + " gate", true, true);
      }
      if (labelWidget[0])
        labelWidget[0]->setLabel(currentTheme, mod->params[EnvelopeModule::ACTION_EXP_PARAM].getValue());
      if (!connected) {
        mod->up[0]=0;
        mod->down[0]=0;
      }
      mod->lights[EnvelopeModule::UP_EXP_LIGHT].setBrightness(mod->up[0]);
      mod->lights[EnvelopeModule::DOWN_EXP_LIGHT].setBrightness(mod->down[0]);
    }
    else if (labelWidget[0]) {
      labelWidget[0]->setLabel(settings::preferDarkPanels ? darkTheme : lightTheme, 0);
    }  
  }

};

}

Model* modelVenomEnvelopeExpander = createModel<Venom::EnvelopeExpander, Venom::EnvelopeExpanderWidget>("EnvelopeExpander");

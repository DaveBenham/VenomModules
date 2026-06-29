// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "math.hpp"

namespace Venom {

struct EnvelopeExpander : VenomModule {

  enum ParamId {
    ACTION_PARAM,
    MODE_PARAM,
    A_PARAM,
    A_CV_PARAM,
    B_PARAM,
    B_CV_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    A_CV_INPUT,
    B_CV_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    GATE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    UP_LIGHT,
    DOWN_LIGHT,
    GATE_LIGHT,
    EXPAND_LIGHT,
    LIGHTS_LEN
  };
  
  int slow = 0,
      action = -1;

  struct BQuantity : ParamQuantity {
    int action = 0;
    float getDisplayValue() override {
      return action && getValue() == -3.f ? 0.f : ParamQuantity::getDisplayValue();
    }
  };

  EnvelopeExpander() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configLight(EXPAND_LIGHT, "Left connection indicator");
    configLight(UP_LIGHT, "Stage n 10V target indicator");
    configSwitch<FixedSwitchQuantity>(ACTION_PARAM, 0.f, 2.f, 0.f, "Stage n action", {"Move", "Hold", "Sustain"});
    configLight(DOWN_LIGHT, "Stage n 0V target indicator");
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 2.f, 0.f, "Stage 1 mode", {"Full", "Retriggerable Full", "Gate"});
    configParam(A_PARAM, -3.f, 1.f, -1.f, "Stage n move time", " s", 10.f, 1.f, 0.f);
    configParam(A_CV_PARAM, -0.1f, 0.1f, 0.f, "Stage n move time CV", "%", 0.f, 1000.f, 0.f);
    configInput(A_CV_INPUT, "Stage n move time CV");
    configParam<BQuantity>(B_PARAM, -3.f, 1.f, -1.f, "Stage n move shape", "", 0.f, 0.5f, 0.5f);
    configParam(B_CV_PARAM, -0.1f, 0.1f, 0.f, "Stage n move shape CV", "%", 0.f, 1000.f, 0.f);
    configInput(B_CV_INPUT, "Stage n move shape CV");
    configLight(GATE_LIGHT, "Stage n gate indicator");
    configOutput(GATE_OUTPUT, "Stage n gate");
  }
  

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

/*    
    // reconfigure A and B params
    if (slow != params[SLOW_PARAM].getValue()) {
      action[0] = action[1] = action[2] = action[3] = -1;
      slow = params[SLOW_PARAM].getValue());
    }
    for (int i=0; i<4; i++) {
      if (action[i] != params[ACTION_PARAM+i*STAGE_PARAMS].getValue()) {
        action[i] = params[ACTION_PARAM+i*STAGE_PARAMS].getValue();
        ParamQuantity *aq = paramQuantities[A_PARAM+i*STAGE_PARAMS];
        BQuantity *bq = static_cast<BQuantity*>(paramQuantities[B_PARAM+i*STAGE_PARAMS]);
        bq->action = action[i];
        switch(action[i]) {
          case 0: // MOVE : A = time, B = shape
            aq->unit = " s";
            aq->displayBase = 10.f;
            aq->displayMultiplier = slow ? 10.f : 1.f;
            aq->offset = 0.f;
            bq->unit = "";
            bq->displayBase = 0.f;
            bq->displayMultiplier = 0.5f;
            bq->offset = 0.5f;
            break;
          case 1: // HOLD : A = Level, B = time
            aq->unit = "%";
            aq->displayBase = 0.f;
            aq->displayMultipler = 0.25f;
            aq->offset = 0.75f;
            bq->unit = " s";
            bq->displayBase = 10.f;
            bq->displayMultiplier = slow ? 10.f : 1.f;
            bq->offset = 0.f;
            break;
          case 2: // SUST : A = Level, B = drift
            aq->unit = "%";
            aq->displayBase = 0.f;
            aq->displayMultipler = 0.25f;
            aq->offset = 0.75f;
            bq->unit = " V/s";
            bq->displayBase = 10.f;
            bq->displayMultiplier = 0.1f;
            bq->offset = 0.f;
            break;
        }
      }
    }
*/
  }
  
};

struct EnvelopeExpanderWidget : VenomWidget {

  int action = -1;

  struct ActionSwitch : GlowingSvgSwitchLockable {
    ActionSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Move.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Hold.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Sust.svg")));
    }
  };

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_Full.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_RTrg.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_Gate.svg")));
    }
  };

  EnvelopeExpanderWidget(EnvelopeExpander* module) {
    setModule(module);
    setVenomPanel("EnvelopeExpander");
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(5.f, 6.5f), module, EnvelopeExpander::EXPAND_LIGHT));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(22.5f, 23.5f), module, EnvelopeExpander::UP_LIGHT));
      addParam(createLockableParamCentered<ActionSwitch>(Vec(22.5f, 40.f), module, EnvelopeExpander::ACTION_PARAM));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(22.5f, 56.5f), module, EnvelopeExpander::DOWN_LIGHT));
      addParam(createLockableParamCentered<ModeSwitch>(Vec(22.5f, 73.f), module, EnvelopeExpander::MODE_PARAM));
      addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5f, 114.5f), module, EnvelopeExpander::A_PARAM));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 150.5f), module, EnvelopeExpander::A_CV_PARAM));
      addInput(createInputCentered<PolyPort>(Vec(22.5f, 184.f), module, EnvelopeExpander::A_CV_INPUT));
      addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5f, 225.f), module, EnvelopeExpander::B_PARAM));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 261.f), module, EnvelopeExpander::B_CV_PARAM));
      addInput(createInputCentered<PolyPort>(Vec(22.5f, 294.5f), module, EnvelopeExpander::B_CV_INPUT));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(22.5f, 317.f), module, EnvelopeExpander::GATE_LIGHT));
      addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 342.5f), module, EnvelopeExpander::GATE_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    if(this->module) {
      EnvelopeExpander* mod = static_cast<EnvelopeExpander*>(this->module);
/*
      for (int i=0; i<4; i++) { // reconfigure A and B names
        if (action[i] != mod->action[i]) {
          action[i] = mod->action[i];
          std::string prefix = "Stage " + std::to_string(i+1);
          switch (action[i]) {
            case 0: // MOVE
              mod->setParamFactoryName(A_PARAM, prefix + " move time");
              mod->setParamFactoryName(A_CV_PARAM, prefix + " move time CV");
              mod->setPortFactoryName(A_CV_INPUT, prefix + " move time CV");
              mod->setParamFactoryName(B_PARAM, prefix + " move shape");
              mod->setParamFactoryName(B_CV_PARAM, prefix + " move shape CV");
              mod->setPortFactoryName(B_CV_INPUT, prefix + " move shape CV");
              break;
            case 1: // HOLD
              mod->setParamFactoryName(A_PARAM, prefix + " hold level");
              mod->setParamFactoryName(A_CV_PARAM, prefix + " hold level CV");
              mod->setPortFactoryName(A_CV_INPUT, prefix + " hold level CV");
              mod->setParamFactoryName(B_PARAM, prefix + " hold time");
              mod->setParamFactoryName(B_CV_PARAM, prefix + " hold time CV");
              mod->setPortFactoryName(B_CV_INPUT, prefix + " hold time CV");
              break;
            case 2: // SUST
              mod->setParamFactoryName(A_PARAM, prefix + " sustain level");
              mod->setParamFactoryName(A_CV_PARAM, prefix + " sustain level CV");
              mod->setPortFactoryName(A_CV_INPUT, prefix + " sustain level CV");
              mod->setParamFactoryName(B_PARAM, prefix + " sustain drift");
              mod->setParamFactoryName(B_CV_PARAM, prefix + " sustain drift CV");
              mod->setPortFactoryName(B_CV_INPUT, prefix + " sustain drift CV");
              break;
          }
        }
      }
*/
    }
  }
};

}

Model* modelVenomEnvelopeExpander = createModel<Venom::EnvelopeExpander, Venom::EnvelopeExpanderWidget>("EnvelopeExpander");

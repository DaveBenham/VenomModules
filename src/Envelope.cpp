// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "math.hpp"

namespace Venom {

struct Envelope : VenomModule {

  enum ParamId {
    ACTION_PARAM,
    MODE_PARAM,
    A_PARAM,
    A_CV_PARAM,
    B_PARAM,
    B_CV_PARAM,
    ACTION2_PARAM,
    MODE2_PARAM,
    A2_PARAM,
    A2_CV_PARAM,
    B2_PARAM,
    B2_CV_PARAM,
    ACTION3_PARAM,
    MODE3_PARAM,
    A3_PARAM,
    A3_CV_PARAM,
    B3_PARAM,
    B3_CV_PARAM,
    ACTION4_PARAM,
    MODE4_PARAM,
    A4_PARAM,
    A4_CV_PARAM,
    B4_PARAM,
    B4_CV_PARAM,
    SLOW_PARAM,
    FROM0_PARAM,
    GATE_IN_PARAM,
    PARAMS_LEN
  };
  #define STAGE_PARAMS 6
  enum InputId {
    A_CV_INPUT,
    B_CV_INPUT,
    A2_CV_INPUT,
    B2_CV_INPUT,
    A3_CV_INPUT,
    B3_CV_INPUT,
    A4_CV_INPUT,
    B4_CV_INPUT,
    GATE_INPUT,
    INPUTS_LEN
  };
  #define STAGE_INPUTS 2
  enum OutputId {
    GATE_OUTPUT,
    GATE2_OUTPUT,
    GATE3_OUTPUT,
    GATE4_OUTPUT,
    IDLE_OUTPUT,
    INV_OUTPUT,
    ENV_OUTPUT,
    OUTPUTS_LEN
  };
  #define STAGE_OUTPUTS 1
  enum LightId {
    UP_LIGHT,
    DOWN_LIGHT,
    GATE_LIGHT,
    UP2_LIGHT,
    DOWN2_LIGHT,
    GATE2_LIGHT,
    UP3_LIGHT,
    DOWN3_LIGHT,
    GATE3_LIGHT,
    UP4_LIGHT,
    DOWN4_LIGHT,
    GATE4_LIGHT,
    SLOW_LIGHT,
    FROM0_LIGHT,
    GATE_IN_LIGHT,
    IDLE_LIGHT,
    LIGHTS_LEN
  };
  #define STAGE_LIGHTS 3

  bool up[4]{},
       down[4]{};
  int slow = 0,
      action[4]{-1,-1,-1,-1};

  struct BQuantity : ParamQuantity {
    int action = 0;
    float getDisplayValue() override {
      return action && getValue() == -3.f ? 0.f : ParamQuantity::getDisplayValue();
    }
  };

  Envelope() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(SLOW_PARAM, 0.f, 1.f, 0.f, "Slow (x10) times", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(FROM0_PARAM, 0.f, 1.f, 0.f, "Retrigger from 0", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(GATE_IN_PARAM, 0.f, 1.f, 0.f, "Manual gate", {"Off", "On"});
    configInput(GATE_INPUT, "Gate");
    configLight(IDLE_LIGHT, "Idle indicator");
    configOutput(IDLE_OUTPUT, "Idle gate");
    configOutput(INV_OUTPUT, "Inverse envelope");
    configOutput(ENV_OUTPUT, "Envelope");

    configLight(UP_LIGHT, "Stage 1 10V target indicator");
    configSwitch<FixedSwitchQuantity>(ACTION_PARAM, 0.f, 2.f, 0.f, "Stage 1 action", {"Move", "Hold", "Sustain"});
    configLight(DOWN_LIGHT, "Stage 1 0V target indicator");
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 2.f, 0.f, "Stage 1 mode", {"Full", "Retriggerable Full", "Gate"});
    configParam(A_PARAM, -3.f, 1.f, -1.f, "Stage 1 move time", " s", 10.f, 1.f, 0.f);
    configParam(A_CV_PARAM, -0.1f, 0.1f, 0.f, "Stage 1 move time CV", "%", 0.f, 1000.f, 0.f);
    configInput(A_CV_INPUT, "Stage 1 move time CV");
    configParam<BQuantity>(B_PARAM, -3.f, 1.f, -1.f, "Stage 1 move shape", "", 0.f, 0.5f, 0.5f);
    configParam(B_CV_PARAM, -0.1f, 0.1f, 0.f, "Stage 1 move shape CV", "%", 0.f, 1000.f, 0.f);
    configInput(B_CV_INPUT, "Stage 1 move shape CV");
    configLight(GATE_LIGHT, "Stage 1 gate indicator");
    configOutput(GATE_OUTPUT, "Stage 1 gate");

    configLight(UP2_LIGHT, "Stage 2 10V target indicator");
    configSwitch<FixedSwitchQuantity>(ACTION2_PARAM, 0.f, 2.f, 0.f, "Stage 2 action", {"Move", "Hold", "Sustain"});
    configLight(DOWN2_LIGHT, "Stage 2 0V target indicator");
    configSwitch<FixedSwitchQuantity>(MODE2_PARAM, 0.f, 2.f, 0.f, "Stage 2 mode", {"Full", "Retriggerable Full", "Gate"});
    configParam(A2_PARAM, -3.f, 1.f, -1.f, "Stage 2 move time", " s", 10.f, 1.f, 0.f);
    configParam(A2_CV_PARAM, -0.1f, 0.1f, 0.f, "Stage 2 move time CV", "%", 0.f, 1000.f, 0.f);
    configInput(A2_CV_INPUT, "Stage 2 move time CV");
    configParam<BQuantity>(B2_PARAM, -3.f, 1.f, -1.f, "Stage 2 move shape", "", 0.f, 0.5f, 0.5f);
    configParam(B2_CV_PARAM, -0.1f, 0.1f, 0.f, "Stage 2 move shape CV", "%", 0.f, 1000.f, 0.f);
    configInput(B2_CV_INPUT, "Stage 2 move shape CV");
    configLight(GATE2_LIGHT, "Stage 2 gate indicator");
    configOutput(GATE2_OUTPUT, "Stage 2 gate");

    configLight(UP3_LIGHT, "Stage 3 10V target indicator");
    configSwitch<FixedSwitchQuantity>(ACTION3_PARAM, 0.f, 2.f, 0.f, "Stage 3 action", {"Move", "Hold", "Sustain"});
    configLight(DOWN3_LIGHT, "Stage 3 0V target indicator");
    configSwitch<FixedSwitchQuantity>(MODE3_PARAM, 0.f, 2.f, 0.f, "Stage 3 mode", {"Full", "Retriggerable Full", "Gate"});
    configParam(A3_PARAM, -3.f, 1.f, -1.f, "Stage 3 move time", " s", 10.f, 1.f, 0.f);
    configParam(A3_CV_PARAM, -0.1f, 0.1f, 0.f, "Stage 3 move time CV", "%", 0.f, 1000.f, 0.f);
    configInput(A3_CV_INPUT, "Stage 3 move time CV");
    configParam<BQuantity>(B3_PARAM, -3.f, 1.f, -1.f, "Stage 3 move shape", "", 0.f, 0.5f, 0.5f);
    configParam(B3_CV_PARAM, -0.1f, 0.1f, 0.f, "Stage 3 move shape CV", "%", 0.f, 1000.f, 0.f);
    configInput(B3_CV_INPUT, "Stage 3 move shape CV");
    configLight(GATE3_LIGHT, "Stage 3 gate indicator");
    configOutput(GATE3_OUTPUT, "Stage 3 gate");

    configLight(UP4_LIGHT, "Stage 4 10V target indicator");
    configSwitch<FixedSwitchQuantity>(ACTION4_PARAM, 0.f, 2.f, 0.f, "Stage 4 action", {"Move", "Hold", "Sustain"});
    configLight(DOWN4_LIGHT, "Stage 4 0V target indicator");
    configSwitch<FixedSwitchQuantity>(MODE4_PARAM, 0.f, 2.f, 0.f, "Stage 4 mode", {"Full", "Retriggerable Full", "Gate"});
    configParam(A4_PARAM, -3.f, 1.f, -1.f, "Stage 4 move time", " s", 10.f, 1.f, 0.f);
    configParam(A4_CV_PARAM, -0.1f, 0.1f, 0.f, "Stage 4 move time CV", "%", 0.f, 1000.f, 0.f);
    configInput(A4_CV_INPUT, "Stage 4 move time CV");
    configParam<BQuantity>(B4_PARAM, -3.f, 1.f, -1.f, "Stage 4 move shape", "", 0.f, 0.5f, 0.5f);
    configParam(B4_CV_PARAM, -0.1f, 0.1f, 0.f, "Stage 4 move shape CV", "%", 0.f, 1000.f, 0.f);
    configInput(B4_CV_INPUT, "Stage 4 move shape CV");
    configLight(GATE4_LIGHT, "Stage 4 gate indicator");
    configOutput(GATE4_OUTPUT, "Stage 4 gate");
  }
  

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    
    // get channel count
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++) {
      int c = inputs[i].getChannels();
      if (c>channels)
        channels = c;
    }
    
    // reconfigure A and B params
    if (slow != params[SLOW_PARAM].getValue()) {
      action[0] = action[1] = action[2] = action[3] = -1;
      slow = params[SLOW_PARAM].getValue();
    }
    bool hi = true;
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
            aq->displayOffset = 0.f;
            bq->unit = "";
            bq->displayBase = 0.f;
            bq->displayMultiplier = 0.5f;
            bq->displayOffset = 0.5f;
            break;
          case 1: // HOLD : A = Level, B = time
            aq->unit = "%";
            aq->displayBase = 0.f;
            aq->displayMultiplier = 25.f;
            aq->displayOffset = 75.f;
            bq->unit = " s";
            bq->displayBase = 10.f;
            bq->displayMultiplier = slow ? 10.f : 1.f;
            bq->displayOffset = 0.f;
            break;
          case 2: // SUST : A = Level, B = drift
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
      if (action[i]==2)
        params[MODE_PARAM+i*STAGE_PARAMS].setValue(2);
      up[i] = (action[i]==0 && (i==3 || action[i+1]==0)) ? hi : false;
      down[i] = (action[i]==0 && (i==3 || action[i+1]==0)) ? !hi : false;
      hi = (action[i]==0 && (i==3 || action[i+1]==0)) ? !hi : hi;
    }
  }
  
};

struct EnvelopeWidget : VenomWidget {

  int action[4] {-1, -1, -1, -1};
  
  std::shared_ptr<window::Svg> labels[4][3];
  
  struct LabelWidget : FramebufferWidget {
    SvgWidget *sw;
    int currentAction = -1;
    int currentTheme = -1;

    LabelWidget(Vec vec) {
      setPosition(vec);
      sw = new SvgWidget;
      addChild(sw);
    }
    
    void setLabel(std::shared_ptr<window::Svg> svg, int theme, int action) {
      sw->setSvg(svg);
      box.size = sw->box.size;
      currentAction = action;
      currentTheme = theme;
      setDirty();
    }

    void  draw (const DrawArgs &args) override {
      oversample = APP->window->pixelRatio<2.0 ? 2.0 : 1.0;
      FramebufferWidget::draw(args);
    }
  };
  
  LabelWidget* labelWidget[4]{};

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

  EnvelopeWidget(Envelope* module) {
    setModule(module);
    setVenomPanel("Envelope");
    
    std::string labelNames[3] {"EnvelopeMoveLabels", "EnvelopeHoldLabels", "EnvelopeSustLabels"};
    if (module) {
      for (int i=0; i<4; i++) {
        for (int j=0; j<3; j++) {
          labels[i][j] = Svg::load(asset::plugin(pluginInstance,faceplatePath(labelNames[j], themes[i])));
        }
      }
    }
    else {
      labels[0][0] = Svg::load(asset::plugin(pluginInstance,faceplatePath(labelNames[0], themes[getDefaultTheme()])));
      labels[1][0] = Svg::load(asset::plugin(pluginInstance,faceplatePath(labelNames[0], themes[getDefaultDarkTheme()])));
    }
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(22.5f,108.f), module, Envelope::SLOW_PARAM, Envelope::SLOW_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(22.5f,150.f), module, Envelope::FROM0_PARAM, Envelope::FROM0_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteLight>>>(Vec(22.5,185.5f), module, Envelope::GATE_IN_PARAM, Envelope::GATE_IN_LIGHT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 218.5f), module, Envelope::GATE_INPUT));
    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(22.5f,241.f), module, Envelope::IDLE_LIGHT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5, 266.5f), module, Envelope::IDLE_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5, 304.5f), module, Envelope::INV_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5, 342.5f), module, Envelope::ENV_OUTPUT));
    for (int i=0; i<4; i++) {
      addChild((labelWidget[i] = new LabelWidget(Vec(45.f+i*45, 0.f))));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(67.5f+i*45, 23.5f), module, Envelope::UP_LIGHT+i*STAGE_LIGHTS));
      addParam(createLockableParamCentered<ActionSwitch>(Vec(67.5f+i*45, 40.f), module, Envelope::ACTION_PARAM+i*STAGE_PARAMS));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(67.5f+i*45, 56.5f), module, Envelope::DOWN_LIGHT+i*STAGE_LIGHTS));
      addParam(createLockableParamCentered<ModeSwitch>(Vec(67.5f+i*45, 73.f), module, Envelope::MODE_PARAM+i*STAGE_PARAMS));
      addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(67.5f+i*45, 114.5f), module, Envelope::A_PARAM+i*STAGE_PARAMS));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(67.5f+i*45, 150.5f), module, Envelope::A_CV_PARAM+i*STAGE_PARAMS));
      addInput(createInputCentered<PolyPort>(Vec(67.5f+i*45, 184.f), module, Envelope::A_CV_INPUT+i*STAGE_INPUTS));
      addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(67.5f+i*45, 225.f), module, Envelope::B_PARAM+i*STAGE_PARAMS));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(67.5f+i*45, 261.f), module, Envelope::B_CV_PARAM+i*STAGE_PARAMS));
      addInput(createInputCentered<PolyPort>(Vec(67.5f+i*45, 294.5f), module, Envelope::B_CV_INPUT+i*STAGE_INPUTS));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(67.5f+i*45, 317.f), module, Envelope::GATE_LIGHT+i*STAGE_LIGHTS));
      addOutput(createOutputCentered<PolyPort>(Vec(67.5f+i*45, 342.5f), module, Envelope::GATE_OUTPUT+i*STAGE_OUTPUTS));
    }
  }

  void step() override {
    VenomWidget::step();
    if(this->module) {
      Envelope* mod = static_cast<Envelope*>(this->module);
      for (int i=0; i<4; i++) { // reconfigure A and B names
        if (action[i] != mod->action[i]) {
          action[i] = mod->action[i];
          std::string prefix = "Stage " + std::to_string(i+1);
          switch (action[i]) {
            case 0: // MOVE
              mod->setParamFactoryName(Envelope::A_PARAM+i*STAGE_PARAMS, prefix + " move time");
              mod->setParamFactoryName(Envelope::A_CV_PARAM+i*STAGE_PARAMS, prefix + " move time CV");
              mod->setPortFactoryName(Envelope::A_CV_INPUT+i*STAGE_INPUTS, prefix + " move time CV");
              mod->setParamFactoryName(Envelope::B_PARAM+i*STAGE_PARAMS, prefix + " move shape");
              mod->setParamFactoryName(Envelope::B_CV_PARAM+i*STAGE_PARAMS, prefix + " move shape CV");
              mod->setPortFactoryName(Envelope::B_CV_INPUT+i*STAGE_INPUTS, prefix + " move shape CV");
              break;
            case 1: // HOLD
              mod->setParamFactoryName(Envelope::A_PARAM+i*STAGE_PARAMS, prefix + " hold level");
              mod->setParamFactoryName(Envelope::A_CV_PARAM+i*STAGE_PARAMS, prefix + " hold level CV");
              mod->setPortFactoryName(Envelope::A_CV_INPUT+i*STAGE_INPUTS, prefix + " hold level CV");
              mod->setParamFactoryName(Envelope::B_PARAM+i*STAGE_PARAMS, prefix + " hold time");
              mod->setParamFactoryName(Envelope::B_CV_PARAM+i*STAGE_PARAMS, prefix + " hold time CV");
              mod->setPortFactoryName(Envelope::B_CV_INPUT+i*STAGE_INPUTS, prefix + " hold time CV");
              break;
            case 2: // SUST
              mod->setParamFactoryName(Envelope::A_PARAM+i*STAGE_PARAMS, prefix + " sustain level");
              mod->setParamFactoryName(Envelope::A_CV_PARAM+i*STAGE_PARAMS, prefix + " sustain level CV");
              mod->setPortFactoryName(Envelope::A_CV_INPUT+i*STAGE_INPUTS, prefix + " sustain level CV");
              mod->setParamFactoryName(Envelope::B_PARAM+i*STAGE_PARAMS, prefix + " sustain drift");
              mod->setParamFactoryName(Envelope::B_CV_PARAM+i*STAGE_PARAMS, prefix + " sustain drift CV");
              mod->setPortFactoryName(Envelope::B_CV_INPUT+i*STAGE_INPUTS, prefix + " sustain drift CV");
              break;
          }
        }
        if (labelWidget[i]->currentAction!=action[i] || labelWidget[i]->currentTheme!=currentTheme)
          labelWidget[i]->setLabel(labels[currentTheme][action[i]], currentTheme, action[i]);
        mod->lights[Envelope::UP_LIGHT+i*STAGE_LIGHTS].setBrightness(mod->up[i]);
        mod->lights[Envelope::DOWN_LIGHT+i*STAGE_LIGHTS].setBrightness(mod->down[i]);
      }
      mod->lights[Envelope::SLOW_LIGHT].setBrightness(mod->params[Envelope::SLOW_PARAM].getValue());
      mod->lights[Envelope::FROM0_LIGHT].setBrightness(mod->params[Envelope::FROM0_PARAM].getValue());
      mod->lights[Envelope::GATE_IN_LIGHT].setBrightness(mod->params[Envelope::GATE_IN_PARAM].getValue());
    }
    else for (int i=0; i<4; i++) {
      labelWidget[i]->setLabel(labels[settings::preferDarkPanels?1:0][0], 0, 0);
    }
  }
};

}

Model* modelVenomEnvelope = createModel<Venom::Envelope, Venom::EnvelopeWidget>("Envelope");

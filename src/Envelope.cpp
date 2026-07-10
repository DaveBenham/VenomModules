// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "EnvelopeModule.hpp"
#include "math.hpp"

namespace Venom {

struct Envelope : EnvelopeModule {

  struct Stage {
    EnvelopeModule *mod = NULL;
    int action = -1;
    int mode = 0;
  };
  Stage stages[20]{};
  int stageCnt = 4,
      oldChannels = 0,
      stage[16]{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  double phase[16],
         timeFactors[3]{1.,10.,100.};
  float oldRetrig = 0.f,
        start[16]{}; // for move only
  bool reset = false,
       block = false;
  dsp::SchmittTrigger gateTrig[16],
                      retrigTrig[16];
  dsp::BooleanTrigger retrigButtonTrig;
  dsp::ClockDivider lightDivider;

  Envelope() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(SLOW_PARAM, 0.f, 2.f, 0.f, "Knob time range", {"Fast 0.001 - 10 s", "Slow 0.01 - 100 s", "Glacial 0.1 - 1000 s"});
    configSwitch<FixedSwitchQuantity>(FROM0_PARAM, 0.f, 1.f, 0.f, "Retrigger from 0", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(GATE_MODE_PARAM, 0.f, 1.f, 0.f, "Manual gate mode", {"Momentary", "Toggle"});
    configSwitch<FixedSwitchQuantity>(RETRIG_MODE_PARAM, 0.f, 2.f, 0.f, "Retrigger input mode", {"Schmitt trigger leading edge", "CV change start", "CV change end"});
    configSwitch<FixedSwitchQuantity>(GATE_IN_PARAM, 0.f, 1.f, 0.f, "Manual gate", {"Low", "High"});
    configSwitch<FixedSwitchQuantity>(RETRIG_PARAM, 0.f, 1.f, 0.f, "Manual retrigger", {"Low", "High"});
    configInput(GATE_INPUT, "Gate");
    configInput(RETRIG_INPUT, "Retrigger");
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
    lightDivider.setDivision(32);
  }
  
  float aParam(int stage) {
    return stage<4 ? params[A_PARAM+stage].getValue() : stages[stage].mod->params[A_EXP_PARAM].getValue();
  }
  
  float aCV(int stage, int c) {
    return stage<4 ? inputs[A_CV_INPUT+stage].getPolyVoltage(c) * params[A_CV_PARAM+stage].getValue() : 
                     stages[stage].mod->inputs[A_CV_EXP_INPUT].getPolyVoltage(c) * stages[stage].mod->params[A_CV_EXP_PARAM].getValue();
  }
  
  float bParam(int stage) {
    return stage<4 ? params[B_PARAM+stage].getValue() : stages[stage].mod->params[B_EXP_PARAM].getValue();
  }
  
  float bCV(int stage, int c) {
    return stage<4 ? inputs[B_CV_INPUT+stage].getPolyVoltage(c) * params[B_CV_PARAM+stage].getValue() : 
                     stages[stage].mod->inputs[B_CV_EXP_INPUT].getPolyVoltage(c) * stages[stage].mod->params[B_CV_EXP_PARAM].getValue();
  }
  
  int nextUngated(int stage) {
    while (++stage<stageCnt && stages[stage].mode==2);
    return stage>=stageCnt ? -1 : stage;
  }
  
  float target(int stage, int c) {
    if (stage == -1)
      return 0.f;
    if (stages[stage].action)
      return clamp((aParam(stage)+3.f)*0.25f + aCV(stage, c)) * 10.f;
    if (stage<4 && (up[stage] || down[stage]))
      return up[stage] ? 10.f : 0.f;
    if (stage>=4 && (stages[stage].mod->up[0] || stages[stage].mod->down[0]))
      return stages[stage].mod->up[0] ? 10.f : 0.f;
    return clamp((aParam(stage+1)+3.f)*0.25f + aCV(stage+1, c)) * 10.f;
  }
  
  float gateRatio(float *v, int c) {
    float sum = 0;
    for (int i=0; i<c; i++)
      sum += v[i];
    return sum/(10.f * c);
  }
  
  void process(const ProcessArgs& args) override {
    EnvelopeModule::process(args);
    
    bool hi = true;
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++) {
      int c = inputs[i].getChannels();
      if (c > channels)
        channels = c;
    }
    int nextAction = params[ACTION_PARAM].getValue();
    for (int i=0; i<4; i++) {
      if (stages[i].action != nextAction) {
        reset = true;
        stages[i].action = nextAction;
      }
      if (nextAction==2)
        params[MODE_PARAM+i].setValue(2);
      stages[i].mode = params[MODE_PARAM+i].getValue();
      nextAction = i<3 ? params[ACTION_PARAM+i+1].getValue() : (expander ? expander->params[ACTION_EXP_PARAM].getValue() : 0);
      up[i] = (stages[i].action==0 && nextAction==0) ? hi : false;
      down[i] = (stages[i].action==0 && nextAction==0) ? !hi : false;
      hi = (stages[i].action==0 && nextAction==0) ? !hi : hi;
    }
    EnvelopeModule *curExpander = expander;
    int newStageCnt = 4;
    block = false;
    while(curExpander && newStageCnt<20) {
      if (curExpander->isBypassed())
        reset = block = true;
      for (int i=0; i<EXP_INPUTS_LEN; i++) {
        int c = curExpander->inputs[i].getChannels();
        if (c > channels)
          channels = c;
      }
      if (stages[newStageCnt].mod != curExpander || stages[newStageCnt].action != nextAction) {
        reset = true;
        stages[newStageCnt].mod = curExpander;
        stages[newStageCnt].action = nextAction;
      }
      if (nextAction==2)
        curExpander->params[MODE_EXP_PARAM].setValue(2);
      stages[newStageCnt].mode = curExpander->params[MODE_EXP_PARAM].getValue();
      nextAction = curExpander->expander ? curExpander->expander->params[ACTION_EXP_PARAM].getValue() : 0;
      curExpander->up[0] = (stages[newStageCnt].action==0 && nextAction==0) ? hi : false;
      curExpander->down[0] = (stages[newStageCnt].action==0 && nextAction==0) ? !hi : false;
      hi = (stages[newStageCnt].action==0 && nextAction==0) ? !hi : hi;
      curExpander = curExpander->expander;
      newStageCnt++;
    }
    if (stageCnt != newStageCnt)
      reset = true;
    while (stageCnt > newStageCnt) {
      stages[--stageCnt].mod = NULL;
      stages[stageCnt].action = -1;
    }
    stageCnt = newStageCnt;
    while (oldChannels > channels) {
      gateTrig[--oldChannels].process(0);
      retrigTrig[oldChannels].process(0);
    }
    if (block) {
      for (int i=0; i<OUTPUTS_LEN; i++) {
        outputs[i].setVoltage(0.f);
        outputs[i].setChannels(1);
      }
      lights[IDLE_LIGHT].setBrightness(0);
      for (int i=0; i<4; i++)
        lights[GATE_LIGHT+i].setBrightness(0);
      for (int i=4; i<stageCnt; i++) {
        stages[i].mod->outputs[GATE_EXP_OUTPUT].setVoltage(0.f);
        stages[i].mod->outputs[GATE_EXP_OUTPUT].setChannels(1);
        stages[i].mod->lights[GATE_EXP_LIGHT].setBrightness(0);
      }
    }
    else {
      bool buttonRetrig = retrigButtonTrig.process(params[RETRIG_PARAM].getValue()),
           from0 = params[FROM0_PARAM].getValue();
      int retrigMode = params[RETRIG_MODE_PARAM].getValue();
      double timeFactor = timeFactors[static_cast<int>(params[SLOW_PARAM].getValue())] * args.sampleRate;
      for (int c=0; c<channels; c++) {
        if (reset) {
          stage[c] = -1;
          phase[c] = 0.;
        }
        bool cvRetrig = retrigMode==0 ? retrigTrig[c].process(inputs[RETRIG_INPUT].getPolyVoltage(c), 0.2f, 2.f) :
                        (retrigTrig[c].processEvent(inputs[RETRIG_INPUT].getPolyVoltage(c)!=oldRetrig)==(retrigMode==1?1:-1)),
             trig = gateTrig[c].process(buttonRetrig || cvRetrig ? 0.f : params[GATE_IN_PARAM].getValue()*10.f + inputs[GATE_INPUT].getPolyVoltage(c), 0.2f, 2.f),
             gate = gateTrig[c].isHigh();
        oldRetrig = inputs[RETRIG_INPUT].getPolyVoltage(c);
        int action = stage[c]==-1 ? 0 : stages[stage[c]].action,
            mode = stage[c]==-1 ? 1 : stages[stage[c]].mode;
        if (stage[c] == -1) { // idle on entry
          outputs[IDLE_OUTPUT].setVoltage(10.f, c);
          outputs[ENV_OUTPUT].setVoltage(0.f, c);
        }  
        else {
          outputs[IDLE_OUTPUT].setVoltage(0.f, c);
          switch (action) {
            case 0: // move
              {
                float trgt = (stage[c]<4 ? up[stage[c]] : stages[stage[c]].mod->up[0]) ? 10.f :
                             (stage[c]<4 ? down[stage[c]] : stages[stage[c]].mod->down[0]) ? 0.f :
                             clamp((aParam(stage[c]+1)+3.f)*0.25f + aCV(stage[c]+1, c)) * 10.f;
                float shape = clamp((bParam(stage[c]) * 0.5f + 0.5) + bCV(stage[c], c), -1.f, 1.f) * 0.95f;
                if (phase[c] == 0.) {
                  float crnt = outputs[ENV_OUTPUT].getVoltage(c);
                  if (stage[c]>0) {
                    int prv = stage[c]-1;
                    start[c] = stages[prv].action>0 ?
                      clamp((aParam(prv)+3.f)*0.25f + aCV(prv, c)) * 10.f :
                      (prv<4 ? up[prv] : stages[prv].mod->up[0]) ? 10.f : 0.f;
                  }
                  else
                    start[c] = 0.f;
                  if (stage[c]>0 || !from0) {
                    if (start[c] < trgt) {
                      if (crnt < start[c])
                        start[c] = crnt;
                      else if (crnt >= trgt )
                        phase[c] = 1.;
                      else
                        phase[c] = static_cast<double>(crnt - start[c]) / static_cast<double>(trgt - start[c]);
                    }
                    else {
                      if (crnt > start[c])
                        start[c] = crnt;
                      else if (crnt <= trgt || start[c]==trgt)
                        phase[c] = 1.;
                      else
                        phase[c] = static_cast<double>(crnt - start[c]) / static_cast<double>(trgt - start[c]);
                    }
                    phase[c] = invNormSigmoid(phase[c], shape);
                  }  
                }
                phase[c] += 1./(std::pow(10., static_cast<double>(aParam(stage[c]))) * timeFactor * std::pow(2., static_cast<double>(aCV(stage[c], c))*10.));
                if (phase[c] > 1.)
                  phase[c] = 1.;
                outputs[ENV_OUTPUT].setVoltage(normSigmoid(phase[c], shape)*(trgt - start[c]) + start[c], c);
                if (mode==2 && !gate) {
                  phase[c] = 0.;
                  stage[c] = nextUngated(stage[c]);
                }
                else if (phase[c] >= 1.) {
                  phase[c] = 0.;
                  start[c] = -1.f;
                  if (++stage[c] >= stageCnt)
                    stage[c] = -1;
                }
              }
              break;
            case 1: // hold
              outputs[ENV_OUTPUT].setVoltage(clamp((aParam(stage[c])+3.f)*0.25f + aCV(stage[c], c)) * 10.f, c);
              if (mode==2 && !gate) {
                phase[c] = 0.;
                stage[c] = nextUngated(stage[c]);
              }
              else {
                phase[c] += bParam(stage[c])==-3 ? 1.f : 1/(std::pow(10., bParam(stage[c])) * timeFactor * std::pow(2., bCV(stage[c], c)*10.f));
                if (phase[c] >= 1.) {
                  phase[c] = 0.;
                  if (++stage[c] >= stageCnt)
                    stage[c] = -1;
                }
              }
              break;
            case 2: // sustain
              {
                int nextStage = nextUngated(stage[c]);
                double drift = (bParam(stage[c])==-3.f ? 0. : std::pow(10., bParam(stage[c]))) + bCV(stage[c], c);
                if (drift<0.001f || phase[c]==0.)
                  outputs[ENV_OUTPUT].setVoltage(clamp((aParam(stage[c])+3.f)*0.25f + aCV(stage[c], c)) * 10.f, c);
                else {
                  float cur = outputs[ENV_OUTPUT].getVoltage(c);
                  float nextTarget = target(nextStage, c);
                  drift *= args.sampleTime;
                  if (nextTarget > cur) {
                    cur += drift;
                    if (cur > nextTarget)
                      cur = nextTarget;
                  }
                  if (nextTarget < cur) {
                    cur -= drift;
                    if (cur < nextTarget)
                      cur = nextTarget;
                  }
                  outputs[ENV_OUTPUT].setVoltage(cur, c); 
                }
                if (gate)
                  phase[c]=0.5;
                else {
                  phase[c] = 0.;
                  stage[c] = nextStage;
                }
              }
              break;
          }
        }
        if (trig && mode==1) { // (re)trigger new envelope
          stage[c] = 0;
          phase[c] = 0.;
        }
        outputs[INV_OUTPUT].setVoltage(10.f - outputs[ENV_OUTPUT].getVoltage(c), c);
        for (int i=0; i<4; i++)
          outputs[GATE_OUTPUT+i].setVoltage(stage[c]==i ? 10.f : 0.f, c);
        for (int i=4; i<stageCnt; i++)
          stages[i].mod->outputs[GATE_EXP_OUTPUT].setVoltage(stage[c]==i ? 10.f : 0.f, c);
      }
      for (int i=0; i<OUTPUTS_LEN; i++)
        outputs[i].setChannels(channels);
      for (int i=4; i<stageCnt; i++)
        stages[i].mod->outputs[GATE_EXP_OUTPUT].setChannels(channels);
      if (lightDivider.process()) {
        lights[IDLE_LIGHT].setBrightnessSmooth(gateRatio(outputs[IDLE_OUTPUT].getVoltages(), channels), args.sampleTime*32);
        for (int i=0; i<4; i++)
          lights[GATE_LIGHT+i].setBrightnessSmooth(gateRatio(outputs[GATE_OUTPUT+i].getVoltages(), channels), args.sampleTime*32);
        for (int i=4; i<stageCnt; i++)
          stages[i].mod->lights[GATE_EXP_LIGHT].setBrightnessSmooth(gateRatio(stages[i].mod->outputs[GATE_EXP_OUTPUT].getVoltages(), channels), args.sampleTime*32);
      }
      reset = false;
    }
  }
  
  void processBypass(const ProcessArgs &args) override {
    for (EnvelopeModule *exp=expander; exp; exp=exp->expander) {
      exp->outputs[GATE_EXP_OUTPUT].setVoltage(0.f);
      exp->outputs[GATE_EXP_OUTPUT].setChannels(0);
      exp->lights[GATE_EXP_LIGHT].setBrightness(0);
    }
    EnvelopeModule::processBypass(args);
  }
  
  void onUnBypass(const UnBypassEvent &e) override {
    reset = true;
  }

  
};

struct EnvelopeWidget : EnvelopeModuleWidget {
  int action[4]{-1,-1,-1,-1};
  int slow=0;
  VCVLightBezelLockable<MediumSimpleLight<WhiteLight>> *manualGate = NULL;

  struct SlowSwitch : GlowingSvgSwitchLockable {
    SlowSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
    }
  };

  struct OnOffSwitch : GlowingSvgSwitchLockable {
    OnOffSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
    }
  };

  struct TriSwitch : GlowingSvgSwitchLockable {
    TriSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
    }
  };

  EnvelopeWidget(Envelope* module) {
    setModule(module);
    setVenomPanel("Envelope");

    addParam(createLockableParamCentered<SlowSwitch>(Vec(17.5f, 111.5f), module, EnvelopeModule::SLOW_PARAM));
    addParam(createLockableParamCentered<OnOffSwitch>(Vec(47.5f, 111.5f), module, EnvelopeModule::FROM0_PARAM));
    addParam(createLockableParamCentered<OnOffSwitch>(Vec(17.5f, 145.5f), module, EnvelopeModule::GATE_MODE_PARAM));
    addParam(createLockableParamCentered<TriSwitch>(Vec(47.5f, 145.5f), module, EnvelopeModule::RETRIG_MODE_PARAM));
    manualGate = createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteLight>>>(Vec(17.5,179.5f), module, EnvelopeModule::GATE_IN_PARAM, EnvelopeModule::GATE_IN_LIGHT);
    addParam(manualGate);
    addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteLight>>>(Vec(47.5,179.5f), module, EnvelopeModule::RETRIG_PARAM, EnvelopeModule::RETRIG_LIGHT));
    addInput(createInputCentered<PolyPort>(Vec(17.5f, 212.5f), module, EnvelopeModule::GATE_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(47.5f, 212.5f), module, EnvelopeModule::RETRIG_INPUT));
    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(32.5f,241.f), module, EnvelopeModule::IDLE_LIGHT));
    addOutput(createOutputCentered<PolyPort>(Vec(32.5, 266.5f), module, EnvelopeModule::IDLE_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(32.5, 304.5f), module, EnvelopeModule::INV_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(32.5, 342.5f), module, EnvelopeModule::ENV_OUTPUT));
    for (int i=0; i<4; i++) {
      addChild((labelWidget[i] = new LabelWidget(Vec(60.f+i*45, 0.f))));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(82.5f+i*45, 23.5f), module, EnvelopeModule::UP_LIGHT+i));
      addParam(createLockableParamCentered<ActionSwitch>(Vec(82.5f+i*45, 40.f), module, EnvelopeModule::ACTION_PARAM+i));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(82.5f+i*45, 56.5f), module, EnvelopeModule::DOWN_LIGHT+i));
      addParam(createLockableParamCentered<ModeSwitch>(Vec(82.5f+i*45, 73.f), module, EnvelopeModule::MODE_PARAM+i));
      addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(82.5f+i*45, 114.5f), module, EnvelopeModule::A_PARAM+i));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(82.5f+i*45, 150.5f), module, EnvelopeModule::A_CV_PARAM+i));
      addInput(createInputCentered<PolyPort>(Vec(82.5f+i*45, 184.f), module, EnvelopeModule::A_CV_INPUT+i));
      addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(82.5f+i*45, 225.f), module, EnvelopeModule::B_PARAM+i));
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(82.5f+i*45, 261.f), module, EnvelopeModule::B_CV_PARAM+i));
      addInput(createInputCentered<PolyPort>(Vec(82.5f+i*45, 294.5f), module, EnvelopeModule::B_CV_INPUT+i));
      addChild(createLightCentered<SmallLight<YellowLight>>(Vec(82.5f+i*45, 317.f), module, EnvelopeModule::GATE_LIGHT+i));
      addOutput(createOutputCentered<PolyPort>(Vec(82.5f+i*45, 342.5f), module, EnvelopeModule::GATE_OUTPUT+i));
    }
  }

  void appendContextMenu(Menu* menu) override {
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Add Stage expander", "", [this](){addExpander(modelVenomEnvelopeExpander,this);}));
    VenomWidget::appendContextMenu(menu);
  }

  void step() override {
    EnvelopeModuleWidget::step();
    if(this->module) {
      Envelope* mod = static_cast<Envelope*>(this->module);
      if (static_cast<bool>(mod->params[EnvelopeModule::GATE_MODE_PARAM].getValue()) != manualGate->latch) {
        manualGate->momentary = !manualGate->momentary;
        manualGate->latch = !manualGate->latch;
      }
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
              aq->displayMultiplier = newSlow==2 ? 100.f : newSlow ? 10.f : 1.f;
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
              bq->displayMultiplier = newSlow==2 ? 100.f : newSlow ? 10.f : 1.f;
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
              bq->displayMultiplier = 1.f;
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
      mod->lights[Envelope::GATE_IN_LIGHT].setBrightness(mod->params[Envelope::GATE_IN_PARAM].getValue());
      mod->lights[Envelope::RETRIG_LIGHT].setBrightness(mod->params[Envelope::RETRIG_PARAM].getValue());
    }
    else for (int i=0; i<4; i++) {
      if (labelWidget[i])
        labelWidget[i]->setLabel(settings::preferDarkPanels ? darkTheme : lightTheme, 0);
    }
  }

};

}

Model* modelVenomEnvelope = createModel<Venom::Envelope, Venom::EnvelopeWidget>("Envelope");

// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "math.hpp"

#define MAX_STAGES 20
namespace Venom {

struct EnvelopeFactory : VenomModule {

  enum ParamId {
    AMP_PARAM,
    OFF_PARAM,
    SLOW_PARAM,
    FROM0_PARAM,
    GATE_MODE_PARAM,
    RETRIG_MODE_PARAM,
    GATE_IN_PARAM,
    RETRIG_PARAM,
    IDLE_PARAM,
    ENUMS(ACTION_PARAM,MAX_STAGES),
    ENUMS(MODE_PARAM,MAX_STAGES),
    ENUMS(A_PARAM,MAX_STAGES),
    ENUMS(A_CV_PARAM,MAX_STAGES),
    ENUMS(B_PARAM,MAX_STAGES),
    ENUMS(B_CV_PARAM,MAX_STAGES),
    ENUMS(TRIG_PARAM,MAX_STAGES),
    PARAMS_LEN
  };
  enum InputId {
    VOCT_INPUT,
    AMP_INPUT,
    OFF_INPUT,
    GATE_INPUT,
    RETRIG_INPUT,
    ENUMS(A_CV_INPUT,MAX_STAGES),
    ENUMS(B_CV_INPUT,MAX_STAGES),
    INPUTS_LEN
  };
  enum OutputId {
    TRIGS_OUTPUT,
    IDLE_OUTPUT,
    ENV_OUTPUT,
    INV_OUTPUT,
    ENUMS(GATE_OUTPUT,MAX_STAGES),
    OUTPUTS_LEN
  };
  enum LightId {
    GATE_IN_LIGHT,
    RETRIG_LIGHT,
    IDLE_LIGHT,
    ENUMS(UP_LIGHT,MAX_STAGES),
    ENUMS(DOWN_LIGHT,MAX_STAGES),
    ENUMS(GATE_LIGHT,MAX_STAGES),
    LIGHTS_LEN
  };

  struct Stage {
    int action = -1;
    bool up = false,
         down = false;
  };
  Stage stages[MAX_STAGES]{};
  int stageCnt = 4,
      oldChannels = 0,
      stage[16]{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  double phase[16]{},
         driftVoltage[16]{}, // for sustain only
         timeFactors[4]{1.,10.,100.,1000.};
  float env[16]{},
        oldRetrig[16]{},
        multiplier[16]{}, // for move only
        start[16]{}, // for move only
        curInput[16]{},
        velocity[16]{};
  int vcaMode = 0,
      eocPrimed = 0;
  bool reset = false,
       pendTrig[16]{},
       randTimes = true,
       randLevels = true,
       randDrifts = true,
       randShapes = true,
       randAttens = true,
       randTrigs = true;
  dsp::SchmittTrigger gateTrig[16],
                      retrigTrig[16];
  dsp::BooleanTrigger retrigButtonTrig;
  dsp::PulseGenerator outTrig[16];                      
  dsp::ClockDivider lightDivider;

  struct BQuantity : ParamQuantity {
    int action = 0;
    float getDisplayValue() override {
      return (action==1 || action==2) && getValue()==-3.f ? 0.f : ParamQuantity::getDisplayValue();
    }
  };
  
  EnvelopeFactory() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configInput(VOCT_INPUT, "V/Oct");
    configParam(AMP_PARAM, -1.f, 1.f, 1.f, "Amplitude", "%", 0.f, 100.f, 0.f);
    configParam(OFF_PARAM, -1.f, 1.f, 0.f, "Offset", "%", 0.f, 100.f, 0.f);
    configInput(AMP_INPUT, "Amplitude");
    configInput(OFF_INPUT, "Offset");
    configSwitch<FixedSwitchQuantity>(SLOW_PARAM, 0.f, 3.f, 0.f, "Knob time range", {"Fast 0.001 - 10 s", "Slow 0.01 - 100 s", "Crawl 0.1 - 1000 s", "Glacial 1 - 10000 s"});
    configSwitch<FixedSwitchQuantity>(FROM0_PARAM, 0.f, 1.f, 0.f, "Retrigger from 0", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(GATE_MODE_PARAM, 0.f, 1.f, 0.f, "Manual gate mode", {"Momentary", "Toggle"});
    configSwitch<FixedSwitchQuantity>(RETRIG_MODE_PARAM, 0.f, 2.f, 0.f, "Retrigger input mode", {"Schmitt trigger leading edge", "CV change start", "CV change end"});
    configSwitch<FixedSwitchQuantity>(GATE_IN_PARAM, 0.f, 1.f, 0.f, "Manual gate", {"Low", "High"});
    configSwitch<FixedSwitchQuantity>(RETRIG_PARAM, 0.f, 1.f, 0.f, "Manual retrigger", {"Low", "High"});
    configInput(GATE_INPUT, "Gate");
    configInput(RETRIG_INPUT, "Retrigger");
    configOutput(TRIGS_OUTPUT, "Stage triggers");
    configSwitch<FixedSwitchQuantity>(IDLE_PARAM, 0.f, 1.f, 1.f, "EOC trigger switch", {"Off", "On"});
    configLight(IDLE_LIGHT, "Idle indicator");
    configOutput(IDLE_OUTPUT, "Idle gate");
    configOutput(ENV_OUTPUT, "Envelope");
    configOutput(INV_OUTPUT, "Inverse envelope");

    for (int i=0; i<MAX_STAGES; i++) {
      std::string prefix = "Stage " + std::to_string(i+1);
      configLight(UP_LIGHT+i, prefix + " 10V target indicator");
      configSwitch<FixedSwitchQuantity>(ACTION_PARAM+i, 0.f, 4.f, 0.f, prefix+" action", {"Move", "Hold", "Sustain", "Rise", "Fall"});
      configLight(DOWN_LIGHT+i, prefix+" 0V target indicator");
      configSwitch<FixedSwitchQuantity>(MODE_PARAM+i, 0.f, 2.f, 0.f, prefix+" mode", {"Full", "Retriggerable Full", "Gate"});
      configParam(A_PARAM+i, -3.f, 1.f, -1.f, prefix+" move time", " s", 10.f, 1.f, 0.f);
      configParam(A_CV_PARAM+i, -0.1f, 0.1f, 0.f, prefix+" move time CV", "%", 0.f, 1000.f, 0.f);
      configInput(A_CV_INPUT+i, prefix+" move time CV");
      configParam<BQuantity>(B_PARAM+i, -3.f, 1.f, -1.f, prefix+" move shape", "", 0.f, 0.5f, 0.5f);
      configParam(B_CV_PARAM+i, -0.1f, 0.1f, 0.f, prefix+" move shape CV", "%", 0.f, 1000.f, 0.f);
      configInput(B_CV_INPUT+i, prefix+" move shape CV");
      configSwitch<FixedSwitchQuantity>(TRIG_PARAM+i, 0.f, 1.f, 0.f, prefix+" trigger switch", {"Off", "On"});
      configLight(GATE_LIGHT+i, prefix+" gate indicator");
      configOutput(GATE_OUTPUT+i, prefix+" gate");
    }
    params[MODE_PARAM+0].setValue(2);
    params[MODE_PARAM+1].setValue(2);
    params[ACTION_PARAM+2].setValue(2);
    params[MODE_PARAM+2].setValue(2);
    params[B_PARAM+2].setValue(-3.f);
    params[MODE_PARAM+3].setValue(1);
    for (int i=0; i<=IDLE_PARAM; i++)
      paramQuantities[i]->randomizeEnabled = false;
    for (int i=0; i<MAX_STAGES; i++) {
      paramQuantities[ACTION_PARAM+i]->randomizeEnabled = false;
      paramQuantities[MODE_PARAM+i]->randomizeEnabled = false;
    }
    lightDivider.setDivision(32);
  }
  
  float aParam(int stage) {
    return params[A_PARAM+stage].getValue();
  }
  
  float aCV(int stage, int c) {
    return inputs[A_CV_INPUT+stage].getPolyVoltage(c) * params[A_CV_PARAM+stage].getValue();
  }
  
  float bParam(int stage) {
    return params[B_PARAM+stage].getValue();
  }
  
  float bCV(int stage, int c) {
    return inputs[B_CV_INPUT+stage].getPolyVoltage(c) * params[B_CV_PARAM+stage].getValue();
  }
  
  int nextUngated(int stage) {
    while (++stage<stageCnt && params[MODE_PARAM+stage].getValue()==2.f);
    return stage>=stageCnt ? -1 : stage;
  }
  
  float target(int stage, int c) {
    if (stage == -1)
      return 0.f;
    if (stages[stage].action==1 || stages[stage].action==2)
      return clamp((aParam(stage)+3.f)*0.25f + aCV(stage, c));
    if (stages[stage].up)
      return 1.f;
    if (stages[stage].down)
      return 0.f;
    return clamp((aParam(stage+1)+3.f)*0.25f + aCV(stage+1, c));
  }
  
  float gateRatio(float *v, int c) {
    float sum = 0;
    for (int i=0; i<c; i++)
      sum += v[i];
    return sum/(10.f * c);
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    bool hi = true;
    int channels = 1;
    for (int i=0; i<INPUTS_LEN; i++) {
      int c = inputs[i].getChannels();
      if (c > channels)
        channels = c;
    }
    int nextAction = params[ACTION_PARAM].getValue();
    for (int i=0; i<stageCnt; i++) {
      if (stages[i].action != nextAction) {
        reset = true;
        stages[i].action = nextAction;
      }
      nextAction = i<stageCnt-1 ? params[ACTION_PARAM+i+1].getValue() : 0;
      switch (stages[i].action) {
        case 0:
          stages[i].up = (nextAction==0 || nextAction>2) ? hi : false;
          stages[i].down = (nextAction==0 || nextAction>2) ? !hi : false;
          hi = (stages[i].action==0 && (nextAction==0 || nextAction>2)) ? !hi : hi;
          break;
        case 3:
          stages[i].up = true;
          stages[i].down = false;
          hi = false;
          break;
        case 4:
          stages[i].up = false;
          stages[i].down = true;
          hi = true;
          break;
        default:
          stages[i].up = stages[i].down = false;
      }
    }
    while (oldChannels > channels) {
      gateTrig[--oldChannels].process(0);
      retrigTrig[oldChannels].process(0);
      env[oldChannels] = 0.f;
      outTrig[oldChannels].reset();
      pendTrig[oldChannels] = false;
    }
    bool buttonRetrig = retrigButtonTrig.process(params[RETRIG_PARAM].getValue()),
         from0 = params[FROM0_PARAM].getValue();
    int retrigMode = params[RETRIG_MODE_PARAM].getValue();
    double timeFactor = timeFactors[static_cast<int>(params[SLOW_PARAM].getValue())] * args.sampleRate;
    for (int c=0; c<channels; c++) {
      if (reset) {
        stage[c] = -1;
        phase[c] = 0.;
        env[c] = 0.f;
        outTrig[c].reset();
        pendTrig[c] = false;
        eocPrimed = 0;
      }
      bool cvRetrig = retrigMode==0 ? retrigTrig[c].process(inputs[RETRIG_INPUT].getPolyVoltage(c), 0.2f, 2.f) :
                      (retrigTrig[c].processEvent(inputs[RETRIG_INPUT].getPolyVoltage(c)!=oldRetrig[c])==(retrigMode==1?1:-1)),
           trig = gateTrig[c].process(buttonRetrig || cvRetrig ? 0.f : params[GATE_IN_PARAM].getValue()*10.f + inputs[GATE_INPUT].getPolyVoltage(c), 0.2f, 2.f),
           gate = gateTrig[c].isHigh();
      oldRetrig[c] = inputs[RETRIG_INPUT].getPolyVoltage(c);
      if (vcaMode) { 
        float newInput = inputs[AMP_INPUT].getPolyVoltage(c);
        if (vcaMode == 2) { // delay trig until VCA input crosses 0
          if (trig) {
            pendTrig[c] = true;
            trig = false;
          }
          if (pendTrig[c] && (((newInput>0.f)!=(curInput[c]>0.f)) || newInput==0.f)) {
            trig = true;
            pendTrig[c] = false;
            velocity[c] = normSigmoid(clamp(inputs[OFF_INPUT].getNormalPolyVoltage(10.f, c)/10.f), -params[OFF_PARAM].getValue()*0.97f);
          }
        }  
        else
          velocity[c] = normSigmoid(clamp(inputs[OFF_INPUT].getNormalPolyVoltage(10.f, c)/10.f), -params[OFF_PARAM].getValue()*0.97f);
        curInput[c] = inputs[AMP_INPUT].getPolyVoltage(c);
      }
      int action = stage[c]==-1 ? 0 : stages[stage[c]].action,
          mode = stage[c]==-1 ? 1 : params[MODE_PARAM+stage[c]].getValue();
      outTrig[c].process(args.sampleTime);    
      if (stage[c] == -1) { // idle on entry
        outputs[IDLE_OUTPUT].setVoltage(10.f, c);
        if (!eocPrimed && (!trig || from0 || vcaMode==2))
          env[c] = 0.f;
        if (eocPrimed) {
          if (params[IDLE_PARAM].getValue())
            outTrig[c].trigger();
          eocPrimed--;
        }
      }  
      else {
        eocPrimed = 2;
        if (phase[c] == 0. && params[TRIG_PARAM+stage[c]].getValue())
          outTrig[c].trigger();
        outputs[IDLE_OUTPUT].setVoltage(0.f, c);
        switch (action) {
          case 0: // move
          case 3: // rise
          case 4: // fall
            {
              float trgt = action==3 ? 1.f :
                           action==4 ? 0.f :
                           stages[stage[c]].up ? 1.f :
                           stages[stage[c]].down ? 0.f :
                           clamp((aParam(stage[c]+1)+3.f)*0.25f + aCV(stage[c]+1, c));
              float shape = clamp((bParam(stage[c]) * 0.5f + 0.5) + bCV(stage[c], c)*2, -1.f, 1.f) * 0.95f;
              if (phase[c] == 0.) {
                multiplier[c] = 1.f;
                float crnt = action==4 && stage[c]==0 ? 1.f : env[c];
                if (stage[c]>0) {
                  int prv = stage[c]-1;
                  start[c] = (stages[prv].action==1 || stages[prv].action==2) ? clamp((aParam(prv)+3.f)*0.25f + aCV(prv, c)) :
                             stages[prv].up ? 1.f : 0.f;
                }
                else
                  start[c] = action==4 && stage[c]==0 ? 1.f : 0.f;
                if (stage[c]>0 || !from0) {
                  if (start[c] < trgt) {
                    if (crnt < start[c]) {
                      multiplier[c] = (trgt - crnt) / (trgt - start[c]);
                      start[c] = crnt;
                    }
                    else if (crnt >= trgt )
                      phase[c] = 1.;
                    else
                      phase[c] = static_cast<double>(crnt - start[c]) / static_cast<double>(trgt - start[c]);
                  }
                  else {
                    if (crnt > start[c] && start[c] > trgt) {
                      multiplier[c] = (trgt - crnt) / (trgt - start[c]);
                      start[c] = crnt;
                    }
                    else if (crnt <= trgt || start[c]==trgt)
                      phase[c] = 1.;
                    else
                      phase[c] = static_cast<double>(crnt - start[c]) / static_cast<double>(trgt - start[c]);
                  }
                  phase[c] = invNormSigmoid(phase[c], trgt>start[c]?-shape:shape);
                }  
              }
              phase[c] += 1./(std::pow(10., aParam(stage[c])) * timeFactor * multiplier[c] * std::pow(2., aCV(stage[c], c)*10. - inputs[VOCT_INPUT].getPolyVoltage(c)));
              if (phase[c] > 1.)
                phase[c] = 1.;
              env[c] = normSigmoid(phase[c], trgt>start[c]?-shape:shape)*(trgt - start[c]) + start[c];
              if (mode==2 && !gate) {
                phase[c] = 0.;
                stage[c] = nextUngated(stage[c]);
              }
              else if (phase[c] >= 1.) {
                phase[c] = 0.;
                start[c] = -1.f;
                if (++stage[c] >= stageCnt) {
                  stage[c] = -1;
                }
              }
            }
            break;
          case 1: // hold
            {
              float v = clamp((aParam(stage[c])+3.f)*0.25f + aCV(stage[c], c));
              if (from0 || v || stage[c])
                env[c] = v;
              if (mode==2 && !gate) {
                phase[c] = 0.;
                stage[c] = nextUngated(stage[c]);
              }
              else {
                phase[c] += bParam(stage[c])==-3 ? 1. : 1./(std::pow(10., bParam(stage[c])) * timeFactor * std::pow(2., bCV(stage[c], c)*10.f - inputs[VOCT_INPUT].getPolyVoltage(c)));
                if (phase[c] >= 1.) {
                  phase[c] = 0.;
                  if (++stage[c] >= stageCnt)
                    stage[c] = -1;
                }
              }
            }
            break;
          case 2: // sustain
            {
              int nextStage = mode == 2 ? nextUngated(stage[c]) : (stage[c]+1)>=stageCnt ? -1 : stage[c]+1;
              double drift = (bParam(stage[c])==-3.f ? 0. : std::pow(10., bParam(stage[c]))) + bCV(stage[c], c);
              if (phase[c]==0.) {
                float v = clamp((aParam(stage[c])+3.f)*0.25f + aCV(stage[c], c));
                driftVoltage[c] = (from0 || v || stage[c]) ? v : env[c];
              }
              if (drift<=0.000001) {
                float v = clamp((aParam(stage[c])+3.f)*0.25f + aCV(stage[c], c));
                if (from0 || v || stage[c])
                  driftVoltage[c] = v;
                env[c] = driftVoltage[c];
              }
              else {
                double nextTarget = target(nextStage, c);
                drift *= args.sampleTime;
                if (nextTarget > driftVoltage[c]) {
                  driftVoltage[c] += drift;
                  if (driftVoltage[c] > nextTarget)
                    driftVoltage[c] = nextTarget;
                }
                if (nextTarget < driftVoltage[c]) {
                  driftVoltage[c] -= drift;
                  if (driftVoltage[c] < nextTarget)
                    driftVoltage[c] = nextTarget;
                }
                env[c] = driftVoltage[c];
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
      if (vcaMode) {
        outputs[ENV_OUTPUT].setVoltage(env[c] * 10.f, c);
        outputs[INV_OUTPUT].setVoltage(env[c] * curInput[c] * (params[AMP_PARAM].getValue()+1)*0.5f * velocity[c], c);
      }
      else {
        float amp = inputs[AMP_INPUT].getNormalPolyVoltage(10.f, c) * params[AMP_PARAM].getValue();
        float off = inputs[OFF_INPUT].getNormalPolyVoltage(10.f, c) * params[OFF_PARAM].getValue();
        outputs[ENV_OUTPUT].setVoltage(env[c] * amp + off, c);
        outputs[INV_OUTPUT].setVoltage((1.f - env[c]) * amp + off, c);
      }
      outputs[TRIGS_OUTPUT].setVoltage(outTrig[c].remaining>0.f ? 10.f : 0.f, c);
      for (int i=0; i<stageCnt; i++)
        outputs[GATE_OUTPUT+i].setVoltage(stage[c]==i ? 10.f : 0.f, c);
    }   
    for (int i=0; i<OUTPUTS_LEN; i++)
      outputs[i].setChannels(channels);
    if (lightDivider.process()) {
      lights[IDLE_LIGHT].setBrightnessSmooth(gateRatio(outputs[IDLE_OUTPUT].getVoltages(), channels), args.sampleTime*32);
      for (int i=0; i<stageCnt; i++)
        lights[GATE_LIGHT+i].setBrightnessSmooth(gateRatio(outputs[GATE_OUTPUT+i].getVoltages(), channels), args.sampleTime*32);
    }
    reset = false;
  }

  void configStageRandomize(int stage) {
    int action = params[ACTION_PARAM+stage].getValue();
    switch (action) {
      case 0: // MOVE
        paramQuantities[A_PARAM+stage]->randomizeEnabled = randTimes;
        paramQuantities[B_PARAM+stage]->randomizeEnabled = randShapes;
        break;
      case 1: // HOLD
        paramQuantities[A_PARAM+stage]->randomizeEnabled = randLevels;
        paramQuantities[B_PARAM+stage]->randomizeEnabled = randTimes;
        break;
      case 2: // SUSTAIN
        paramQuantities[A_PARAM+stage]->randomizeEnabled = randLevels;
        paramQuantities[B_PARAM+stage]->randomizeEnabled = randDrifts;
        break;
    }
    paramQuantities[A_CV_PARAM+stage]->randomizeEnabled = randAttens;
    paramQuantities[B_CV_PARAM+stage]->randomizeEnabled = randAttens;
    paramQuantities[TRIG_PARAM+stage]->randomizeEnabled = randTrigs;
  }
  
  void configRandomize() {
    for (int i=0; i<MAX_STAGES; i++)
      configStageRandomize(i);
  }


  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_t* array = json_array();
    for (int i=0; i<MAX_STAGES; i++){
      json_t* obj = json_object();
      json_object_set_new(obj, "action", json_integer(stages[i].action));
      json_object_set_new(obj, "up", json_boolean(stages[i].up));
      json_object_set_new(obj, "down", json_boolean(stages[i].down));
      json_array_append_new(array, obj);
    }
    json_object_set_new(rootJ, "stages", array);
    json_object_set_new(rootJ, "stageCnt", json_integer(stageCnt));
    json_object_set_new(rootJ, "vcaType", json_integer(vcaMode));
    json_object_set_new(rootJ, "randTimes", json_boolean(randTimes));
    json_object_set_new(rootJ, "randLevels", json_boolean(randLevels));
    json_object_set_new(rootJ, "randShapes", json_boolean(randShapes));
    json_object_set_new(rootJ, "randDrifts", json_boolean(randDrifts));
    json_object_set_new(rootJ, "randAttens", json_boolean(randAttens));
    json_object_set_new(rootJ, "randTrigs", json_boolean(randTrigs));
    return rootJ;
  }
  
  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val = NULL;
    json_t* array;
    json_t* obj;
    size_t index;
    if ((array = json_object_get(rootJ, "stages"))){
      json_array_foreach(array, index, obj){
        if ((val = json_object_get(obj,"action")))
          stages[index].action = json_integer_value(val);
        if ((val = json_object_get(obj,"up")))
          stages[index].up = json_boolean_value(val);
        if ((val = json_object_get(obj,"down")))
          stages[index].down = json_boolean_value(val);
      }
    }
    if ((val = json_object_get(rootJ, "stageCnt")))
      stageCnt = json_integer_value(val);
    if ((val = json_object_get(rootJ, "vcaType")))
      vcaMode = json_integer_value(val);
    if ((val = json_object_get(rootJ, "randTimes")))
      randTimes = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "randLevels")))
      randLevels = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "randShapes")))
      randShapes = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "randDrifts")))
      randDrifts = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "randAttens")))
      randAttens = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "randTrigs")))
      randTrigs = json_boolean_value(val);
    configRandomize();
  }
  
  void onUnBypass(const UnBypassEvent &e) override {
    reset = true;
  }

  
};

struct EnvelopeFactoryWidget : VenomWidget {
  bool resizeNeeded = false;
  int slow=0,
      vcaMode=0,
      stageCnt=0,
      lightTheme = getDefaultTheme(),
      darkTheme = getDefaultDarkTheme();
  VCVLightBezelLockable<MediumSimpleLight<WhiteLight>> *manualGate = NULL;
  
  struct LabelWidget : FramebufferWidget {
    std::string labelNames[2] {"EnvFactoryLabel", "EnvelopeFactoryLabel"};
    SvgWidget *sw;
    int currentTheme = -1;
    int currentName = -1;

    LabelWidget() {
      box.pos = Vec(0.f,0.f);
      sw = new SvgWidget;
      addChild(sw);
    }
    
    void setLabel(int theme, int cnt) {
      if (theme!=currentTheme || (cnt>1?1:0)!=currentName) {
        currentTheme = theme;
        currentName = cnt>1 ? 1 : 0;
        sw->setSvg(Svg::load(asset::plugin(pluginInstance,faceplatePath(labelNames[currentName], themes[theme]))));
        box.size = sw->box.size;
        setDirty();
      }
      box.pos = Vec((75.f + cnt*45.f - box.size.x)/2.f, 0.f);
    }

    void  draw (const DrawArgs &args) override {
      oversample = APP->window->pixelRatio<2.0 ? 2.0 : 1.0;
      FramebufferWidget::draw(args);
    }
  };

  struct StagePlateWidget : FramebufferWidget {
    std::string plateNames[3] {"EnvelopeStageMove", "EnvelopeStageHold", "EnvelopeStageSust"};
    SvgWidget *sw;
    int currentTheme = -1,
        currentAction = -1;

    StagePlateWidget() {
      box.pos = Vec(0.f,0.f);
      sw = new SvgWidget;
      addChild(sw);
    }
    
    void setPlate(int theme, int action) {
      if (action>2)
        action = 0;
      if (theme!=currentTheme || action!=currentAction) {
        currentTheme = theme;
        currentAction = action;
        sw->setSvg(Svg::load(asset::plugin(pluginInstance,faceplatePath(plateNames[action], themes[theme]))));
        box.size = sw->box.size;
        setDirty();
      }
    }

    void  draw (const DrawArgs &args) override {
      oversample = APP->window->pixelRatio<2.0 ? 2.0 : 1.0;
      FramebufferWidget::draw(args);
    }
  };
  
  struct StageWidget : Widget {
    int action = -1;
    StagePlateWidget *plate;
    
    StageWidget(Vec pos) {
      box.pos = pos;
      box.size = Vec(46.f,380.f);
      plate = new StagePlateWidget;
      addChild(plate);
    }
  };
  
  StageWidget *stages[MAX_STAGES];
  LabelWidget *nameLabel = new LabelWidget;
  SvgPanel *borderPanel = new SvgPanel;

  struct ActionSwitch : GlowingSvgSwitchLockable {
    ActionSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Move.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Hold.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Sust.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Rise.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/action_Fall.svg")));
    }
  };

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_Full.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_RTrg.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_Gate.svg")));
    }
  };

  struct SlowSwitch : GlowingSvgSwitchLockable {
    SlowSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
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

  EnvelopeFactoryWidget(EnvelopeFactory* module) {
    setModule(module);
    setVenomPanel("EnvelopeFactory");

    addInput(createInputCentered<PolyPort>(Vec(22.f, 62.5f), module, EnvelopeFactory::VOCT_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.f, 106.5f), module, EnvelopeFactory::AMP_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(52.f, 106.5f), module, EnvelopeFactory::OFF_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(22.f, 135.5f), module, EnvelopeFactory::AMP_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(52.f, 135.5f), module, EnvelopeFactory::OFF_INPUT));
    addParam(createLockableParamCentered<SlowSwitch>(Vec(22.f, 171.5f), module, EnvelopeFactory::SLOW_PARAM));
    addParam(createLockableParamCentered<OnOffSwitch>(Vec(52.f, 171.5f), module, EnvelopeFactory::FROM0_PARAM));
    addParam(createLockableParamCentered<OnOffSwitch>(Vec(22.f, 199.5f), module, EnvelopeFactory::GATE_MODE_PARAM));
    addParam(createLockableParamCentered<TriSwitch>(Vec(52.f, 199.5f), module, EnvelopeFactory::RETRIG_MODE_PARAM));
    manualGate = createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteLight>>>(Vec(22.,227.5f), module, EnvelopeFactory::GATE_IN_PARAM, EnvelopeFactory::GATE_IN_LIGHT);
    addParam(manualGate);
    addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteLight>>>(Vec(52.,227.5f), module, EnvelopeFactory::RETRIG_PARAM, EnvelopeFactory::RETRIG_LIGHT));
    addInput(createInputCentered<PolyPort>(Vec(22.f, 256.5f), module, EnvelopeFactory::GATE_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(52.f, 256.5f), module, EnvelopeFactory::RETRIG_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.f, 304.5f), module, EnvelopeFactory::TRIGS_OUTPUT));
    addChild(createLockableParamCentered<OnOffSwitch>(Vec(42.5f, 287.5f), module, EnvelopeFactory::IDLE_PARAM));
    addChild(createLightCentered<SmallLight<YellowLight>>(Vec(62.f,287.5f), module, EnvelopeFactory::IDLE_LIGHT));
    addOutput(createOutputCentered<PolyPort>(Vec(52.f, 304.5f), module, EnvelopeFactory::IDLE_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.f, 342.5f), module, EnvelopeFactory::ENV_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(52.f, 342.5f), module, EnvelopeFactory::INV_OUTPUT));
    for (int i=0; i<(module ? MAX_STAGES : 4); i++) {
      StageWidget *stg = new StageWidget(Vec(74.f+i*45, 0.f));
      stg->addChild(createLightCentered<SmallLight<YellowLight>>(Vec(23.5f, 23.5f), module, EnvelopeFactory::UP_LIGHT+i));
      stg->addChild(createLockableParamCentered<ActionSwitch>(Vec(23.5f, 40.f), module, EnvelopeFactory::ACTION_PARAM+i));
      stg->addChild(createLightCentered<SmallLight<YellowLight>>(Vec(23.5f, 56.5f), module, EnvelopeFactory::DOWN_LIGHT+i));
      stg->addChild(createLockableParamCentered<ModeSwitch>(Vec(23.5f, 73.f), module, EnvelopeFactory::MODE_PARAM+i));
      stg->addChild(createLockableParamCentered<RoundBlackKnobLockable>(Vec(23.5f, 114.5f), module, EnvelopeFactory::A_PARAM+i));
      stg->addChild(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(23.5f, 150.5f), module, EnvelopeFactory::A_CV_PARAM+i));
      stg->addChild(createInputCentered<PolyPort>(Vec(23.5f, 184.f), module, EnvelopeFactory::A_CV_INPUT+i));
      stg->addChild(createLockableParamCentered<RoundBlackKnobLockable>(Vec(23.5f, 225.f), module, EnvelopeFactory::B_PARAM+i));
      stg->addChild(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(23.5f, 261.f), module, EnvelopeFactory::B_CV_PARAM+i));
      stg->addChild(createInputCentered<PolyPort>(Vec(23.5f, 294.5f), module, EnvelopeFactory::B_CV_INPUT+i));
      stg->addChild(createLockableParamCentered<OnOffSwitch>(Vec(14.f, 326.5f), module, EnvelopeFactory::TRIG_PARAM+i));
      stg->addChild(createLightCentered<SmallLight<YellowLight>>(Vec(33.5f, 326.5f), module, EnvelopeFactory::GATE_LIGHT+i));
      stg->addChild(createOutputCentered<PolyPort>(Vec(23.5f, 342.5f), module, EnvelopeFactory::GATE_OUTPUT+i));
      if (module)
        stg->hide();
      stages[i] = stg;
      addChild(stg);
    }
    addChild(nameLabel);
    borderPanel->sw->setSvg(Svg::load(asset::plugin(pluginInstance,"res/emptyPanel.svg")));
    addChild(borderPanel);
    if (!module){
      setSize(Vec(255,380));
      borderPanel->box.size = box.size;
      borderPanel->panelBorder->box.size = box.size;
    }
  }
  
  struct stageCountAction : history::Action {
    int64_t modId;
    int oldVal;
    int newVal;
  
    stageCountAction(EnvelopeFactory *mod, int cnt) {
      name = "set EnvelopeFactory stage count";
      modId = mod->id;
      oldVal = mod->stageCnt;
      newVal = cnt;
    }
  
    void undo() override {
      EnvelopeFactory *mod = dynamic_cast<EnvelopeFactory*>(APP->engine->getModule(modId));
      if (mod)
        mod->stageCnt = oldVal;
    }
  
    void redo() override {
      EnvelopeFactory *mod = dynamic_cast<EnvelopeFactory*>(APP->engine->getModule(modId));
      if (mod)
        mod->stageCnt = newVal;
    }
  };  

  struct vcaModeAction : history::Action {
    int64_t modId;
    int oldVal;
    int newVal;
  
    vcaModeAction(EnvelopeFactory *mod, int mode) {
      name = "set EnvelopeFactory VCA mode";
      modId = mod->id;
      oldVal = mod->vcaMode;
      newVal = mode;
    }
  
    void undo() override {
      EnvelopeFactory *mod = dynamic_cast<EnvelopeFactory*>(APP->engine->getModule(modId));
      if (mod)
        mod->vcaMode = oldVal;
    }
  
    void redo() override {
      EnvelopeFactory *mod = dynamic_cast<EnvelopeFactory*>(APP->engine->getModule(modId));
      if (mod)
        mod->vcaMode = newVal;
    }
  };  

  void appendContextMenu(Menu* menu) override {
    EnvelopeFactory* module = dynamic_cast<EnvelopeFactory*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexSubmenuItem(
      "Stage count",
      {"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20"},
      [=]() {
        return module->stageCnt - 1;
      },
      [=](int cnt) {
        if (++cnt != module->stageCnt) {
          APP->history->push(new stageCountAction(module, cnt));
          module->reset = true;
          module->stageCnt = cnt;
        }
      }
    ));
    menu->addChild(createIndexSubmenuItem(
      "VCA mode",
      {"Off","Standard VCA", "VCA with 0 crossing synced triggers"},
      [=]() {
        return module->vcaMode;
      },
      [=](int mode) {
        if (mode != module->vcaMode) {
          APP->history->push(new vcaModeAction(module, mode));
          module->vcaMode = mode;
        }
      }
    ));
    menu->addChild(createSubmenuItem("Randomize configuration", "", [=](Menu* submenu) {
      submenu->addChild(createBoolMenuItem("Stage times", "", [=](){return module->randTimes;}, [=](bool val){module->randTimes=val; module->configRandomize();}));
      submenu->addChild(createBoolMenuItem("Stage levels", "", [=](){return module->randLevels;}, [=](bool val){module->randLevels=val; module->configRandomize();}));
      submenu->addChild(createBoolMenuItem("Stage shapes", "", [=](){return module->randShapes;}, [=](bool val){module->randShapes=val; module->configRandomize();}));
      submenu->addChild(createBoolMenuItem("Stage drifts", "", [=](){return module->randDrifts;}, [=](bool val){module->randDrifts=val; module->configRandomize();}));
      submenu->addChild(createBoolMenuItem("Stage attenuverters", "", [=](){return module->randAttens;}, [=](bool val){module->randAttens=val; module->configRandomize();}));
      submenu->addChild(createBoolMenuItem("Stage triggers", "", [=](){return module->randTrigs;}, [=](bool val){module->randTrigs=val; module->configRandomize();}));
    }));    
    VenomWidget::appendContextMenu(menu);
  }

  void draw(const DrawArgs &args) override {
    static_cast<SvgPanel*>(getPanel())->panelBorder->hide();
    VenomWidget::draw(args);
  }

  void moveRecursive(const std::vector<ModuleWidget*>& vec, size_t index, float x) {
    if (index >= vec.size() || vec[index]->box.pos.x >= x)
      return;
    moveRecursive(vec, index+1, x + vec[index]->box.size.x);
    vec[index]->setPosition(Vec(x, vec[index]->box.pos.y));
  }

  void step() override {
    if (this->module) {
      EnvelopeFactory* mod = static_cast<EnvelopeFactory*>(this->module);
      if ((mod->defaultTheme!=getDefaultTheme() && mod->currentTheme==0) ||
          (mod->defaultDarkTheme!=getDefaultDarkTheme() && mod->currentTheme==0) ||
          mod->prevTheme!=mod->currentTheme ||
          vcaMode!=mod->vcaMode)
      {
        moduleName = mod->vcaMode ? "EnvelopeFactoryVca" : "EnvelopeFactory";
        mod->prevTheme = -1;
        resizeNeeded = true;
      }
    }
    VenomWidget::step();
    if(this->module) {
      EnvelopeFactory* mod = static_cast<EnvelopeFactory*>(this->module);
      if (static_cast<bool>(mod->params[EnvelopeFactory::GATE_MODE_PARAM].getValue()) != manualGate->latch) {
        manualGate->momentary = !manualGate->momentary;
        manualGate->latch = !manualGate->latch;
      }
      if (vcaMode != mod->vcaMode) {
        vcaMode = mod->vcaMode;
        mod->setParamFactoryName(EnvelopeFactory::AMP_PARAM, vcaMode ? "VCA level" : "Amplitude");
        mod->setParamFactoryName(EnvelopeFactory::OFF_PARAM, vcaMode ? "Velocity response (exp<->linear<->log))" : "Offset");
        mod->setPortFactoryName(EnvelopeFactory::AMP_INPUT, vcaMode ? "VCA" : "Amplitude", false);
        mod->setPortFactoryName(EnvelopeFactory::OFF_INPUT, vcaMode ? "Velocity" : "Offset", false);
        mod->setPortFactoryName(EnvelopeFactory::INV_OUTPUT, vcaMode ? "VCA" : "Inverse envelope", true);
        ParamQuantity *pq = mod->paramQuantities[EnvelopeFactory::AMP_PARAM];
        pq->displayMultiplier = vcaMode ? 50.f : 100.f;
        pq->displayOffset = vcaMode ? 50.f : 0.f;
        pq = mod->paramQuantities[EnvelopeFactory::OFF_PARAM];
        pq->displayMultiplier = vcaMode ? 1.f : 100.f;
        pq->unit = vcaMode ? "" : "%";
      }
      int newSlow = mod->params[EnvelopeFactory::SLOW_PARAM].getValue();
      if (stageCnt != mod->stageCnt || resizeNeeded) {
        while (stageCnt > mod->stageCnt) { // hide stages and delete cables
          stages[--stageCnt]->hide();
          PortWidget *port = getInput(EnvelopeFactory::A_CV_INPUT+stageCnt);
          if (port)
            APP->scene->rack->clearCablesOnPort(port);
          port = getInput(EnvelopeFactory::B_CV_INPUT+stageCnt);
          if (port)
            APP->scene->rack->clearCablesOnPort(port);
          port = getOutput(EnvelopeFactory::GATE_OUTPUT+stageCnt);
          if (port)
            APP->scene->rack->clearCablesOnPort(port);
        }
        if (stageCnt < mod->stageCnt) { // shift neighbors right and show stages
          std::vector<ModuleWidget*> mods = APP->scene->rack->getModules();
          mods.erase( std::remove_if( mods.begin(), mods.end(), [this](ModuleWidget* mw){
            return mw==this || std::abs(mw->box.pos.y-this->box.pos.y)>10.f || mw->box.pos.x+mw->box.size.x<=this->box.pos.x;
          }), mods.end());
          std::sort(mods.begin(), mods.end(), [](ModuleWidget *a, ModuleWidget *b){return a->box.pos.x < b->box.pos.x;});
          moveRecursive(mods, 0, box.pos.x + 75.f + mod->stageCnt*45.f);
          while (stageCnt < mod->stageCnt)
            stages[stageCnt++]->show();
        }
        setSize(Vec(75+mod->stageCnt*45, 380));
        borderPanel->box.size = box.size;
        borderPanel->panelBorder->box.size = box.size;
        borderPanel->fb->setDirty();
        resizeNeeded = false;
      }  
      for (int i=0; i<mod->stageCnt; i++) { // reconfigure A and B
        int newAction = mod->params[EnvelopeFactory::ACTION_PARAM+i].getValue();
        if (newAction!=stages[i]->action || newSlow!=slow) {
          stages[i]->action = newAction;
          std::string prefix = "Stage " + std::to_string(i+1);
          ParamQuantity *aq = mod->paramQuantities[EnvelopeFactory::A_PARAM+i];
          EnvelopeFactory::BQuantity *bq = static_cast<EnvelopeFactory::BQuantity*>(mod->paramQuantities[EnvelopeFactory::B_PARAM+i]);
          bq->action = newAction;
          switch (newAction) {
            case 0: // MOVE
            case 3: // RISE
            case 4: // FALL
              prefix += (newAction==3) ? " rise " : (newAction==4) ? " fall " : " move ";
              mod->setParamFactoryName(EnvelopeFactory::A_PARAM+i, prefix + "time");
              mod->setParamFactoryName(EnvelopeFactory::A_CV_PARAM+i, prefix + "time CV");
              mod->setPortFactoryName(EnvelopeFactory::A_CV_INPUT+i, prefix + "time CV", false);
              mod->setParamFactoryName(EnvelopeFactory::B_PARAM+i, prefix + "shape");
              mod->setParamFactoryName(EnvelopeFactory::B_CV_PARAM+i, prefix + "shape CV");
              mod->setPortFactoryName(EnvelopeFactory::B_CV_INPUT+i, prefix + "shape CV", false);
              aq->unit = " s";
              aq->displayBase = 10.f;
              aq->displayMultiplier = newSlow==3 ? 1000.f : newSlow==2 ? 100.f : newSlow ? 10.f : 1.f;
              aq->displayOffset = 0.f;
              bq->unit = "";
              bq->displayBase = 0.f;
              bq->displayMultiplier = 0.5f;
              bq->displayOffset = 0.5f;
              break;
            case 1: // HOLD
              prefix += " hold ";
              mod->setParamFactoryName(EnvelopeFactory::A_PARAM+i, prefix + "level");
              mod->setParamFactoryName(EnvelopeFactory::A_CV_PARAM+i, prefix + "level CV");
              mod->setPortFactoryName(EnvelopeFactory::A_CV_INPUT+i, prefix + "level CV", false);
              mod->setParamFactoryName(EnvelopeFactory::B_PARAM+i, prefix + "time");
              mod->setParamFactoryName(EnvelopeFactory::B_CV_PARAM+i, prefix + "time CV");
              mod->setPortFactoryName(EnvelopeFactory::B_CV_INPUT+i, prefix + "time CV", false);
              aq->unit = "%";
              aq->displayBase = 0.f;
              aq->displayMultiplier = 25.f;
              aq->displayOffset = 75.f;
              bq->unit = " s";
              bq->displayBase = 10.f;
              bq->displayMultiplier = newSlow==3 ? 1000.f : newSlow==2 ? 100.f : newSlow ? 10.f : 1.f;
              bq->displayOffset = 0.f;
              break;
            case 2: // SUST
              prefix += " sustain ";
              mod->setParamFactoryName(EnvelopeFactory::A_PARAM+i, prefix + "level");
              mod->setParamFactoryName(EnvelopeFactory::A_CV_PARAM+i, prefix + "level CV");
              mod->setPortFactoryName(EnvelopeFactory::A_CV_INPUT+i, prefix + "level CV", false);
              mod->setParamFactoryName(EnvelopeFactory::B_PARAM+i, prefix + "drift");
              mod->setParamFactoryName(EnvelopeFactory::B_CV_PARAM+i, prefix + "drift CV");
              mod->setPortFactoryName(EnvelopeFactory::B_CV_INPUT+i, prefix + "drift CV", false);
              aq->unit = "%";
              aq->displayBase = 0.f;
              aq->displayMultiplier = 25.f;
              aq->displayOffset = 75.f;
              bq->unit = " %/s";
              bq->displayBase = 10.f;
              bq->displayMultiplier = 100.f;
              bq->displayOffset = 0.f;
              break;
          }
        }
        mod->configStageRandomize(i);
        stages[i]->plate->setPlate(currentTheme, stages[i]->action);
        mod->lights[EnvelopeFactory::UP_LIGHT+i].setBrightness(mod->stages[i].up);
        mod->lights[EnvelopeFactory::DOWN_LIGHT+i].setBrightness(mod->stages[i].down);
      }
      nameLabel->setLabel(currentTheme, stageCnt);
      slow = newSlow;
      mod->lights[EnvelopeFactory::GATE_IN_LIGHT].setBrightness(mod->params[EnvelopeFactory::GATE_IN_PARAM].getValue());
      mod->lights[EnvelopeFactory::RETRIG_LIGHT].setBrightness(mod->params[EnvelopeFactory::RETRIG_PARAM].getValue());
    }
    else for (int i=0; i<4; i++) {
      stages[i]->plate->setPlate(settings::preferDarkPanels ? darkTheme : lightTheme, 0);
      nameLabel->setLabel(settings::preferDarkPanels ? darkTheme : lightTheme, 4);
    }
  }

};

}

Model* modelVenomEnvelopeFactory = createModel<Venom::EnvelopeFactory, Venom::EnvelopeFactoryWidget>("EnvelopeFactory");

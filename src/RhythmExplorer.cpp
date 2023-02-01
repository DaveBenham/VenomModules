// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

// Thanks to Andrew Hanson for the initial prototype code
// that I was able to flesh out into this final form

#include "plugin.hpp"
#include "ThemeStrings.hpp"

#define SLIDER_COUNT 8
#define MAX_STEP_LENGTH 16
#define MAX_UNIT64 18446744073709551615ul
#define LIGHT_OFF 0.02f
#define LIGHT_DIM 0.1f
#define LIGHT_FADE 5e-6f

#define MODULE_NAME RhythmExplorer
static const std::string moduleName = "RhythmExplorer";

static const std::vector<std::string> CHANNEL_DIVISION_LABELS = {
  "1/2",
  "1/4",
  "1/8",
  "1/16",
  "1/32",
  "1/2 Triplet",
  "1/4 Triplet",
  "1/8 Triplet",
  "1/16 Triplet",
  "1/32 Triplet"
};

static const std::vector<std::string> CHANNEL_MODE_LABELS = {
  "All",
  "Linear",
  "Offbeat",
  "Global Default"
};

static const std::vector<std::string> POLY_MODE_LABELS = {
  "All",
  "Linear",
  "Offbeat"
};

static const int GATE_LENGTH [10] = {
  48,
  24,
  12,
  6,
  3,
  32,
  16,
  8,
  4,
  2
};

struct RhythmExplorer : Module {
  enum ParamId {
    ENUMS(DENSITY_PARAM, SLIDER_COUNT),
    NEW_SEED_BUTTON_PARAM,
    BAR_COUNT_PARAM,
    ENUMS(RATE_PARAM, SLIDER_COUNT),
    RESET_BUTTON_PARAM,
    ENUMS(MUTE_CHANNEL_PARAM, SLIDER_COUNT),
    RUN_GATE_PARAM,
    BAR_LENGTH_PARAM,
    ENUMS(MODE_CHANNEL_PARAM, SLIDER_COUNT),
    MODE_POLY_PARAM,
    MUTE_POLY_PARAM,
    POLAR_PARAM,
    LOCK_PARAM,
    INIT_DENSITY_PARAM,
    PATOFF_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    CLOCK_INPUT,
    SEED_INPUT,
    RNG_OVERRIDE_INPUT,
    RESET_TRIGGER_INPUT,
    ENUMS(DENSITY_CHANNEL_INPUT, SLIDER_COUNT),
    ENUMS(MODE_CHANNEL_INPUT, SLIDER_COUNT),
    DENSITY_POLY_INPUT,
    MODE_POLY_INPUT,
    NEW_SEED_TRIGGER_INPUT,
    RUN_GATE_INPUT,
    BAR_COUNT_INPUT,
    BAR_LENGTH_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(GATE_OUTPUT, SLIDER_COUNT),
    ENUMS(CLOCK_OUTPUT, SLIDER_COUNT),
    SEED_OUTPUT,
    GATE_POLY_OUTPUT,
    CLOCK_POLY_OUTPUT,
    START_OF_BAR_OUTPUT,
    GATE_OR_OUTPUT,
    GATE_XOR_OUTPUT,
    GATE_XOR_ODD_OUTPUT,
    GATE_XOR_ONE_OUTPUT,
    START_OF_PHRASE_OUTPUT,
    RESET_TRIGGER_OUTPUT,
    RUN_GATE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(BAR_STEP_LIGHT, MAX_STEP_LENGTH),
    ENUMS(PHRASE_STEP_LIGHT, MAX_STEP_LENGTH),
    ENUMS(DENSITY_LIGHT, SLIDER_COUNT),
    ENUMS(MUTE_CHANNEL_LIGHT, SLIDER_COUNT),
    MUTE_POLY_LIGHT,
    NEW_SEED_LIGHT,
    RESET_LIGHT,
    RUN_GATE_LIGHT,
    LOCK_LIGHT,
    INIT_DENSITY_LIGHT,
    PATOFF_LIGHT,
    LIGHTS_LEN
  };

  enum Modes {
    ALL_MODE,
    LINEAR_MODE,
    OFFBEAT_MODE,
    GLOBAL_MODE
  };

  //Persistant State
  float internalSeed;
  bool runGateActive;

  #include "ThemeModVars.hpp"

  //Non Persistant State
  bool isUni;
  int currentPulse;
  int currentCycle;
  int currentBar;
  rack::random::Xoroshiro128Plus rng;
  bool resetEvent;
  bool clockHigh;
  bool resetTrigHigh;
  bool resetBtnHigh;
  bool newSeedBtnHigh;
  bool newSeedTrigHigh;
  bool runGateBtnHigh;
  bool runGateTrigHigh;
  bool lockActive;
  bool initBtnHigh;
  bool densityLightOn[SLIDER_COUNT];
  dsp::ClockDivider lightDivider;
  dsp::PulseGenerator trigGenerator;

  RhythmExplorer() {

    struct FixedSwitchQuantity : SwitchQuantity {
      std::string getDisplayValueString() override {
        return labels[getValue()];
      }
    };
    struct GlobalModeQuantity : SwitchQuantity {
      std::string getDisplayValueString() override {
        RhythmExplorer* module = reinterpret_cast<RhythmExplorer*>(this->module);
        Input* input = &module->inputs[RhythmExplorer::MODE_POLY_INPUT];
        if (input->isPolyphonic()) {
          std::string rtn = "";
          for(int i=0; i<SLIDER_COUNT; i++)
            rtn += "\nMode " + std::to_string(i+1) + " " + labels[rack::math::clamp(static_cast<int>(input->getPolyVoltage(i)), 0, 2)];
          return rtn;
        }
        if (input->isConnected())
          return labels[rack::math::clamp(static_cast<int>(input->getVoltage()), 0, 2)];
        // else paramter
        return labels[getValue()];
      }
    };

    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configInput(CLOCK_INPUT,"24PPQN Clock");
    configInput(NEW_SEED_TRIGGER_INPUT,"Dice Trigger");
    configInput(RESET_TRIGGER_INPUT,"Reset Trigger");
    configInput(SEED_INPUT,"Seed");
    configInput(RNG_OVERRIDE_INPUT,"Random Override");
    configOutput(SEED_OUTPUT,"Seed Output");

    configButton(NEW_SEED_BUTTON_PARAM, "Dice Trigger");
    configButton(RESET_BUTTON_PARAM,"Reset Trigger");

    configButton(RUN_GATE_PARAM, "Run Gate");
    configInput(RUN_GATE_INPUT,"Run Gate");
    
    configSwitch(PATOFF_PARAM, 0.f, 1.f, 0.f, "Pattern Status", {"On", "Off"});

    configOutput(GATE_OR_OUTPUT,"OR Trigger");
    configOutput(GATE_XOR_ODD_OUTPUT,"XOR Odd Parity Trigger");
    configOutput(GATE_XOR_ONE_OUTPUT,"XOR 1 Hot Trigger");

    configParam(BAR_COUNT_PARAM, 1, MAX_STEP_LENGTH, 1, "Phrase Bar Count");
    configParam(BAR_LENGTH_PARAM, 1, MAX_STEP_LENGTH, 4, "Bar 1/4 Count");
    configInput(BAR_COUNT_INPUT,"Phrase Bar Count CV");
    configInput(BAR_LENGTH_INPUT,"Bar 1/4 Count CV");
    configOutput(START_OF_BAR_OUTPUT,"Start of Bar Trigger");
    configOutput(START_OF_PHRASE_OUTPUT,"Start of Phrase Trigger");

    for(int si = 0; si < SLIDER_COUNT; si++){
      std::string si_s = std::to_string(si+1);
      configSwitch<FixedSwitchQuantity>(RATE_PARAM + si, 0, 9, si+1, "Division " + si_s, CHANNEL_DIVISION_LABELS);
      configParam(DENSITY_PARAM + si, 0.f, 10.f, 0.f, "Density " + si_s, "%", 0.f, 10.f);
      lights[DENSITY_LIGHT + si].setBrightness(LIGHT_DIM);
      configOutput(GATE_OUTPUT + si, "Gate " + si_s);
      configOutput(CLOCK_OUTPUT + si, "Clock " + si_s);
      configInput(DENSITY_CHANNEL_INPUT + si, "Density CV " + si_s);
      if (si>0)
        configInput(MODE_CHANNEL_INPUT + si, "Mode CV " + si_s);
      configSwitch(MUTE_CHANNEL_PARAM + si, 0.f, 1.f, 0.f, "Mute " + si_s, {"Unmuted", "Muted"});
      if (si==0)
        configSwitch<FixedSwitchQuantity>(MODE_CHANNEL_PARAM + si, 0, 0, 0, "Mode " + si_s, {"All"});
      else
        configSwitch<FixedSwitchQuantity>(MODE_CHANNEL_PARAM + si, 0, 3, 0, "Mode " + si_s, CHANNEL_MODE_LABELS);
    }
    configSwitch<FixedSwitchQuantity>(POLAR_PARAM, 0, 1, 0, "Density Polarity", {"Uniploar", "Bipolar"});
    configSwitch<GlobalModeQuantity>(MODE_POLY_PARAM, 0, 2 , 0, "Global Mode", POLY_MODE_LABELS);
    configSwitch(MUTE_POLY_PARAM, 0.f, 1.f, 0.f, "Global Mute", {"Unmuted", "Muted"});
    configInput(MODE_POLY_INPUT, "Global Mode CV");
    configInput(DENSITY_POLY_INPUT,"Density Poly CV");
    configOutput(CLOCK_POLY_OUTPUT, "Clock Poly");
    configOutput(GATE_POLY_OUTPUT, "Gate Poly");
    configOutput(RESET_TRIGGER_OUTPUT, "Reset Trigger");
    configOutput(RUN_GATE_OUTPUT, "Run Gate");
    configSwitch(LOCK_PARAM, 0.f, 1.f, 0.f, "Divisions, Modes, and Polarity Status", {"Unlocked", "Locked"});
    configButton(INIT_DENSITY_PARAM, "Initialize Densities");

    lightDivider.setDivision(32);

    initialize();
  }

  void onReset(const ResetEvent& e) override {
    ParamQuantity* q;
    for (int i=0; i<SLIDER_COUNT; i++){
      q = getParamQuantity(DENSITY_PARAM + i);
      if (!q) continue;
      q->defaultValue = 0.f;
      q->displayMultiplier = 10.f;
      q->displayOffset = 0.f;
    }
    Module::onReset(e);
    initialize();
  }

  void initialize(){
    currentPulse = 0;
    currentCycle = 0;
    currentBar = 0;
    rng = {};
    resetEvent = false;
    runGateActive = false;
    initBtnHigh = false;
    clockHigh = false;
    resetTrigHigh = false;
    resetBtnHigh = false;
    newSeedBtnHigh = false;
    newSeedTrigHigh = false;
    runGateBtnHigh = false;
    runGateTrigHigh = false;
    isUni = true;
    for (int i=0; i<SLIDER_COUNT; i++){
      densityLightOn[i] = false;
    }
    getSeed();
    reseedRng();
    updateLights();
    setLockStatus();
  }

  json_t *dataToJson() override{
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "internalSeed", json_real(internalSeed));
    json_object_set_new(rootJ, "runGateActive", json_boolean(runGateActive));
    #include "ThemeToJson.hpp"

    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {

    internalSeed = json_real_value(json_object_get(rootJ, "internalSeed"));
    runGateActive = json_is_true(json_object_get(rootJ, "runGateActive"));
    #include "ThemeFromJson.hpp"

    reseedRng();
  }

  void updateLights(){
    lights[RUN_GATE_LIGHT].setBrightness(runGateActive ? 1.f : LIGHT_OFF);
    for (int i=0; i<SLIDER_COUNT; i++)
      lights[MUTE_CHANNEL_LIGHT + i].setBrightness(params[MUTE_CHANNEL_PARAM + i].getValue()>0.f ? 1.f : LIGHT_OFF);
    lights[MUTE_POLY_LIGHT].setBrightness(params[MUTE_POLY_PARAM].getValue()>0.f ? 1.f : LIGHT_OFF);
    lights[PATOFF_LIGHT].setBrightness((params[PATOFF_PARAM].getValue()>0.f) ? 1.f : LIGHT_OFF);
    lights[LOCK_LIGHT].setBrightness(lockActive ? 1.f : LIGHT_OFF);
  }

  void getSeed(){
    internalSeed = inputs[SEED_INPUT].isConnected() ? rack::math::clamp(inputs[SEED_INPUT].getVoltage(), 0.f, 10.f) : rack::math::clamp(rack::random::uniform() * 10.f, 1e-19f, 10.f);
    if (internalSeed < 1e-19f)
      internalSeed = 0.f;
  }

  void reseedRng(){
    if (internalSeed > 0.f && params[PATOFF_PARAM].getValue() <= 0.f) {
      float seed1 = internalSeed / 10.f;
      float seed2 = std::fmod(internalSeed,1.f);
      uint64_t s1 = seed1 * MAX_UNIT64;
      uint64_t s2 = seed2 * MAX_UNIT64;
      rng.seed(s1, s2);
    }
  }

  void setLockStatus(){
    ParamQuantity* q;
    float val;
    lockActive = params[LOCK_PARAM].getValue() > 0.f;
    for (int i=0; i<SLIDER_COUNT; i++){
      if ((q = getParamQuantity(RATE_PARAM + i))){
        val = q->getValue();
        q->minValue = lockActive ? val : 0.f;
        q->maxValue = lockActive ? val : 9.f;
        q->defaultValue = lockActive ? val : i+1;
      }
      if (i>0 && (q = getParamQuantity(MODE_CHANNEL_PARAM + i))){
        val = q->getValue();
        q->minValue = lockActive ? val : 0.f;
        q->maxValue = lockActive ? val : 3.f;
        q->defaultValue = lockActive ? val : 0.f;
      }
    }
    if ((q = getParamQuantity(MODE_POLY_PARAM))) {
      val = q->getValue();
      q->minValue = lockActive ? val : 0.f;
      q->maxValue = lockActive ? val : 2.f;
      q->defaultValue = lockActive ? val : 0.f;
    }
    if ((q = getParamQuantity(POLAR_PARAM))){
      val = q->getValue();
      q->minValue = lockActive ? val : 0.f;
      q->maxValue = lockActive ? val : 1.f;
      q->defaultValue = lockActive ? val : 0.f;
    }
  }

  inline bool schmittTrigger(bool & state, float input){
    if(!state && input >= 2.0f){
      state = true;
      return true;
    }else if(state && input <= 0.1f){
      state = false;
      return false;
    }
    return false;
  }
  
  inline bool buttonTrigger(bool & state, float input){
    if(!state && input >= 1.0f){
      state = true;
      return true;
    }else if(state && input <= 0.f){
      state = false;
      return false;
    }
    return false;
  }

  void process(const ProcessArgs& args) override {
    
    // Density polarity
    if ((params[POLAR_PARAM].getValue()==0) != isUni){
      isUni = !isUni;
      ParamQuantity* q;
      for (int i=0; i<SLIDER_COUNT; i++){
        q = getParamQuantity(DENSITY_PARAM + i);
        if (!q) continue;
        q->defaultValue = isUni ? 0.f : 5.f;
        q->displayMultiplier = isUni ? 10.f : 20.f;
        q->displayOffset = isUni ? 0.f : -100.f;
      }
    }

    // Pattern Off
    lights[PATOFF_LIGHT].setBrightnessSmooth((params[PATOFF_PARAM].getValue()>0.f) ? 1.f : LIGHT_OFF, args.sampleTime);

    // Lock
    if ((params[LOCK_PARAM].getValue()>0.f) != lockActive)
      setLockStatus();
    lights[LOCK_LIGHT].setBrightnessSmooth(lockActive ? 1.f : LIGHT_OFF, args.sampleTime);

    // Init Density
    if(buttonTrigger(initBtnHigh,params[INIT_DENSITY_PARAM].getValue())){
      ParamQuantity* q;
      for (int i=0; i<SLIDER_COUNT; i++){
        if ((q = getParamQuantity(DENSITY_PARAM + i))){
          q->reset();
        }
      }
    }
    lights[INIT_DENSITY_LIGHT].setBrightnessSmooth(initBtnHigh ? 1.f : LIGHT_OFF, args.sampleTime);

    // Mutes - fully handled,  Modes - handled for button lable display only, LOCK may prevent true display
    for (int i=0; i<SLIDER_COUNT; i++) {
      if(i>0 && inputs[MODE_CHANNEL_INPUT + i].isConnected())
        params[MODE_CHANNEL_PARAM + i].setValue(rack::math::clamp(static_cast<int>(inputs[MODE_CHANNEL_INPUT + i].getVoltage()), 0, 3));
      lights[MUTE_CHANNEL_LIGHT + i].setBrightness(params[MUTE_CHANNEL_PARAM + i].getValue()>0.f ? 1.f : LIGHT_OFF);
    }
    if(inputs[MODE_POLY_INPUT].isConnected())
      params[MODE_POLY_PARAM].setValue(rack::math::clamp(static_cast<int>(inputs[MODE_POLY_INPUT].getVoltage()), 0, 2));
    lights[MUTE_POLY_LIGHT].setBrightness(params[MUTE_POLY_PARAM].getValue()>0.f ? 1.f : LIGHT_OFF);

    //Clock Logic and event timing
    bool newBar = false;
    bool newPhrase = false;

    bool oldClockHigh = clockHigh;
    bool clockEvent = schmittTrigger(clockHigh,inputs[CLOCK_INPUT].getVoltage());
    if (oldClockHigh && !clockHigh) { // clear triggers && lights
      outputs[GATE_OR_OUTPUT].setVoltage(0.f);
      outputs[GATE_XOR_ODD_OUTPUT].setVoltage(0.f);
      outputs[GATE_XOR_ONE_OUTPUT].setVoltage(0.f);
      outputs[START_OF_PHRASE_OUTPUT].setVoltage(0.f);
      outputs[CLOCK_POLY_OUTPUT].setVoltage(0.f, 8);
      outputs[START_OF_BAR_OUTPUT].setVoltage(0.f);
      outputs[CLOCK_POLY_OUTPUT].setVoltage(0.f, 9);
      for (int i=0; i<SLIDER_COUNT; i++){
        outputs[GATE_OUTPUT + i].setVoltage(0.f);
        outputs[GATE_POLY_OUTPUT].setVoltage(0.f, i);
        outputs[CLOCK_OUTPUT + i].setVoltage(0.f);
        outputs[CLOCK_POLY_OUTPUT].setVoltage(0.f, i);
        densityLightOn[i] = false;
      }
    }

    if (inputs[RUN_GATE_INPUT].isConnected()) {
      schmittTrigger(runGateTrigHigh,inputs[RUN_GATE_INPUT].getVoltage());
      buttonTrigger(runGateBtnHigh,params[RUN_GATE_PARAM].getValue());
      if ((runGateTrigHigh ^ runGateBtnHigh) != runGateActive){
        runGateActive = !runGateActive;
        if (runGateActive)
          resetEvent = true;
      }
    }
    else {
      if(buttonTrigger(runGateBtnHigh,params[RUN_GATE_PARAM].getValue())){
        runGateActive = !runGateActive;
        if (runGateActive)
          resetEvent = true;
      }
    }
    lights[RUN_GATE_LIGHT].setBrightnessSmooth(runGateActive ? 1.f : LIGHT_OFF, args.sampleTime);
    outputs[RUN_GATE_OUTPUT].setVoltage(runGateActive ? 10.f : 0.f);

    if(schmittTrigger(resetTrigHigh,inputs[RESET_TRIGGER_INPUT].getVoltage()) && trigGenerator.remaining <= 0) resetEvent = true;
    if(buttonTrigger(resetBtnHigh,params[RESET_BUTTON_PARAM].getValue()) && trigGenerator.remaining <= 0) resetEvent = true;
    lights[RESET_LIGHT].setBrightnessSmooth(resetTrigHigh || resetBtnHigh ? 1.f : LIGHT_OFF, args.sampleTime);

    bool newSeedEvent = false;
    if(schmittTrigger(newSeedTrigHigh,inputs[NEW_SEED_TRIGGER_INPUT].getVoltage())) newSeedEvent = true;
    if(buttonTrigger(newSeedBtnHigh,params[NEW_SEED_BUTTON_PARAM].getValue())) newSeedEvent = true;
    lights[NEW_SEED_LIGHT].setBrightnessSmooth(newSeedTrigHigh || newSeedBtnHigh ? 1.f : LIGHT_OFF, args.sampleTime);

    outputs[GATE_POLY_OUTPUT].setChannels(8);
    outputs[CLOCK_POLY_OUTPUT].setChannels(10);

    if(newSeedEvent){
      getSeed();
    }

    int maxPhrase = rack::math::clamp(params[BAR_COUNT_PARAM].getValue() + inputs[BAR_COUNT_INPUT].getVoltage()*3.f/2.f, 1.f, 16.f);
    int maxBar = rack::math::clamp(params[BAR_LENGTH_PARAM].getValue() + inputs[BAR_LENGTH_INPUT].getVoltage()*3.f/2.f, 1.f, 16.f);

    //Supress clock events if not running
    if(clockEvent && runGateActive){

      if(resetEvent){
        currentPulse = 0;
        currentCycle = 0;
        currentBar = 0;
        newBar = true;
        newPhrase = true;
        // Reset always delayed until start of clock cycle
        trigGenerator.trigger();
        resetEvent = false;
      }
      else {
        if (++currentPulse % 24 == 0)
          currentCycle++;
        if (currentCycle >= maxBar){
          newBar = true;
          currentCycle = 0;
          currentBar++;
        }
        if (currentBar >= maxPhrase){
          currentPulse = 0;
          currentBar = 0;
          newPhrase = true;
        }
      }
      if (newBar) {
        outputs[START_OF_BAR_OUTPUT].setVoltage(10.f);
        outputs[CLOCK_POLY_OUTPUT].setVoltage(10.f, 9);
      }
      if (newPhrase) {
        outputs[START_OF_PHRASE_OUTPUT].setVoltage(10.f);
        outputs[CLOCK_POLY_OUTPUT].setVoltage(10.f, 8);
        reseedRng();
      }

      bool linearChannelShadow = false;
      bool linearGlobalShadow = false;
      bool offbeatShadow = false;
      int outGateCount = 0;
      for(int si = 0; si < SLIDER_COUNT; si++){
        int gateLength = GATE_LENGTH[ static_cast<int>(params[RATE_PARAM + si].getValue()) ];
        if(currentPulse % gateLength == 0){
          outputs[CLOCK_OUTPUT + si].setVoltage(10.f);
          outputs[CLOCK_POLY_OUTPUT].setVoltage(10.f, si);
          float rndFloat = (rng() >> 32) * 2.32830629e-9f;
          rndFloat = rack::math::clamp(inputs[RNG_OVERRIDE_INPUT].getNormalPolyVoltage(rndFloat, si), 0.f, 10.f - 5e-7f);
          float threshold = rack::math::clamp(
            rack::math::clamp(
              params[DENSITY_PARAM + si].getValue() * (isUni ? 1.f : 2.f) - (isUni ? 0.f : 10.f)
              + inputs[DENSITY_POLY_INPUT].getPolyVoltage(si),
              isUni ? 0.f : -10.f, 10.f
            ) + inputs[DENSITY_CHANNEL_INPUT + si].getVoltage(),
            0.f, 10.f
          );
          bool thresholdMet = rndFloat < threshold;

          int globalMode = rack::math::clamp(
            static_cast<int>(
              inputs[MODE_POLY_INPUT].getNormalPolyVoltage(
                params[MODE_POLY_PARAM].getValue(),
                si
              )
            ),
            0,2
          );
          int channelMode;
          if (si == 0)
            channelMode = ALL_MODE;
          else if (inputs[MODE_CHANNEL_INPUT + si].isConnected())
            channelMode = rack::math::clamp( static_cast <int>(inputs[MODE_CHANNEL_INPUT + si].getVoltage()), 0, 3);
          else
            channelMode = params[MODE_CHANNEL_PARAM + si].getValue();
          if (channelMode == GLOBAL_MODE)
            channelMode = globalMode;

          if (thresholdMet) {
            if ( !params[MUTE_CHANNEL_PARAM + si].getValue() && !params[MUTE_POLY_PARAM].getValue() && (channelMode == ALL_MODE || (channelMode == LINEAR_MODE && !linearChannelShadow) || (channelMode == OFFBEAT_MODE && !offbeatShadow))){
              linearChannelShadow = true;
              outputs[GATE_OUTPUT + si].setVoltage(10.f);
              outputs[GATE_POLY_OUTPUT].setVoltage(10.f, si);
              lights[DENSITY_LIGHT + si].setBrightness(1.f);
              densityLightOn[si] = true;
            }
            if (!params[MUTE_CHANNEL_PARAM + si].getValue() && !params[MUTE_POLY_PARAM].getValue() && (globalMode == ALL_MODE || (globalMode == LINEAR_MODE && !linearGlobalShadow) || (globalMode == OFFBEAT_MODE && !offbeatShadow))){
              linearGlobalShadow = true;
              outGateCount++;
            }
          }
          offbeatShadow = true;
        }
      }

      outputs[GATE_OR_OUTPUT].setVoltage(outGateCount > 0 ? 10.f : 0.f);
      outputs[GATE_XOR_ODD_OUTPUT].setVoltage((outGateCount % 2 == 1) ? 10.f : 0.f);
      outputs[GATE_XOR_ONE_OUTPUT].setVoltage((outGateCount == 1) ? 10.f : 0.f);

    }
    outputs[RESET_TRIGGER_OUTPUT].setVoltage(trigGenerator.process(args.sampleTime) ? 10.f : 0.f);

    if (lightDivider.process()) {
      float lightTime = args.sampleTime * lightDivider.getDivision();
      // Density slider lights off
      for (int i=0; i<SLIDER_COUNT; i++){
        if (!densityLightOn[i])
          lights[DENSITY_LIGHT + i].setBrightnessSmooth(LIGHT_DIM, lightTime);
      }
      //Update Cycle Lights
      for(int ci = 0; ci < MAX_STEP_LENGTH; ci++){
        lights[BAR_STEP_LIGHT + ci].setBrightnessSmooth(
          ci <= currentCycle ? 1.f :
          ci < maxBar ? LIGHT_DIM : 0.f,
          lightTime
        );
        lights[PHRASE_STEP_LIGHT + ci].setBrightnessSmooth(
          ci <= currentBar ? 1.f :
          ci < maxPhrase ? LIGHT_DIM : 0.f,
          lightTime
        );
      }
    }

    outputs[SEED_OUTPUT].setVoltage(internalSeed);
  }

};

struct VCVBezelBig : app::SvgSwitch {
  VCVBezelBig() {
    momentary = true;
    addFrame(Svg::load(asset::plugin(pluginInstance, "res/VCVBezelBig.svg")));
  }
};


template <typename TBase>
struct VCVBezelLightBig : TBase {
  VCVBezelLightBig() {
    this->borderColor = color::BLACK_TRANSPARENT;
    this->bgColor = color::BLACK_TRANSPARENT;
    this->box.size = mm2px(math::Vec(11.1936, 11.1936));
  }
};


template <typename TBase, typename TLight = WhiteLight>
struct LightButton : TBase {
  app::ModuleLightWidget* light;

  LightButton() {
    light = new TLight;
    // Move center of light to center of box
    light->box.pos = this->box.size.div(2).minus(light->box.size.div(2));
    this->addChild(light);
  }

  app::ModuleLightWidget* getLight() {
    return light;
  }
};

using VCVBezelLightBigWhite = LightButton<VCVBezelBig, VCVBezelLightBig<WhiteLight>>;

struct RhythmExplorerWidget : ModuleWidget {

  struct GlowingSvgSwitch : app::SvgSwitch {
    void drawLayer(const DrawArgs& args, int layer) override {
      if (layer==1) {
        if (module && !module->isBypassed()) {
          draw(args);
        }
      }
      app::SvgSwitch::drawLayer(args, layer);
    }
  };

  struct DivSwitch : GlowingSvgSwitch {
    DivSwitch() {
      shadow->opacity = 0.0;
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_0.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_1.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_2.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_3.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_4.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_5.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_6.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_7.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_8.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/rate_9.svg")));
    }
  };

  struct ModeSwitch : GlowingSvgSwitch {
    ModeSwitch() {
      shadow->opacity = 0.0;
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_0.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_1.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_2.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/mode_3.svg")));
    }
  };

  RhythmExplorerWidget(RhythmExplorer* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, faceplatePath(moduleName, module ? module->currentThemeStr() : themes[getDefaultTheme()]))));

    float dx = RACK_GRID_WIDTH * 2.f;
    float dy = RACK_GRID_WIDTH * 2.f;

    float yStart = RACK_GRID_WIDTH*2.6f;
    float xStart = RACK_GRID_WIDTH;

    float x = xStart + dx * 0.75f;
    float y = yStart + dy;

    addParam(createLightParamCentered<VCVBezelLightBigWhite>(Vec(x,y), module, RhythmExplorer::NEW_SEED_BUTTON_PARAM, RhythmExplorer::NEW_SEED_LIGHT));
    y += dy + dy;
    addParam(createLightParamCentered<VCVBezelLightBigWhite>(Vec(x,y), module, RhythmExplorer::RESET_BUTTON_PARAM, RhythmExplorer::RESET_LIGHT));
    y += dy + dy;
    addParam(createLightParamCentered<VCVBezelLightBigWhite>(Vec(x,y), module, RhythmExplorer::RUN_GATE_PARAM, RhythmExplorer::RUN_GATE_LIGHT));
    
    x = xStart + dx/2.f;
    y = yStart + dy * 7.6f;
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(Vec(x,y), module, RhythmExplorer::PATOFF_PARAM, RhythmExplorer::PATOFF_LIGHT));

    x = xStart + dx * 2.5f;
    y = yStart + dy * 3.5f;
    for(int li = 0; li < 8; li++, y-=dy/2.f){
      addChild(createLightCentered<MediumLight<YellowLight>>(Vec(x-dx/5.f,y), module, RhythmExplorer::PHRASE_STEP_LIGHT + li));
      addChild(createLightCentered<MediumLight<YellowLight>>(Vec(x+dx/5.f,y), module, RhythmExplorer::PHRASE_STEP_LIGHT + li + 8));
      addChild(createLightCentered<MediumLight<YellowLight>>(Vec(x+dx-dx/5.f,y), module, RhythmExplorer::BAR_STEP_LIGHT + li));
      addChild(createLightCentered<MediumLight<YellowLight>>(Vec(x+dx+dx/5.f,y), module, RhythmExplorer::BAR_STEP_LIGHT + li + 8));
    }

    y = yStart + dy * 4.6f;
    addParam(createParamCentered<RotarySwitch<RoundSmallBlackKnob>>(Vec(x,y), module, RhythmExplorer::BAR_COUNT_PARAM));
    addParam(createParamCentered<RotarySwitch<RoundSmallBlackKnob>>(Vec(x+dx,y), module, RhythmExplorer::BAR_LENGTH_PARAM));
    y += dy;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::BAR_COUNT_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(x+dx,y), module, RhythmExplorer::BAR_LENGTH_INPUT));
    y += dy + dy;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::START_OF_PHRASE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(x+dx,y), module, RhythmExplorer::START_OF_BAR_OUTPUT));

    x += dx * 2.5f;

    for(int si = 0; si < SLIDER_COUNT; si++, x+=dx){
      y = yStart;
      DivSwitch* divSwitch = createParamCentered<DivSwitch>(Vec(x,y), module, RhythmExplorer::RATE_PARAM + si);
      if (!module) {
        divSwitch->sw->svg = divSwitch->frames[si+1];
      }
      addParam(divSwitch);
      y += dy * 1.8f;
      addParam(createLightParamCentered<VCVLightSlider<YellowLight>>(Vec(x,y), module, RhythmExplorer::DENSITY_PARAM + si, RhythmExplorer::DENSITY_LIGHT + si));
      y += dy * 1.8f;
      addParam(createParamCentered<ModeSwitch>(Vec(x,y), module, RhythmExplorer::MODE_CHANNEL_PARAM + si));
      y += dy;
      addParam(createLightParamCentered<VCVLightBezelLatch<MediumSimpleLight<WhiteLight>>>(Vec(x, y), module, RhythmExplorer::MUTE_CHANNEL_PARAM + si, RhythmExplorer::MUTE_CHANNEL_LIGHT + si));
      y += dy;
      addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::DENSITY_CHANNEL_INPUT + si));
      y += dy;
      if (si > 0)
        addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::MODE_CHANNEL_INPUT + si));
      y += dy;
      addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::CLOCK_OUTPUT + si));
      y += dy;
      addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::GATE_OUTPUT + si));
    }
    addParam(createParam<CKSS>(Vec(x, yStart + dy*0.75f), module, RhythmExplorer::POLAR_PARAM));
    x += dx * 0.5f;
    y = yStart + dy * 3.6f;
    addParam(createParamCentered<ModeSwitch>(Vec(x,y), module, RhythmExplorer::MODE_POLY_PARAM));
    y += dy;
    addParam(createLightParamCentered<VCVLightBezelLatch<MediumSimpleLight<WhiteLight>>>(Vec(x, y), module, RhythmExplorer::MUTE_POLY_PARAM, RhythmExplorer::MUTE_POLY_LIGHT));
    y += dy;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::DENSITY_POLY_INPUT));
    y += dy;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::MODE_POLY_INPUT));
    y += dy;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::CLOCK_POLY_OUTPUT));
    y += dy;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::GATE_POLY_OUTPUT));
    x += dx * 1.5f;
    y = yStart;
    addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(Vec(x,y), module, RhythmExplorer::LOCK_PARAM, RhythmExplorer::LOCK_LIGHT));
    y += dy * 1.82f;
    addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(Vec(x,y), module, RhythmExplorer::INIT_DENSITY_PARAM, RhythmExplorer::INIT_DENSITY_LIGHT));

    x = xStart + dx * 2.5f;
    dx *= 1.1f;
    x -= dx/2.f;
    y = yStart + dy * 10.1f;

    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::CLOCK_INPUT));
    x += dx;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::RNG_OVERRIDE_INPUT));
    x += dx;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::SEED_INPUT));
    x += dx;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::NEW_SEED_TRIGGER_INPUT));
    x += dx;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::RESET_TRIGGER_INPUT));
    x += dx;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::RUN_GATE_INPUT));
    x += dx * 1.5f;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::GATE_OR_OUTPUT));
    x += dx;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::GATE_XOR_ODD_OUTPUT));
    x += dx;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::GATE_XOR_ONE_OUTPUT));
    x += dx;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::SEED_OUTPUT));
    x += dx;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::RESET_TRIGGER_OUTPUT));
    x += dx;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::RUN_GATE_OUTPUT));

  }

  void appendContextMenu(Menu* menu) override {
    RhythmExplorer* module = dynamic_cast<RhythmExplorer*>(this->module);
    assert(module);
    #include "ThemeMenu.hpp"
  }

  #include "ThemeStep.hpp"
};


Model* modelRhythmExplorer = createModel<RhythmExplorer, RhythmExplorerWidget>("RhythmExplorer");

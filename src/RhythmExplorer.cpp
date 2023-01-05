#include "plugin.hpp"
#include "util.hpp"

#define SLIDER_COUNT 8
#define MAX_STEP_LENGTH 16
#define MAX_UNIT64 18446744073709551615ul
#define LIGHT_OFF 0.02f
#define LIGHT_DIM 0.1f

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
  bool channelMuteActive[SLIDER_COUNT];
  bool polyMuteActive;

  //Non Persistant State
  int currentPulse;
  int currentCycle;
  int currentBar;
  rack::random::Xoroshiro128Plus rng;
  bool clockHigh;
  bool resetTrigHigh;
  bool resetBtnHigh;
  bool newSeedBtnHigh;
  bool newSeedTrigHigh;
  bool runGateBtnHigh;
  bool runGateTrigHigh;
  bool channelMuteHigh[SLIDER_COUNT];
  bool polyMuteHigh;

  RhythmExplorer() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configInput(CLOCK_INPUT,"Clock");
    configInput(NEW_SEED_TRIGGER_INPUT,"Dice Trigger");
    configInput(RESET_TRIGGER_INPUT,"Reset Trigger");
    configInput(SEED_INPUT,"Seed");
    configInput(RNG_OVERRIDE_INPUT,"Random Override");
    configOutput(SEED_OUTPUT,"Seed Output");

    configButton(NEW_SEED_BUTTON_PARAM, "Dice Trigger");
    configButton(RESET_BUTTON_PARAM,"Reset Trigger");

    configButton(RUN_GATE_PARAM, "Run Gate");
    configInput(RUN_GATE_INPUT,"Run Gate");

    configOutput(GATE_OR_OUTPUT,"OR Trigger");
    configOutput(GATE_XOR_ODD_OUTPUT,"XOR Odd Parity Trigger");
    configOutput(GATE_XOR_ONE_OUTPUT,"XOR 1 Hot Trigger");

    configParam(BAR_COUNT_PARAM, 1, MAX_STEP_LENGTH, 1, "Phrase Bar Count");
    configParam(BAR_LENGTH_PARAM, 1, MAX_STEP_LENGTH, 4, "Bar 1/4 Count");
    configInput(BAR_COUNT_INPUT,"Phrase Bar Count CV");
    configInput(BAR_LENGTH_INPUT,"Bar 1/4 Count CV");
    configOutput(START_OF_BAR_OUTPUT,"Start of Measure Trigger");
    configOutput(START_OF_PHRASE_OUTPUT,"Start of Cycle Trigger");

    for(int si = 0; si < SLIDER_COUNT; si++){
      std::string si_s = std::to_string(si+1);
      configSwitch(RATE_PARAM + si, 0, 9, si+1, "Rate " + si_s, CHANNEL_DIVISION_LABELS);
      configParam(DENSITY_PARAM + si, 0.f, 10.f, 0.f, "Density " + si_s, " V");
      lights[DENSITY_LIGHT + si].setBrightness(LIGHT_DIM);
      configOutput(GATE_OUTPUT + si, "Gate " + si_s);
      configOutput(CLOCK_OUTPUT + si, "Clock " + si_s);
      configInput(DENSITY_CHANNEL_INPUT + si, "Density CV " + si_s);
      configInput(MODE_CHANNEL_INPUT + si, "Mode CV " + si_s);
      configButton(MUTE_CHANNEL_PARAM + si, "Mute " + si_s);
      configSwitch(MODE_CHANNEL_PARAM + si, 0, 3 , 0, "Mode " + si_s, CHANNEL_MODE_LABELS);
    }
    configSwitch(MODE_POLY_PARAM, 0, 2 , 0, "Global Mode", POLY_MODE_LABELS);
    configButton(MUTE_POLY_PARAM, "Global Mute");
    configInput(MODE_POLY_INPUT, "Global Mode CV");
    configInput(DENSITY_POLY_INPUT,"Density Poly CV");
    configOutput(CLOCK_POLY_OUTPUT, "Clock Poly");
    configOutput(GATE_POLY_OUTPUT, "Gate Poly");

    initialize();
  }

  void onReset(const ResetEvent& e) override {
    Module::onReset(e);
    initialize();
  }

  void initialize(){
    currentPulse = 0;
    currentCycle = 0;
    currentBar = 0;
    rng = {};
    runGateActive = true;
    clockHigh = false;
    resetTrigHigh = false;
    resetBtnHigh = false;
    newSeedBtnHigh = false;
    newSeedTrigHigh = false;
    runGateBtnHigh = false;
    runGateTrigHigh = false;
    for (int i=0; i<SLIDER_COUNT; i++)
      channelMuteHigh[i] = false;
    polyMuteHigh = false;
    getSeed();
    reseedRng();
    updateLights();
  }

  json_t *dataToJson() override{
    json_t *jobj = json_object();

    json_object_set_new(jobj, "internalSeed", json_real(internalSeed));
    json_object_set_new(jobj, "runGateActive", json_bool(runGateActive));
    json_object_set_new(jobj, "channelMuteActive0", json_bool(channelMuteActive[0]));
    json_object_set_new(jobj, "channelMuteActive1", json_bool(channelMuteActive[1]));
    json_object_set_new(jobj, "channelMuteActive2", json_bool(channelMuteActive[2]));
    json_object_set_new(jobj, "channelMuteActive3", json_bool(channelMuteActive[3]));
    json_object_set_new(jobj, "channelMuteActive4", json_bool(channelMuteActive[4]));
    json_object_set_new(jobj, "channelMuteActive5", json_bool(channelMuteActive[5]));
    json_object_set_new(jobj, "channelMuteActive6", json_bool(channelMuteActive[6]));
    json_object_set_new(jobj, "channelMuteActive7", json_bool(channelMuteActive[7]));
    json_object_set_new(jobj, "polyMuteActive", json_bool(polyMuteActive));

    return jobj;
  }

  void dataFromJson(json_t *jobj) override {

    internalSeed = json_real_value(json_object_get(jobj, "internalSeed"));
    runGateActive = json_is_true(json_object_get(jobj, "runGateActive"));
    channelMuteActive[0] = json_is_true(json_object_get(jobj, "channelMuteActive0"));
    channelMuteActive[1] = json_is_true(json_object_get(jobj, "channelMuteActive1"));
    channelMuteActive[2] = json_is_true(json_object_get(jobj, "channelMuteActive2"));
    channelMuteActive[3] = json_is_true(json_object_get(jobj, "channelMuteActive3"));
    channelMuteActive[4] = json_is_true(json_object_get(jobj, "channelMuteActive4"));
    channelMuteActive[5] = json_is_true(json_object_get(jobj, "channelMuteActive5"));
    channelMuteActive[6] = json_is_true(json_object_get(jobj, "channelMuteActive6"));
    channelMuteActive[7] = json_is_true(json_object_get(jobj, "channelMuteActive7"));
    polyMuteActive = json_is_true(json_object_get(jobj, "polyMuteActive"));

    reseedRng();
    updateLights();
  }

  void updateLights(){
    lights[RUN_GATE_LIGHT].setBrightness(runGateActive ? 1.f : LIGHT_OFF);
    for (int i=0; i<SLIDER_COUNT; i++)
      lights[MUTE_CHANNEL_LIGHT + i].setBrightness(channelMuteActive[i] ? 1.f : LIGHT_OFF);
    lights[MUTE_POLY_LIGHT].setBrightness(polyMuteActive ? 1.f : LIGHT_OFF);
  }

  void process(const ProcessArgs& args) override {    

    //Clock Logic
    bool newBar = false;
    bool newPhrase = false;

    bool oldClockHigh = clockHigh;
    bool clockEvent = schmittTrigger(clockHigh,inputs[CLOCK_INPUT].getVoltage());
    if (oldClockHigh && !clockHigh) { // clear triggers
      outputs[GATE_OR_OUTPUT].setVoltage(0.f);
      outputs[GATE_XOR_ODD_OUTPUT].setVoltage(0.f);
      outputs[GATE_XOR_ONE_OUTPUT].setVoltage(0.f);
      for (int i=0; i<SLIDER_COUNT; i++){
        outputs[GATE_OUTPUT + i].setVoltage(0.f);
        outputs[GATE_POLY_OUTPUT].setVoltage(0.f, i);
        outputs[CLOCK_OUTPUT + i].setVoltage(0.f);
        outputs[CLOCK_POLY_OUTPUT].setVoltage(0.f, i);
        lights[DENSITY_LIGHT + i].setBrightness(LIGHT_DIM);
      }
    }

    bool resetEvent = false;

    if (inputs[RUN_GATE_INPUT].isConnected()) {
      schmittTrigger(runGateTrigHigh,inputs[RUN_GATE_INPUT].getVoltage());
      buttonTrigger(runGateBtnHigh,params[RUN_GATE_PARAM].getValue());
      if ((runGateTrigHigh ^ runGateBtnHigh) != runGateActive){
        runGateActive = !runGateActive;
        if (runGateActive)
          resetEvent = true;
        lights[RUN_GATE_LIGHT].setBrightness(runGateActive ? 1 : LIGHT_OFF);
      }
    }
    else {
      if(buttonTrigger(runGateBtnHigh,params[RUN_GATE_PARAM].getValue())){
        runGateActive = !runGateActive;
        if (runGateActive)
          resetEvent = true;
        lights[RUN_GATE_LIGHT].setBrightness(runGateActive ? 1 : LIGHT_OFF);
      }
    }

    if(schmittTrigger(resetTrigHigh,inputs[RESET_TRIGGER_INPUT].getVoltage())) resetEvent = true;
    if(buttonTrigger(resetBtnHigh,params[RESET_BUTTON_PARAM].getValue())) resetEvent = true;
    lights[RESET_LIGHT].setBrightness(resetTrigHigh || resetBtnHigh ? 1.f : LIGHT_OFF);

    bool newSeedEvent = false;
    if(schmittTrigger(newSeedTrigHigh,inputs[NEW_SEED_TRIGGER_INPUT].getVoltage())) newSeedEvent = true;
    if(buttonTrigger(newSeedBtnHigh,params[NEW_SEED_BUTTON_PARAM].getValue())) newSeedEvent = true;
    lights[NEW_SEED_LIGHT].setBrightness(newSeedTrigHigh || newSeedBtnHigh ? 1.f : LIGHT_OFF);

    // Mutes
    for (int i=0; i<SLIDER_COUNT; i++) {
      if(buttonTrigger(channelMuteHigh[i],params[MUTE_CHANNEL_PARAM + i].getValue())) {
        channelMuteActive[i] = !channelMuteActive[i];
        lights[MUTE_CHANNEL_LIGHT + i].setBrightness(channelMuteActive[i] ? 1.f : LIGHT_OFF);
      }
    }
    if(buttonTrigger(polyMuteHigh,params[MUTE_POLY_PARAM].getValue())) {
      polyMuteActive = !polyMuteActive;
      lights[MUTE_POLY_LIGHT].setBrightness(polyMuteActive ? 1.f : LIGHT_OFF);
    }

    bool newCycle = false;
    bool endOfCycle = false;

    outputs[GATE_POLY_OUTPUT].setChannels(8);
    outputs[CLOCK_POLY_OUTPUT].setChannels(8);

    if(resetEvent){
      currentPulse = 0;
      currentCycle = 0;
      currentBar = 0;
      endOfCycle = true;
      newBar = true;
      newPhrase = true;
    }

    if(newSeedEvent){
      getSeed();
    }

    //Supress clock events if runGate does not match theRunGatTrig
    if(clockEvent && runGateActive){

      if (!resetEvent)
        currentPulse++;
      if (currentPulse % 24 == 0){
        newCycle = true;
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
          rndFloat = rack::math::clamp(inputs[RNG_OVERRIDE_INPUT].getNormalPolyVoltage(rndFloat, si), 0.f, 10.f);
          float threshold = rack::math::clamp(
            rack::math::clamp(
              params[DENSITY_PARAM + si].getValue()
              + inputs[DENSITY_POLY_INPUT].getPolyVoltage(si),
              0.f, 10.f
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
          int channelMode = rack::math::clamp(
            static_cast<int>(
              inputs[MODE_CHANNEL_INPUT + si].getNormalPolyVoltage(
                params[MODE_CHANNEL_PARAM + si].getValue(),
                si
              )
            ),
            0,3
          );
          if (channelMode == GLOBAL_MODE)
            channelMode = globalMode;

          if (thresholdMet) {
            if (channelMode == ALL_MODE || (channelMode == LINEAR_MODE && !linearChannelShadow) || (channelMode == OFFBEAT_MODE && !offbeatShadow)){
              linearChannelShadow = true;
              if (!channelMuteActive[si] && !polyMuteActive) {
                outputs[GATE_OUTPUT + si].setVoltage(10.f);
                outputs[GATE_POLY_OUTPUT].setVoltage(10.f, si);
                lights[DENSITY_LIGHT + si].setBrightness(1.f);
              }
            }
            if (!polyMuteActive && (globalMode == ALL_MODE || (globalMode == LINEAR_MODE && !linearGlobalShadow) || (globalMode == OFFBEAT_MODE && !offbeatShadow))){
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

    int maxPhrase = params[BAR_COUNT_PARAM].getValue();
    int maxBar = params[BAR_LENGTH_PARAM].getValue();
    if(newCycle){
      if (!resetEvent)
        currentCycle++;
      if (currentCycle >= maxBar){
        newBar = true;
        currentCycle = 0;
        currentBar++;
      }
    }
    if (currentBar >= maxPhrase){
      currentPulse = 0;
      currentBar = 0;
      endOfCycle = true;
      newPhrase = true;
    }
    outputs[START_OF_PHRASE_OUTPUT].setVoltage(newPhrase ? 10.f : 0.f);
    outputs[START_OF_BAR_OUTPUT].setVoltage(newBar ? 10.f : 0.f);

    if(endOfCycle){
      reseedRng();
    }

    //Update Cycle Lights
    for(int ci = 0; ci < MAX_STEP_LENGTH; ci++){
      lights[BAR_STEP_LIGHT + ci].setBrightness(
        ci <= currentCycle ? 1.f :
        ci < maxBar ? LIGHT_DIM : 0.f
      );
      lights[PHRASE_STEP_LIGHT + ci].setBrightness(
        ci <= currentBar ? 1.f :
        ci < maxPhrase ? LIGHT_DIM : 0.f
      );
    }

    outputs[SEED_OUTPUT].setVoltage(internalSeed);
  }
  
  void getSeed(){
    internalSeed = inputs[SEED_INPUT].isConnected() ? rack::math::clamp(inputs[SEED_INPUT].getVoltage(), 0.f, 10.f) : rack::math::clamp(rack::random::uniform() * 10.f, 1e-19f, 10.f);
    if (internalSeed < 1e-19f)
      internalSeed = 0.f;
  }

  void reseedRng(){
    if (internalSeed > 0.f) {
      float seed1 = internalSeed / 10.f;
      float seed2 = std::fmod(internalSeed,1.f);
      uint64_t s1 = seed1 * MAX_UNIT64;
      uint64_t s2 = seed2 * MAX_UNIT64;
      rng.seed(s1, s2);
    }
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

  struct RateSwitch : app::SvgSwitch {
    RateSwitch() {
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

  struct ModeSwitch : app::SvgSwitch {
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
    setPanel(createPanel(asset::plugin(pluginInstance, "res/RhythmExplorer.svg")));

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

    x = xStart + dx * 2.5f;
    y = yStart + dy * 3.5f;
    for(int li = 0; li < 8; li++, y-=dy/4.f){
      addChild(createLightCentered<MediumLight<YellowLight>>(Vec(x-dx/5.f,y), module, RhythmExplorer::PHRASE_STEP_LIGHT + li));
      addChild(createLightCentered<MediumLight<YellowLight>>(Vec(x+dx/5.f,y), module, RhythmExplorer::PHRASE_STEP_LIGHT + li + 8));
      addChild(createLightCentered<MediumLight<YellowLight>>(Vec(x+dx-dx/5.f,y), module, RhythmExplorer::BAR_STEP_LIGHT + li));
      addChild(createLightCentered<MediumLight<YellowLight>>(Vec(x+dx+dx/5.f,y), module, RhythmExplorer::BAR_STEP_LIGHT + li + 8));
      y -= dy/4.f;
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
      addParam(createParamCentered<RateSwitch>(Vec(x,y), module, RhythmExplorer::RATE_PARAM + si));
      y += dy * 1.8f;
      addParam(createLightParamCentered<VCVLightSlider<YellowLight>>(Vec(x,y), module, RhythmExplorer::DENSITY_PARAM + si, RhythmExplorer::DENSITY_LIGHT + si));
      y += dy * 1.8f;
      addParam(createParamCentered<ModeSwitch>(Vec(x,y), module, RhythmExplorer::MODE_CHANNEL_PARAM + si));
      y += dy;
      addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(x, y), module, RhythmExplorer::MUTE_CHANNEL_PARAM + si, RhythmExplorer::MUTE_CHANNEL_LIGHT + si));
      y += dy;
      addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::DENSITY_CHANNEL_INPUT + si));
      y += dy;
      addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::MODE_CHANNEL_INPUT + si));
      y += dy;
      addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::CLOCK_OUTPUT + si));
      y += dy;
      addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::GATE_OUTPUT + si));
    }
    x += dx * 0.5f;
    y = yStart + dy * 3.6f;
    addParam(createParamCentered<ModeSwitch>(Vec(x,y), module, RhythmExplorer::MODE_POLY_PARAM));
    y += dy;
    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(Vec(x, y), module, RhythmExplorer::MUTE_POLY_PARAM, RhythmExplorer::MUTE_POLY_LIGHT));
    y += dy;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::DENSITY_POLY_INPUT));
    y += dy;
    addInput(createInputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::MODE_POLY_INPUT));
    y += dy;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::CLOCK_POLY_OUTPUT));
    y += dy;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::GATE_POLY_OUTPUT));

    x = xStart + dx * 2.5f;
    dx *= 1.1f;
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
    x += dx * 1.5f;
    addOutput(createOutputCentered<PJ301MPort>(Vec(x,y), module, RhythmExplorer::SEED_OUTPUT));

  }

  // void appendContextMenu(Menu* menu) override {
  //  RhythmExplorer* module = dynamic_cast<RhythmExplorer*>(this->module);

  //  menu->addChild(new MenuEntry); //Blank Row
  //  menu->addChild(createMenuLabel("RhythmExplorer"));
  // }
};


Model* modelRhythmExplorer = createModel<RhythmExplorer, RhythmExplorerWidget>("RhythmExplorer");

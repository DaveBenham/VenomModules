// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"
#include "BenjolinModule.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

struct BenjolinOsc : BenjolinModule {
  
  enum ParamId {
    OVER_PARAM,
    FREQ1_PARAM,
    FREQ2_PARAM,
    RUNG1_PARAM,
    RUNG2_PARAM,
    CV1_PARAM,
    CV2_PARAM,
    PATTERN_PARAM,
    CHAOS_PARAM,
    DOUBLE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    CV1_INPUT,
    CV2_INPUT,
    CHAOS_INPUT,
    DOUBLE_INPUT,
    CLOCK_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    TRI1_OUTPUT,
    TRI2_OUTPUT,
    PULSE1_OUTPUT,
    PULSE2_OUTPUT,
    XOR_OUTPUT,
    PWM_OUTPUT,
    RUNG_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    CHAOS_LIGHT,
    DOUBLE_LIGHT,
    LIGHTS_LEN
  };
  
  int oversample = -1;
  int dacMode=0, dacBit1=2, dacBit2=8, dacBit3=64, dacShift1=1, dacShift2=2, dacShift3=4;
  std::vector<int> oversampleValues = {1,2,4,8,16,32};
  OversampleFilter_4 upSample, downSampleA, downSampleB;
  dsp::SchmittTrigger clockTrig;
  simd::float_4 osc{0.f,0.f,1.f,1.f}, triMask{1.f,1.f,0.f,0.f}, dir{1.f,1.f,0.f,0.f},
                in{}, outA{}, outB{};
  float *tri1=&osc[0], *tri2=&osc[1], *pul1=&osc[2], *pul2=&osc[3], *dir1=&dir[0], *dir2=&dir[1],
        *tri1Out=&outA[0], *tri2Out=&outA[1], *pul1Out=&outA[2], *pul2Out=&outA[3], 
        *xorOut=&outB[0], *pwmOut=&outB[1], *rungOut=&outB[2],
        *cv1In=&in[0], *cv2In=&in[1], *clockIn=&in[2],
        normScale=5.f,
        xorVal=0, rung=0;
  unsigned char asr = rack::random::uniform()*126+1;
  bool origNormScale=false, chaosIn=false, dblIn=false, unipolarClock=false;
 
  BenjolinOsc() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    configParam(FREQ1_PARAM, -9.3f, 9.0f, 0.f, "Oscillator 1 frequency");
    configParam(FREQ2_PARAM, -9.3f, 9.0f, 0.f, "Oscillator 2 frequency");
    configParam(RUNG1_PARAM, -1.f, 1.f, 0.f, "Oscillator 1 rungler modulation amount");
    configParam(RUNG2_PARAM, -1.f, 1.f, 0.f, "Oscillator 2 rungler modulation amount");
    configParam(CV1_PARAM, -1.f, 1.f, 0.f, "Oscillator 1 CV modulation amount");
    configParam(CV2_PARAM, -1.f, 1.f, 0.f, "Oscillator 2 CV modulation amount");
    configInput(CV1_INPUT, "Oscillator 1 CV modulation")->description = "Normalled to oscillator 2 triangle output";
    configInput(CV2_INPUT, "Oscillator 2 CV modulation")->description = "Normalled to oscillator 1 triangle output";
    configOutput(TRI1_OUTPUT, "Oscillator 1 triangle");
    configOutput(TRI2_OUTPUT, "Oscillator 2 triangle");
    configOutput(PULSE1_OUTPUT, "Oscillator 1 pulse");
    configOutput(PULSE2_OUTPUT, "Oscillator 2 pulse");
    configParam(PATTERN_PARAM, -1.f, 1.f, 0.f, "Rungler cycle chance");
    configSwitch<FixedSwitchQuantity>(CHAOS_PARAM, 0.f, 1.f, 0.f, "Maximum chaos", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(DOUBLE_PARAM, 0.f, 1.f, 0.f, "Double clock", {"Off", "On"});
    configInput(CHAOS_INPUT,"Maximum chaos trigger");
    configInput(DOUBLE_INPUT,"Double clock trigger");
    configInput(CLOCK_INPUT,"Clock")->description = "Normalled to oscillator 2 pulse output";
    configOutput(XOR_OUTPUT,"XOR");
    configOutput(PWM_OUTPUT,"PWM");
    configOutput(RUNG_OUTPUT,"Rungler");
  }

  void onSampleRateChange() override {
    float rate = APP->engine->getSampleRate();
    std::vector< std::string > labels;
    int maxOver;
    if (rate>384000.f) {
      maxOver = 1;
      labels = {"Off", "x2"};
    }
    else if (rate>192000.f) {
      maxOver = 2;
      labels = {"Off", "x2", "x4"};
    }
    else if (rate>96000.f) {
      maxOver = 3;
      labels = {"Off", "x2", "x4", "x8"};
    }
    else if (rate>48000.f) {
      maxOver = 4;
      labels = {"Off", "x2", "x4", "x8", "x16"};
    }
    else {
      maxOver = 5;
      labels = {"Off", "x2", "x4", "x8", "x16", "x32"};
    }
    if (params[OVER_PARAM].getValue()>maxOver)
      params[OVER_PARAM].setValue(maxOver);
    SwitchQuantity *switchQuant = static_cast<SwitchQuantity*>(paramQuantities[OVER_PARAM]);
    switchQuant->maxValue = maxOver;
    switchQuant->labels = labels;
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (oversample != oversampleValues[params[OVER_PARAM].getValue()]) {
      oversample = oversampleValues[params[OVER_PARAM].getValue()];
      upSample.setOversample(oversample);
      downSampleA.setOversample(oversample);
      downSampleB.setOversample(oversample);
    }
    simd::float_4 k = triMask * 60.f * args.sampleTime / oversample;
    bool cv1Connected = inputs[CV1_INPUT].isConnected(),
         cv2Connected = inputs[CV2_INPUT].isConnected(),
         clockConnected = inputs[CLOCK_INPUT].isConnected(),
         chaos = params[CHAOS_PARAM].getValue(),
         dbl = params[DOUBLE_PARAM].getValue();
    int trig=0;
    *cv1In = inputs[CV1_INPUT].getVoltage();
    *cv2In = inputs[CV2_INPUT].getVoltage();
    *clockIn = inputs[CLOCK_INPUT].getVoltage();
    if ((inputs[CHAOS_INPUT].getVoltage()>=2.f) != chaosIn) {
      chaosIn = !chaosIn;
      if (chaosIn) {
        chaos = !chaos;
        params[CHAOS_PARAM].setValue(chaos);
      }
    }
    if ((inputs[DOUBLE_INPUT].getVoltage()>=2.f) != dblIn) {
      dblIn = !dblIn;
      if (dblIn) {
        dbl = !dbl;
        params[DOUBLE_PARAM].setValue(dbl);
      }
    }
    for (int o=0; o<oversample; o++){
      if (oversample>1 && (cv1Connected || cv2Connected || clockConnected)){
        in = upSample.process(o ? simd::float_4::zero() : in * oversample);
      }
      simd::float_4 freq = {
        params[FREQ1_PARAM].getValue() + 0.0001f + (cv1Connected ? *cv1In : *tri2*normScale) * 0.9f * params[CV1_PARAM].getValue() + rung * 0.9f * params[RUNG1_PARAM].getValue(),
        params[FREQ2_PARAM].getValue() + (cv2Connected ? *cv2In : *tri1*normScale) * 0.9f * params[CV2_PARAM].getValue() + rung * 0.9f * params[RUNG2_PARAM].getValue(),
        0.f,0.f
      };
      freq = simd::clamp(freq, -9.3f, 9.7f);
      osc += dsp::exp2_taylor5(freq) * k * dir;
      if (*tri1 > 1.f || *tri1 < -1.f) {
        *tri1 = *dir1 + *dir1 - *tri1;
        *dir1 *= -1.f;
      }  
      if (*tri2 > 1.f || *tri2 < -1.f) {
        *tri2 = *dir2 + *dir2 - *tri2;
        *dir2 *= -1.f;
      }
      if (*pul1 != math::sgn(*tri1)) *pul1*=-1.f;
      if (*pul2 != math::sgn(*tri2)) *pul2*=-1.f;
      *pwmOut = *tri2>*tri1 ? 5.f : -5.f;
      trig = clockTrig.processEvent( clockConnected ? *clockIn : *pul2*normScale, unipolarClock?0.1f:-1.f, 1.f);
      if (trig>0 || (trig && dbl)){
        float ptrn = chaos ? 0.5f : params[PATTERN_PARAM].getValue();
        unsigned char data = (ptrn>=*tri1 || ptrn>=10.f) ^ (chaos ? ((asr&32)>>5)^((asr&64)>>6) : (asr&128)>>7);
        asr = (asr<<1)|data;
        xorVal = asr&1 ? 5.f : -5.f;
        rung = (((asr&dacBit1)>>dacShift1)+((asr&dacBit2)>>dacShift2)+((asr&dacBit3)>>dacShift3)) * 1.428571f - 5.f;
      }
      *xorOut = xorVal;
      *rungOut = rung;
      if (oversample>1) {
        outA = downSampleA.process(osc*5.f);
        outB = downSampleB.process(outB);
      }
      else
        outA = osc*5.f;
    }
    for (BenjolinModule* expndr = rightExpander; expndr; expndr = expndr->rightExpander){
      if (!expndr->isBypassed() && expndr->model == modelBenjolinGatesExpander){
        BenjolinGatesExpander* gates = static_cast<BenjolinGatesExpander*>(expndr);
        float hi = gates->params[GATES_POLARITY_PARAM].getValue() ? 5.f : 10.f;
        float lo = gates->params[GATES_POLARITY_PARAM].getValue() ? -5.f : 0.f;
        int mode = static_cast<int>(gates->params[GATES_MODE_PARAM].getValue());
        unsigned char val;
        for (int i=0; i<8; i++){
          val = asr & gates->gateBits[i];
          switch (gates->gateLogic[i]){
            case BenjolinGatesExpander::AND:
              if (val != gates->gateBits[i]) val = 0;
              break;
            case BenjolinGatesExpander::XOR:
              val = gates->setCount(val) == 1;
              break;
          }
          switch (mode){
            // case 0: gate do nothing
            case 1: // clock gate
              val = (val && clockTrig.isHigh());
              break;
            case 2: // inverse clock gate
              val = (val && !clockTrig.isHigh());
              break;
            case 3: // trigger
              if (val != gates->oldVal[i]) {
                if (val) gates->trigGenerator[i].trigger();
                gates->oldVal[i] = val;
              }
              break;
            case 4: // clock rise trigger
              if (val && trig>0) gates->trigGenerator[i].trigger();
              break;
            case 5: // clock fall trigger
              if (val && trig<0) gates->trigGenerator[i].trigger();
              break;
            case 6: // clock edge trigger
              if (val && trig) gates->trigGenerator[i].trigger();
              break;
          }
          if (mode >= 3 /*trigger*/) {
            if (val)
              val = gates->trigGenerator[i].process(args.sampleTime);
            else
              gates->trigGenerator[i].reset();
          }
          gates->outputs[i].setVoltage(val ? hi : lo);
          gates->lights[GATE_LIGHT+i].setBrightnessSmooth(val!=0, args.sampleTime);
        }
      }
      if (!expndr->isBypassed() && expndr->model == modelBenjolinVoltsExpander){
        BenjolinVoltsExpander* volts = static_cast<BenjolinVoltsExpander*>(expndr);
        float val = 0.f;
        float div = 0.f;
        for (int i=0; i<8; i++){
          float v = volts->getBitValue(VOLT_PARAM+i);
          div += v;
          if (asr & (1<<i)){
            val += v;
          }
        }
        if (div)
          val = (val/div - 0.5f) * volts->params[VOLTS_RANGE_PARAM].getValue();
        volts->outputs[VOLTS_OUTPUT].setVoltage(val + volts->params[VOLTS_OFFSET_PARAM].getValue());
      }
    }
    outputs[TRI1_OUTPUT].setVoltage(*tri1Out);
    outputs[TRI2_OUTPUT].setVoltage(*tri2Out);
    outputs[PULSE1_OUTPUT].setVoltage(*pul1Out);
    outputs[PULSE2_OUTPUT].setVoltage(*pul2Out);
    outputs[XOR_OUTPUT].setVoltage(*xorOut);
    outputs[PWM_OUTPUT].setVoltage(*pwmOut);
    outputs[RUNG_OUTPUT].setVoltage(*rungOut);
  }
  
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "origNormScale", json_boolean(origNormScale));
    json_object_set_new(rootJ, "unipolarClock", json_boolean(unipolarClock));
    json_object_set_new(rootJ, "dacMode", json_integer(dacMode));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t *val;
    if ((val = json_object_get(rootJ, "origNormScale")))
      origNormScale = json_boolean_value(val);
    else
      origNormScale=true;
    normScale = origNormScale ? 1.f : 5.f;
    if ((val = json_object_get(rootJ, "unipolarClock")))
      unipolarClock = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "dacMode"))){
      setDacMode(json_integer_value(val));
      dacBit1 = dacMode ? 16 : 2;
      dacBit2 = dacMode ? 32 : 8;
      dacBit3 = 64;
      dacShift1 = dacMode ? 4 : 1;
      dacShift2 = dacMode ? 4 : 2;
      dacShift3 = 4;
    }
  }

  void setDacMode(int mode) {
    dacMode = mode;
    dacBit1 = mode ? 16 : 2;
    dacBit2 = mode ? 32 : 8;
    dacBit3 = 64;
    dacShift1 = mode ? 4 : 1;
    dacShift2 = mode ? 4 : 2;
    dacShift3 = 4;
  }

};

struct BenjolinOscWidget : VenomWidget {

  struct OverSwitch : GlowingSvgSwitchLockable {
    OverSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
    }
  };

  BenjolinOscWidget(BenjolinOsc* module) {
    setModule(module);
    setVenomPanel("BenjolinOsc");
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(24.865,96.097), module, BenjolinOsc::FREQ1_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(63.288,96.097), module, BenjolinOsc::FREQ2_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(120.923,76.204), module, BenjolinOsc::OVER_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(24.865,148.33), module, BenjolinOsc::RUNG1_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(63.288,148.33), module, BenjolinOsc::RUNG2_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(120.923,148.33), module, BenjolinOsc::PATTERN_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(24.865,202.602), module, BenjolinOsc::CV1_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(63.288,202.602), module, BenjolinOsc::CV2_PARAM));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(101.712,202.602), module, BenjolinOsc::CHAOS_PARAM, BenjolinOsc::CHAOS_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(140.135,202.602), module, BenjolinOsc::DOUBLE_PARAM, BenjolinOsc::DOUBLE_LIGHT));
    addInput(createInputCentered<MonoPort>(Vec(24.865,235.665), module, BenjolinOsc::CV1_INPUT));
    addInput(createInputCentered<MonoPort>(Vec(63.288,235.665), module, BenjolinOsc::CV2_INPUT));
    addInput(createInputCentered<MonoPort>(Vec(101.712,235.665), module, BenjolinOsc::CHAOS_INPUT));
    addInput(createInputCentered<MonoPort>(Vec(140.135,235.665), module, BenjolinOsc::DOUBLE_INPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(24.865,288.931), module, BenjolinOsc::TRI1_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(63.288,288.931), module, BenjolinOsc::TRI2_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(101.712,288.931), module, BenjolinOsc::XOR_OUTPUT));
    addInput(createInputCentered<MonoPort>(Vec(140.135,288.931), module, BenjolinOsc::CLOCK_INPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(24.865,330.368), module, BenjolinOsc::PULSE1_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(63.288,330.368), module, BenjolinOsc::PULSE2_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(101.712,330.368), module, BenjolinOsc::PWM_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(140.135,330.368), module, BenjolinOsc::RUNG_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    BenjolinOsc* module = dynamic_cast<BenjolinOsc*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolMenuItem("Original release normalled values", "",
      [=]() {
        return module->origNormScale;
      },
      [=](bool val){
        module->origNormScale = val;
        module->normScale = module->origNormScale ? 1.f : 5.f;
      }
    ));
    menu->addChild(createBoolPtrMenuItem("Unipolar clock input", "", &module->unipolarClock));
    menu->addChild(createIndexSubmenuItem(
      "Rungler DAC configuration",
      {"bits 2,4,8","bits 6,7,8 (Rob Hordijks original design)"},
      [=]() {return module->dacMode;},
      [=](int i) {module->setDacMode(i);}
    ));
    menu->addChild(createMenuItem("Add Benjolin Gates Expander", "", [this](){addExpander(modelBenjolinGatesExpander,this);}));
    menu->addChild(createMenuItem("Add Benjolin Volts Expander", "", [this](){addExpander(modelBenjolinVoltsExpander,this);}));
    VenomWidget::appendContextMenu(menu);
  }

  void step() override {
    VenomWidget::step();
    BenjolinOsc* mod = dynamic_cast<BenjolinOsc*>(this->module);
    if(mod) {
      mod->lights[BenjolinOsc::CHAOS_LIGHT].setBrightness(mod->params[BenjolinOsc::CHAOS_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
      mod->lights[BenjolinOsc::DOUBLE_LIGHT].setBrightness(mod->params[BenjolinOsc::DOUBLE_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }
  }
};

Model* modelBenjolinOsc = createModel<BenjolinOsc, BenjolinOscWidget>("BenjolinOsc");

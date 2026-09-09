// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3
//
// Code derived from VCV Random algorithms

#include "Venom.hpp"

namespace Venom {

struct Random : VenomModule {
  enum ParamIds {
    OFFSET_PARAM,
    RATE_PARAM,
    PROB_PARAM,
    RAND_PARAM,
    RATE_CV_PARAM,
    PROB_CV_PARAM,
    RAND_CV_PARAM,
    STEP_PARAM,
    LIN_PARAM,
    EXP_PARAM,
    SMTH_PARAM,
    STEP_CV_PARAM,
    LIN_CV_PARAM,
    EXP_CV_PARAM,
    SMTH_CV_PARAM,
    PARAMS_LEN
  };
  enum InputIds {
    DATA_INPUT,
    TRIG_INPUT,
    RATE_CV_INPUT,
    PROB_CV_INPUT,
    RAND_CV_INPUT,
    STEP_CV_INPUT,
    LIN_CV_INPUT,
    EXP_CV_INPUT,
    SMTH_CV_INPUT,
    INPUTS_LEN
  };
  enum OutputIds {
    TRIG_OUTPUT,
    STEP_OUTPUT,
    LIN_OUTPUT,
    EXP_OUTPUT,
    SMTH_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightIds {
    OFFSET_LIGHT,
    RATE_LIGHT,
    PROB_LIGHT,
    RAND_LIGHT,
    STEP_LIGHT,
    LIN_LIGHT,
    EXP_LIGHT,
    SMTH_LIGHT,
    LIGHTS_LEN
  };

  using float_4 = simd::float_4;

  float_4 lastVoltage[4]{},
          nextVoltage[4]{},
          phase[4]{},
          clockFreq[4]{},
          clockPhase[4]{},
          clockTrig{},
          deltaPhase{};
  dsp::TTimer<float_4> clockTimer[4]{};
  dsp::TSchmittTrigger<float_4> clockTrigger[4]{};
  dsp::PulseGenerator pulseGenerator[16]{};
  int fixedChannels = 0;
  bool linearSteps = false;
  
  struct StepsParamQuantity : ParamQuantity {
    float getDisplayValue() override {
      Random* module = static_cast<Random*>(this->module);
      return std::ceil((module->linearSteps ? getValue() : std::pow(getValue(),2)) * 15.f + 1.f);
    }
    
    void setDisplayValue(float val) override {
      Random* module = static_cast<Random*>(this->module);
      val = (val-1.f)/15.f;
      if (!module->linearSteps)
        val = std::pow(val, 0.5f);
      setValue(val - 0.00001);
    }
  };

  Random() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    
    configSwitch(OFFSET_PARAM, 0.f, 1.f, 0.f, "Offset", {"Bipolar", "Unipolar"});
    configParam(RATE_PARAM, std::log2(0.002f), std::log2(2000.f), std::log2(2.f), "Internal trigger rate", " Hz", 2);
    configParam(PROB_PARAM, 0.f, 1.f, 1.f, "Trigger probability", "%", 0, 100);
    configParam(RAND_PARAM, 0.f, 1.f, 1.f, "Random spread", "%", 0, 100);
    
    configParam(RATE_CV_PARAM, -1.f, 1.f, 0.f, "Internal trigger rate CV", "%", 0, 100)->randomizeEnabled = false;
    configParam(PROB_CV_PARAM, -0.1f, 0.1f, 0.f, "Trigger probability CV", "%", 0, 1000)->randomizeEnabled = false;
    configParam(RAND_CV_PARAM, -0.1f, 0.1f, 0.f, "Random spread CV", "%", 0, 1000)->randomizeEnabled = false;
    
    configParam<StepsParamQuantity>(STEP_PARAM, 0.f, 1.f, 1.f, "Step count", "");
    configParam(LIN_PARAM, 0.f, 1.f, 1.f, "Linear shape", "%", 0, 100);
    configParam(EXP_PARAM, 0.f, 1.f, 1.f, "Exponential shape", "%", 0, 100);
    configParam(SMTH_PARAM, 0.f, 1.f, 1.f, "Smooth shape", "%", 0, 100);

    configParam(STEP_CV_PARAM, -0.1f, 0.1f, 0.f, "Step count CV", "%", 0, 1000)->randomizeEnabled = false;
    configParam(LIN_CV_PARAM, -0.1f, 0.1f, 0.f, "Linear shape CV", "%", 0, 1000)->randomizeEnabled = false;
    configParam(EXP_CV_PARAM, -0.1f, 0.1f, 0.f, "Exponential shape CV", "%", 0, 1000)->randomizeEnabled = false;
    configParam(SMTH_CV_PARAM, -0.1f, 0.1f, 0.f, "Smooth shape CV", "%", 0, 1000)->randomizeEnabled = false;

    configInput(DATA_INPUT, "Data");
    configInput(TRIG_INPUT, "Trigger");

    configInput(RATE_CV_INPUT, "Internal trigger rate");
    configInput(PROB_CV_INPUT, "Trigger probability");
    configInput(RAND_CV_INPUT, "Random spread");

    configInput(STEP_CV_INPUT, "Step count CV");
    configInput(LIN_CV_INPUT, "Linear shape CV");
    configInput(EXP_CV_INPUT, "Exponential shape CV");
    configInput(SMTH_CV_INPUT, "Smooth shape CV");

    configOutput(TRIG_OUTPUT, "Trigger");
    configOutput(STEP_OUTPUT, "Stepped");
    configOutput(LIN_OUTPUT, "Linear");
    configOutput(EXP_OUTPUT, "Exponential");
    configOutput(SMTH_OUTPUT, "Smooth");
  }
  
  float_4 random4() {
    float_4 rtn{};
    for (int i=0; i<4; i++)
      rtn[i] = random::uniform();
    return rtn;
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    int channels = fixedChannels ? fixedChannels : 1;
    if (!fixedChannels) {
      for (int i=0; i<INPUTS_LEN; i++)
        channels = std::max(channels, inputs[i].getChannels());
    }
    float trigBrightness = 3.f / channels;

    float offset = params[OFFSET_PARAM].getValue() ? -5.f : 0.f;
    for (int s=0, c=0; c<channels; s++, c+=4) {
      // clock triggers
      if (inputs[TRIG_INPUT].isConnected()) {
        clockTimer[s].process(args.sampleTime);
        clockTrig = clockTrigger[s].process(inputs[TRIG_INPUT].getPolyVoltageSimd<float_4>(c), 0.1f, 2.f);
        clockFreq[s] = ifelse(clockTrig, 1.f / clockTimer[s].time, clockFreq[s]);
        clockTimer[s].time = ifelse(clockTrig, 0.f, clockTimer[s].time);
        deltaPhase = fmin(clockFreq[s] * args.sampleTime, 0.5f);
      }
      else if (inputs[RATE_CV_INPUT].getChannels()<=1) {
        if (!s) {
          clockFreq[s] = dsp::exp2_taylor5(params[RATE_PARAM].getValue() + inputs[RATE_CV_INPUT].getVoltage() * params[RATE_CV_PARAM].getValue());
          deltaPhase = fmin(clockFreq[s] * args.sampleTime, 0.5f);
          clockPhase[s] = clockPhase[0][0] + deltaPhase[0];
          clockTrig = clockPhase[s] >= 1.f;
          clockPhase[s] = ifelse(clockTrig, clockPhase[s] - 1.f, clockPhase[s]);
        }
        else
          clockPhase[s] = clockPhase[0];
      }
      else {
        clockFreq[s] = dsp::exp2_taylor5(params[RATE_PARAM].getValue() + inputs[RATE_CV_INPUT].getPolyVoltageSimd<float_4>(c) * params[RATE_CV_PARAM].getValue());
        deltaPhase = fmin(clockFreq[s] * args.sampleTime, 0.5f);
        clockPhase[s] += deltaPhase;
        clockTrig = clockPhase[s] >= 1.f;
        clockPhase[s] = ifelse(clockTrig, clockPhase[s] - 1.f, clockPhase[s]);
      }
      float brightness = lights[RATE_LIGHT].getBrightness();
      for (int i=0; i<4; i++)
        if (clockTrig[i])
          brightness += trigBrightness;
      lights[RATE_LIGHT].setBrightness(brightness);

      // sample triggers
      float_4 prob = clamp(params[PROB_PARAM].getValue() + inputs[PROB_CV_INPUT].getPolyVoltageSimd<float_4>(c) * params[PROB_CV_PARAM].getValue(), 0.f, 1.f);
      float_4 rand = clamp(params[RAND_PARAM].getValue() + inputs[RAND_CV_INPUT].getPolyVoltageSimd<float_4>(c) * params[RAND_CV_PARAM].getValue(), 0.f, 1.f);
      float_4 sampleTrig = clockTrig & (prob >= random4());
      lastVoltage[s] = ifelse(sampleTrig, nextVoltage[s], lastVoltage[s]);
      nextVoltage[s] = ifelse(
        sampleTrig, 
        crossfade(
          lastVoltage[s],
          inputs[DATA_INPUT].isConnected() ? 
            inputs[DATA_INPUT].getPolyVoltageSimd<float_4>(c) :
            random4() * 10.f + offset,
          rand
        ),
        nextVoltage[s]
      );
      phase[s] = ifelse(sampleTrig, 0.f, phase[s]);
      brightness = lights[PROB_LIGHT].getBrightness();
      for (int i=0; i<4; i++)
        if (sampleTrig[i]) {
          brightness += trigBrightness;
          pulseGenerator[s*4+i].trigger();
        }
      lights[PROB_LIGHT].setBrightness(brightness);
  
      // Advance phase
      phase[s] = fmin(1.f, phase[s] + deltaPhase);
  
      // Stepped
      if (outputs[STEP_OUTPUT].isConnected()) {
        float_4 shape = clamp(params[STEP_PARAM].getValue() + inputs[STEP_CV_INPUT].getPolyVoltageSimd<float_4>(c) * params[STEP_CV_PARAM].getValue(), 0.f, 1.f);
        float_4 steps = ceil((linearSteps ? shape : pow(shape, 2)) * 15 + 1);
        float_4 v = ceil(phase[s] * steps) / steps;
        outputs[STEP_OUTPUT].setVoltageSimd(rescale(v, 0.f, 1.f, lastVoltage[s], nextVoltage[s]), c);
      }
  
      // Linear
      if (outputs[LIN_OUTPUT].isConnected()) {
        float_4 shape = clamp(params[LIN_PARAM].getValue() + inputs[LIN_CV_INPUT].getPolyVoltageSimd<float_4>(c) * params[LIN_CV_PARAM].getValue(), 0.f, 1.f);
        float_4 slope = 1.f / shape;
        float_4 v = ifelse(slope<1e6f, fmin(phase[s] * slope, 1.f), 1.f);
        outputs[LIN_OUTPUT].setVoltageSimd(rescale(v, 0.f, 1.f, lastVoltage[s], nextVoltage[s]), c);
      }
  
      // Exponential
      if (outputs[EXP_OUTPUT].isConnected()) {
        float_4 shape = clamp(params[EXP_PARAM].getValue() + inputs[EXP_CV_INPUT].getPolyVoltageSimd<float_4>(c) * params[EXP_CV_PARAM].getValue(), 0.f, 1.f);
        float_4 b = pow(shape, 8);
        float_4 v = ifelse(0.999f<b, phase[s], ifelse(1e-20f<b, (pow(b, phase[s])-1.f)/(b-1.f), 1.f));
        outputs[EXP_OUTPUT].setVoltageSimd(rescale(v, 0.f, 1.f, lastVoltage[s], nextVoltage[s]), c);
      }

      // Smooth
      if (outputs[SMTH_OUTPUT].isConnected()) {
        float_4 shape = clamp(params[SMTH_PARAM].getValue() + inputs[SMTH_CV_INPUT].getPolyVoltageSimd<float_4>(c) * params[SMTH_CV_PARAM].getValue(), 0.f, 1.f);
        float_4 p = 1.f / shape;
        float_4 v = ifelse(p<1e6f, fmin(phase[s]*p, 1.f), 1.f);
        v = ifelse(p<1e6f, cos(M_PI*v), 1.f);
        v = ifelse(p<1e5f, (1.f-v)/2.f, 1.f);
        outputs[SMTH_OUTPUT].setVoltageSimd(rescale(v, 0.f, 1.f, lastVoltage[s], nextVoltage[s]), c);
      }
  
      // Trigger output
      for (int i=s*4, j=0, end=std::min(s*4+4, channels); i<end; i++, j++) {
        outputs[TRIG_OUTPUT].setVoltage(pulseGenerator[i].process(args.sampleTime) ? 10.f : 0.f, i);
        if (phase[s][j] > 0.5f)
          pulseGenerator[i].reset();
      }
    }
    for (int i=0; i<OUTPUTS_LEN; i++)
      outputs[i].setChannels(channels);

    // Lights
    lights[RATE_LIGHT].setSmoothBrightness(0.f, args.sampleTime);
    lights[PROB_LIGHT].setSmoothBrightness(0.f, args.sampleTime);
    lights[RAND_LIGHT].setBrightness(1.f);
    lights[STEP_LIGHT].setBrightness(1.f);
    lights[LIN_LIGHT].setBrightness(1.f);
    lights[EXP_LIGHT].setBrightness(1.f);
    lights[SMTH_LIGHT].setBrightness(1.f);
    lights[OFFSET_LIGHT].setBrightness(offset==0.f);
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "fixedChannels", json_integer(fixedChannels));
    json_object_set_new(rootJ, "linearSteps", json_boolean(linearSteps));
    return rootJ;
  }
  
  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val = NULL;
    if ((val = json_object_get(rootJ, "fixedChannels")))
      fixedChannels = json_integer_value(val);
    if ((val = json_object_get(rootJ, "linearSteps")))
      linearSteps = json_boolean_value(val);
  }
};


struct RandomWidget : VenomWidget {
  RandomWidget(Random* module) {
    setModule(module);
    setVenomPanel("Random");

    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(22.5f,40.f), module, Random::OFFSET_PARAM, Random::OFFSET_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightSliderLockable<YellowLight>>(Vec(52.5f,69.f), module, Random::RATE_PARAM, Random::RATE_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightSliderLockable<YellowLight>>(Vec(82.5f,69.f), module, Random::PROB_PARAM, Random::PROB_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightSliderLockable<YellowLight>>(Vec(112.5f,69.f), module, Random::RAND_PARAM, Random::RAND_LIGHT));

    addInput(createInputCentered<PolyPort>(Vec(22.5f,76.5f), module, Random::DATA_INPUT));

    addInput(createInputCentered<PolyPort>(Vec(22.5f,115.5f), module, Random::TRIG_INPUT));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 125.f), module, Random::RATE_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(82.5f, 125.f), module, Random::PROB_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(112.5f, 125.f), module, Random::RAND_CV_PARAM));

    addOutput(createOutputCentered<PolyPort>(Vec(22.5f,156.f), module, Random::TRIG_OUTPUT));
    addInput(createInputCentered<PolyPort>(Vec(52.5f,156.f), module, Random::RATE_CV_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(82.5f,156.f), module, Random::PROB_CV_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(112.5f,156.f), module, Random::RAND_CV_INPUT));

    addParam(createLockableLightParamCentered<VCVLightSliderLockable<YellowLight>>(Vec(22.5f,223.f), module, Random::STEP_PARAM, Random::STEP_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightSliderLockable<YellowLight>>(Vec(52.5f,223.f), module, Random::LIN_PARAM, Random::LIN_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightSliderLockable<YellowLight>>(Vec(82.5f,223.f), module, Random::EXP_PARAM, Random::EXP_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightSliderLockable<YellowLight>>(Vec(112.5f,223.f), module, Random::SMTH_PARAM, Random::SMTH_LIGHT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(22.5f, 279.f), module, Random::STEP_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 279.f), module, Random::LIN_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(82.5f, 279.f), module, Random::EXP_CV_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(112.5f, 279.f), module, Random::SMTH_CV_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(22.5f,310.f), module, Random::STEP_CV_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(52.5f,310.f), module, Random::LIN_CV_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(82.5f,310.f), module, Random::EXP_CV_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(112.5f,310.f), module, Random::SMTH_CV_INPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(22.5f,342.5f), module, Random::STEP_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(52.5f,342.5f), module, Random::LIN_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(82.5f,342.5f), module, Random::EXP_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(112.5f,342.5f), module, Random::SMTH_OUTPUT));
  }

  void appendContextMenu(Menu* menu) override {
    Random* module = static_cast<Random*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexPtrSubmenuItem("Polyphony channels", {"Auto","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"}, &module->fixedChannels));
    menu->addChild(createBoolPtrMenuItem("Linear step count scale", "", &module->linearSteps));
    VenomWidget::appendContextMenu(menu);
  }
};

}

Model* modelVenomRandom = createModel<Venom::Random, Venom::RandomWidget>("Random");

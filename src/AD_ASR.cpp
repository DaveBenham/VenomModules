// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "math.hpp"

struct AD_ASR : VenomModule {
  enum ParamId {
    SPEED_PARAM,
    LOOP_PARAM,
    TOGGLE_PARAM,
    TRIG_PARAM,
    GATE_PARAM,
    RISE_SHAPE_PARAM,
    FALL_SHAPE_PARAM,
    RISE_TIME_PARAM,
    FALL_TIME_PARAM,
    RISE_CV_PARAM,
    FALL_CV_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RISE_CV_INPUT,
    FALL_CV_INPUT,
    TRIG_INPUT,
    GATE_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    RISE_OUTPUT,
    FALL_OUTPUT,
    SUS_OUTPUT,
    ENV_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    TRIG_LIGHT,
    GATE_LIGHT,
    LIGHTS_LEN
  };
 
  struct TimeQuantity : ParamQuantity {
    float getDisplayValue() override {
      float offset = 0.f;
      switch (static_cast<int>(module->params[SPEED_PARAM].getValue())) {
        case 0:
          offset = 4.f;
          break;
        case 2:
          offset = -3.5f;
          break;
      }
      return pow(2.f, module->params[paramId].getValue() + offset);
    }
    void setDisplayValue(float v) override {
      if (v<=0.f)
        setValue(-8.5f);
      else {
        float offset = 0.f;
        switch (static_cast<int>(module->params[SPEED_PARAM].getValue())) {
          case 0:
            offset = -4.f;
            break;
          case 2:
            offset = 3.5f;
            break;
        }
        setValue(log2(v) + offset);
      }
    }
  };

  using float_4 = simd::float_4;

  int toggleState=0,
      gateBtnVal=0,
      trigBtnState=0,
      gateBtnState=0,
      oldChannels=1;
  
  float const minTime = pow(2.f,-12.f);
  float const maxTime = pow(2.f,7.5f);
  
  float_4 trigCVState[4]{},
          gateCVState[4]{},
          phasor[4]{},
          stage[4]{},
          fullRise[4]{};

  AD_ASR() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(SPEED_PARAM, 0.f, 2.f, 1.f, "Stage speed", {"Slow (0.044 - 181 sec)", "Medium (0.0027 - 11.3 sec)", "Fast (0.00024 - 1.0 sec)"});
    configSwitch<FixedSwitchQuantity>(LOOP_PARAM, 0.f, 1.f, 0.f, "AD Loop", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(TOGGLE_PARAM, 0.f, 1.f, 0.f, "Gate button toggle mode", {"Off", "On"});

    configParam(TRIG_PARAM, 0.f, 1.f, 0.f, "Manual AD trigger");
    configParam(GATE_PARAM, 0.f, 1.f, 0.f, "Manual ASR gate");

    configParam(RISE_SHAPE_PARAM, -1.f, 1.f, 0.f, "Attack shape");
    configParam(FALL_SHAPE_PARAM, -1.f, 1.f, 0.f, "Decay/Release shape");

    configParam<TimeQuantity>(RISE_TIME_PARAM, -8.5f, 3.5f, -3.f, "Attack time", " s", 2, 1, 0);
    configParam<TimeQuantity>(FALL_TIME_PARAM, -8.5f, 3.5f, 0.f, "Decay/Release time", " s", 2, 1, 0);

    configParam(RISE_CV_PARAM, -1.f, 1.f, 0.f, "Attack time CV amount", "%", 0, 100, 0);
    configParam(FALL_CV_PARAM, -1.f, 1.f, 0.f, "Decay/Release time CV amount", "%", 0, 100, 0);
    
    configInput(RISE_CV_INPUT, "Attack time CV");
    configInput(FALL_CV_INPUT, "Decay/Release time CV");
    
    configInput(TRIG_INPUT, "AD trigger");
    configInput(GATE_INPUT, "ASR gate");
    
    configOutput(RISE_OUTPUT, "Attack gate");
    configOutput(FALL_OUTPUT, "Decay/Release gate");
    configOutput(SUS_OUTPUT, "Sustain gate");
    configOutput(ENV_OUTPUT, "Envelope");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    int undersample=1;
    // undersample if speed is slow and samplerate is high
    if (params[SPEED_PARAM].getValue()==0.f && args.sampleRate>97000.f){
      if (args.sampleRate>385000.f)
        undersample=8;
      else if (args.sampleRate>143000.f)
        undersample=4;
      else
        undersample=2;
      if (args.frame % undersample)
        return;
    }
    // get channel count
    int channels=1;
    for (int i=0; i<INPUTS_LEN; i++){
      if(inputs[i].getChannels() > channels)
        channels = inputs[i].getChannels();
    }
    // clear dropped channels
    for (int c=channels+1; c<oldChannels; c++) { // clear dropped channels
      int s=c/4;
      int i=c%4;
      trigCVState[s][i] = 0.f;
      gateCVState[s][i] = 0.f;
    }
    oldChannels = channels;
    // reset gate button if toggle switch change and button not locked
    if (params[TOGGLE_PARAM].getValue() != toggleState) {
      if (!paramExtensions[GATE_PARAM].locked) // only change gateBtnState if gate button not locked
        gateBtnState = 0.f;
      toggleState = !toggleState;
    }
    // compute button states and trigs
    int trigBtnNewState = params[TRIG_PARAM].getValue();
    int trigBtnTrig = trigBtnNewState > trigBtnState;
    int gateBtnNewState = gateBtnState;
    if (params[GATE_PARAM].getValue() != gateBtnVal){ // gate button value change
      gateBtnVal = !gateBtnVal;
      if (params[TOGGLE_PARAM].getValue()) { // gate button toggle mode on
         if (gateBtnVal)
           gateBtnNewState = !gateBtnNewState;
      } else { // gate button toggle mode off
        gateBtnNewState = gateBtnVal;
      }
    }
    int gateBtnTrig = gateBtnNewState > gateBtnState;
    
    // compute some constants that work for all channels
    float offset = 0.f;
    switch (static_cast<int>(params[SPEED_PARAM].getValue())) {
      case 0:
        offset = 4.f;
        break;
      case 2:
        offset = -3.5f;
        break;
    }
    float riseParm = params[RISE_TIME_PARAM].getValue() + offset;
    float fallParm = params[FALL_TIME_PARAM].getValue() + offset;
    float riseShape = -params[RISE_SHAPE_PARAM].getValue()*0.9f;
    float fallShape = -params[FALL_SHAPE_PARAM].getValue()*0.9f;
    float riseCVAmt = params[RISE_CV_PARAM].getValue();
    float fallCVAmt = params[FALL_CV_PARAM].getValue();
    float loop = params[LOOP_PARAM].getValue();
    // iterate the channels
    for (int s=0, c=0; c<channels; s++, c+=4){
      // compute trig CV state and trig
      float_4 trigCVVal = inputs[TRIG_INPUT].getPolyVoltageSimd<float_4>(c);
      float_4 trigCVNewState = ifelse(trigCVVal>2.f, 1.f, trigCVState[s]);
      trigCVNewState = ifelse(trigCVVal<0.2f, 0.f, trigCVNewState);
      float_4 trigCVTrig = ifelse(trigCVNewState > trigCVState[s], 1.f, 0.f);
      // compute gate CV state and trig
      float_4 gateCVVal = inputs[GATE_INPUT].getPolyVoltageSimd<float_4>(c);
      float_4 gateCVNewState = ifelse(gateCVVal>2.f, 1.f, gateCVState[s]);
      gateCVNewState = ifelse(gateCVVal<0.2f, 0.f, gateCVNewState);
      float_4 gateCVTrig = ifelse(gateCVNewState > gateCVState[s], 1.f, 0.f);
      // compute current stage deltas
      float_4 riseDelta = args.sampleTime * undersample / clamp(pow(2.f, inputs[RISE_CV_INPUT].getPolyVoltageSimd<float_4>(c)*riseCVAmt + riseParm), minTime, maxTime);
      float_4 fallDelta = -args.sampleTime * undersample / clamp(pow(2.f, inputs[FALL_CV_INPUT].getPolyVoltageSimd<float_4>(c)*fallCVAmt + fallParm), minTime, maxTime);
      float_4 delta{};
      delta = ifelse(stage[s]==1.f, riseDelta, delta);
      delta = ifelse(stage[s]==3.f, fallDelta, delta);
      // update phasor
      phasor[s] = clamp(phasor[s]+delta);
      // compute and apply stage shapes
      float_4 shape{};
      shape = ifelse(stage[s]==1.f, riseShape, shape);
      shape = ifelse(stage[s]==3.f, fallShape, shape);
      float_4 curve = normSigmoid(phasor[s], shape);
      // set ENV output
      outputs[ENV_OUTPUT].setVoltageSimd(curve*10.f, c);
      // progress through stages
      float_4 newStage = stage[s];
      float_4 hold = gateCVNewState + gateBtnNewState;
      newStage = ifelse((stage[s]==1.f) & ((hold+fullRise[s]+loop)==0.f), 3.f, newStage);
      newStage = ifelse(phasor[s]!=1.f, newStage, ifelse(hold>0.f, 2.f, 3.f));
      newStage = ifelse(phasor[s]==0.f, 0.f, newStage);
      float_4 blockTrig = trigCVState[s] + trigBtnState;
      float_4 loopTrig = ifelse((blockTrig==0.f) & (stage[s]==3.f) & (phasor[s]==0.f), loop, 0.f);
      blockTrig += gateCVState[s] + gateBtnState;
      blockTrig += ifelse(stage[s]!=0.f, loop, 0.f);
      float_4 fullTrig = ifelse((blockTrig==0.f) & (stage[s]!=1.f), trigCVTrig + trigBtnTrig, 0.f);
      float_4 anyTrig = ifelse((blockTrig==0.f) & (stage[s]!=1.f), fullTrig + gateCVTrig + gateBtnTrig, 0.f);
      anyTrig += loopTrig;
      newStage = ifelse(anyTrig>0.f, 1.f, newStage);
      // adjust phasor to incoming curve value if entering stage 1 or 3
      phasor[s] = ifelse((newStage==1.f) & (stage[s]!=1.f), invNormSigmoid(curve, riseShape), phasor[s]);
      phasor[s] = ifelse((newStage==3.f) & (stage[s]!=3.f), invNormSigmoid(curve, fallShape), phasor[s]);
      //set final stage outcome
      stage[s] = newStage;
      fullRise[s] = ifelse(anyTrig>0.f, ifelse(fullTrig>0.f, 1.f, 0.f), fullRise[s]);
      // set output gates
      outputs[RISE_OUTPUT].setVoltageSimd(ifelse(stage[s]==1.f, 10.f, 0.f), c);
      outputs[SUS_OUTPUT].setVoltageSimd(ifelse(stage[s]==2.f, 10.f, 0.f), c);
      outputs[FALL_OUTPUT].setVoltageSimd(ifelse(stage[s]==3.f, 10.f, 0.f), c);
      // update global states
      trigCVState[s] = trigCVNewState;
      gateCVState[s] = gateCVNewState;
    } // end channel loop
    trigBtnState = trigBtnNewState;
    gateBtnState = gateBtnNewState;
    // update output channel counts
    for (int i=0; i<OUTPUTS_LEN; i++) {
      outputs[i].setChannels(channels);
    }
  }

};

struct AD_ASRWidget : VenomWidget {

  struct SpeedSwitch : GlowingSvgSwitchLockable {
    SpeedSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };

  struct OnOffSwitch : GlowingSvgSwitchLockable {
    OnOffSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
    }
  };

  AD_ASRWidget(AD_ASR* module) {
    setModule(module);
    setVenomPanel("AD_ASR");

    addParam(createLockableParam<SpeedSwitch>(Vec(13.22f, 44.297f), module, AD_ASR::SPEED_PARAM));
    addParam(createLockableParam<OnOffSwitch>(Vec(33.72f, 44.297f), module, AD_ASR::LOOP_PARAM));
    addParam(createLockableParam<OnOffSwitch>(Vec(54.22f, 44.297f), module, AD_ASR::TOGGLE_PARAM));

    addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteLight>>>(Vec(21.f, 82.5), module, AD_ASR::TRIG_PARAM, AD_ASR::TRIG_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteLight>>>(Vec(54.f, 82.5), module, AD_ASR::GATE_PARAM, AD_ASR::GATE_LIGHT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(21.f, 112.5f), module, AD_ASR::RISE_SHAPE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(54.f, 112.5f), module, AD_ASR::FALL_SHAPE_PARAM));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(21.f, 151.f), module, AD_ASR::RISE_TIME_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(54.f, 151.f), module, AD_ASR::FALL_TIME_PARAM));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(21.f, 187.f), module, AD_ASR::RISE_CV_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(54.f, 187.f), module, AD_ASR::FALL_CV_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(21.f, 220.5), module, AD_ASR::RISE_CV_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(54.f, 220.5), module, AD_ASR::FALL_CV_INPUT));

    addInput(createInputCentered<PolyPort>(Vec(21.f, 266.5), module, AD_ASR::TRIG_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(54.f, 266.5), module, AD_ASR::GATE_INPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(21.f, 304.5), module, AD_ASR::RISE_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(54.f, 304.5), module, AD_ASR::FALL_OUTPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(21.f, 341.5), module, AD_ASR::SUS_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(54.f, 341.5), module, AD_ASR::ENV_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    AD_ASR* mod = dynamic_cast<AD_ASR*>(this->module);
    if(mod) {
      mod->lights[AD_ASR::TRIG_LIGHT].setBrightness(mod->trigBtnState);
      mod->lights[AD_ASR::GATE_LIGHT].setBrightness(mod->gateBtnState);
    }
  }

};

Model* modelAD_ASR = createModel<AD_ASR, AD_ASRWidget>("AD_ASR");

// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "math.hpp"

namespace Venom {

struct AD_ASR : VenomModule { 
  enum ParamId {
    SPEED_PARAM,
    MODE_PARAM,
    TOGGLE_PARAM,
    TRIG_PARAM,
    GATE_PARAM,
    RISE_SHAPE_PARAM,
    FALL_SHAPE_PARAM,
    RISE_TIME_PARAM,
    FALL_TIME_PARAM,
    RISE_CV_PARAM,
    FALL_CV_PARAM,
    RISE_OUT_PARAM,
    FALL_OUT_PARAM,
    SUS_OUT_PARAM,
    ENV_OUT_PARAM,
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
        case 3:
          offset = 8.f;
      }
      return pow(2.f, module->params[paramId].getValue() + offset);
    }
    void setDisplayValue(float v) override {
      int speedParam = static_cast<int>(module->params[SPEED_PARAM].getValue());
      if (v<=0.f)
        setValue(speedParam==3 ? -4.5 : -8.5f);
      else {
        float offset = 0.f;
        switch (speedParam) {
          case 0:
            offset = -4.f;
            break;
          case 2:
            offset = 3.5f;
            break;
          case 3:
            offset = -8.f;
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
      oldChannels=1,
      riseOutMode=0,
      susOutMode=0,
      fallOutMode=0;

  float mode = 0.f,
        blockRetrigStage = 1.f;

  const float normMinTime = pow(2.f, -12.f),
              normMaxTime = pow(2.f, 7.5f),
              slowMinTime = pow(2.f, -5.f),
              slowMaxTime = pow(2.f, 11.5f);

  float_4 loop{};

  float_4 trigCVState[4]{},
          gateCVState[4]{},
          phasor[4]{},
          stage[4]{},
          fullRise[4]{},
          riseTrig[4]{},
          fallTrig[4]{},
          susTrig[4]{};

  AD_ASR() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(SPEED_PARAM, 0.f, 3.f, 1.f, "Stage speed", {"Slow (0.044 - 181 sec)", "Medium (0.0027 - 11.3 sec)", "Fast (0.00024 - 1.0 sec)", "Glacial (0.707 sec - 48.27 min)"});
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 3.f, 0.f, "Mode", {
      "1) TRIG=AD, GATE=ASR | Retrig current level",
      "2) TRIG=AD, GATE=ASR | Retrig 0V",
      "3) Loop: TRIG=Start, held TRIG=Stop, GATE=Hold 10V",
      "4) GATE low: TRIG=AD, Retrig 0V | GATE high: Loop, TRIG=Hard sync"
    });
    configSwitch<FixedSwitchQuantity>(TOGGLE_PARAM, 0.f, 1.f, 0.f, "Gate button toggle mode", {"Off", "On"});

    configParam(TRIG_PARAM, 0.f, 1.f, 0.f, "Manual trigger");
    configParam(GATE_PARAM, 0.f, 1.f, 0.f, "Manual gate");

    configParam(RISE_SHAPE_PARAM, -1.f, 1.f, 0.f, "Rise shape");
    configParam(FALL_SHAPE_PARAM, -1.f, 1.f, 0.f, "Fall shape");

    configParam<TimeQuantity>(RISE_TIME_PARAM, -8.5f, 3.5f, -3.f, "Rise time", " s", 2, 1, 0);
    configParam<TimeQuantity>(FALL_TIME_PARAM, -8.5f, 3.5f, 0.f, "Fall time", " s", 2, 1, 0);

    configParam(RISE_CV_PARAM, -1.f, 1.f, 0.f, "Rise time CV amount", "%", 0, 100, 0);
    configParam(FALL_CV_PARAM, -1.f, 1.f, 0.f, "Fall time CV amount", "%", 0, 100, 0);
    
    configInput(RISE_CV_INPUT, "Rise time CV");
    configInput(FALL_CV_INPUT, "Fall time CV");
    
    configInput(TRIG_INPUT, "Trigger");
    configInput(GATE_INPUT, "Gate");

    configSwitch<FixedSwitchQuantity>(RISE_OUT_PARAM, 0.f, 2.f, 0.f, "Rise output mode", {"Gate", "Start trigger", "End trigger"});
    configSwitch<FixedSwitchQuantity>(FALL_OUT_PARAM, 0.f, 3.f, 0.f, "Fall output mode", {"Gate", "Start trigger", "End trigger", "EOC trigger"});
    configSwitch<FixedSwitchQuantity>(SUS_OUT_PARAM, 0.f, 2.f, 0.f, "Sus output mode", {"Gate", "Start trigger", "End trigger"});
    configSwitch<FixedSwitchQuantity>(ENV_OUT_PARAM, 0.f, 2.f, 0.f, "Env output mode", {"Unipolar 0 to 10", "Inverted unipolar 10 to 0", "Bipolar -5 to 5"});
    
    configOutput(RISE_OUTPUT, "Rise gate");
    configOutput(FALL_OUTPUT, "Fall gate");
    configOutput(SUS_OUTPUT, "Sustain gate");
    configOutput(ENV_OUTPUT, "Envelope");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    int speedParam = static_cast<int>(params[SPEED_PARAM].getValue());
    int undersample=1;
    // under-sample (skip frames) if speed is slow and sample rate is high
    if ((speedParam==0 || speedParam==3) && args.sampleRate>97000.f){
      if (args.sampleRate>385000.f)
        undersample=8;
      else if (args.sampleRate>143000.f)
        undersample=4;
      else // sampleRate>97000.f
        undersample=2;
    }
    if (speedParam==3)
      undersample*=16;
    if (args.frame % undersample)
      return;
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
    // update stage output modes
    if (riseOutMode != params[RISE_OUT_PARAM].getValue()){
      riseOutMode = params[RISE_OUT_PARAM].getValue();
      PortInfo* portInfo = outputInfos[RISE_OUTPUT];
      PortExtension* portExt = &outputExtensions[RISE_OUTPUT];
      bool chng = portInfo->name == portExt->factoryName;
      switch(riseOutMode){
        case 0:
          portExt->factoryName="Rise gate";
          break;
        case 1:
          portExt->factoryName="Rise start trigger";
          break;
        case 2:
          portExt->factoryName="Rise end trigger";
          break;
      }
      if (chng)
        portInfo->name = portExt->factoryName;
    }
    if (susOutMode != params[SUS_OUT_PARAM].getValue()){
      susOutMode = params[SUS_OUT_PARAM].getValue();
      PortInfo* portInfo = outputInfos[RISE_OUTPUT];
      PortExtension* portExt = &outputExtensions[RISE_OUTPUT];
      bool chng = portInfo->name == portExt->factoryName;
      switch(susOutMode){
        case 0:
          portExt->factoryName="Sustain gate";
          break;
        case 1:
          portExt->factoryName="Sustain start trigger";
          break;
        case 2:
          portExt->factoryName="Sustain end trigger";
          break;
      }
      if (chng)
        portExt->factoryName = portInfo->name;
    }
    if (fallOutMode != params[FALL_OUT_PARAM].getValue()){
      fallOutMode = params[FALL_OUT_PARAM].getValue();
      PortInfo* portInfo = outputInfos[RISE_OUTPUT];
      PortExtension* portExt = &outputExtensions[RISE_OUTPUT];
      bool chng = portInfo->name == portExt->factoryName;
      switch(fallOutMode){
        case 0:
          portExt->factoryName="Fall gate";
          break;
        case 1:
          portExt->factoryName="Fall start trigger";
          break;
        case 2:
          portExt->factoryName="Fall end trigger";
          break;
        case 3:
          portExt->factoryName="EOC trigger";
          break;
      }
      if (chng)
        portExt->factoryName = portInfo->name;
    }
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
    // partially manage loop mode
    if (mode != params[MODE_PARAM].getValue()) {
      mode = params[MODE_PARAM].getValue();
      loop = mode == 2.f ? 1.f : 0.f;
      blockRetrigStage = mode == 0.f || mode == 2.f ? 1.f : -1.f /*none*/;
    }
    // compute some process constants that work for all channels
    float offset = 0.f;
    switch (static_cast<int>(params[SPEED_PARAM].getValue())) {
      case 0:
        offset = 4.f;
        break;
      case 2:
        offset = -3.5f;
        break;
      case 3:
        offset = 8.f;
        break;
    }
    float riseParm = params[RISE_TIME_PARAM].getValue() + offset;
    float fallParm = params[FALL_TIME_PARAM].getValue() + offset;
    float riseShape = -params[RISE_SHAPE_PARAM].getValue()*0.95f;
    float fallShape = -params[FALL_SHAPE_PARAM].getValue()*0.95f;
    float riseCVAmt = params[RISE_CV_PARAM].getValue();
    float fallCVAmt = params[FALL_CV_PARAM].getValue();
    float minTime = speedParam==3 ? slowMinTime : normMinTime;
    float maxTime = speedParam==3 ? slowMaxTime : normMaxTime;
    int envMode = static_cast<int>(params[ENV_OUT_PARAM].getValue());
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
      float_4 eocMask = (stage[s]==3.f) & (phasor[s]==0.f);
      // compute and apply stage shapes
      float_4 shape{};
      shape = ifelse(stage[s]==1.f, riseShape, shape);
      shape = ifelse(stage[s]==3.f, fallShape, shape);
      float_4 curve = normSigmoid(phasor[s], shape);
      // set ENV output
      switch (envMode) {
        case 0:  // unipolar
          outputs[ENV_OUTPUT].setVoltageSimd(curve*10.f, c);
          break;
        case 1:  // inverted unipolar
          outputs[ENV_OUTPUT].setVoltageSimd(curve*-10.f + 10.f, c);
          break;
        default: // 2 bipolar
          outputs[ENV_OUTPUT].setVoltageSimd(curve*10.f - 5.f, c);
      }
      // progress through stages
      loop = mode==3.f ? gateCVNewState + gateBtnNewState : loop;
      float_4 newStage = stage[s];
      float_4 hold = mode != 3.f ? gateCVNewState + gateBtnNewState : 0.f;
      newStage = ifelse((stage[s]==1.f) & ((hold+fullRise[s]+loop)==0.f), 3.f, newStage);
      newStage = ifelse(phasor[s]!=1.f, newStage, ifelse(hold>0.f, 2.f, 3.f));
      newStage = ifelse(phasor[s]==0.f, 0.f, newStage);
      float_4 blockTrig = mode != 3.f ? trigCVState[s] + trigBtnState : 0.f;
      float_4 loopTrig = ifelse((blockTrig==0.f) & (stage[s]==3.f) & (phasor[s]==0.f), loop, 0.f);
      if (mode != 3.f) {
        blockTrig += gateCVState[s] + gateBtnState;
        blockTrig += ifelse(stage[s]!=0.f, loop, 0.f);
      }  
      float_4 fullTrig = ifelse((blockTrig==0.f) & (stage[s]!=blockRetrigStage), trigCVTrig + trigBtnTrig, 0.f);
      float_4 anyTrig = ifelse((blockTrig==0.f) & (stage[s]!=blockRetrigStage), fullTrig + gateCVTrig + gateBtnTrig, 0.f);
      anyTrig += loopTrig;
      newStage = ifelse(anyTrig>0.f, 1.f, newStage);
      // adjust phasor to incoming curve value or 0.f if entering stage 1
      phasor[s] = ifelse((newStage==1.f) & (anyTrig>0.f), mode==1.f || mode==3.f ? 0.f : invNormSigmoid(curve, riseShape), phasor[s]);
      // adjust phasor to incoming curve value if entering stage 3
      phasor[s] = ifelse((newStage==3.f) & (stage[s]!=3.f), invNormSigmoid(curve, fallShape), phasor[s]);
      // set output gates/triggers
      switch (static_cast<int>(params[RISE_OUT_PARAM].getValue())){
        case 1: // Rise start trigger
          riseTrig[s] = ifelse((newStage==1.f) & (stage[s]!=1.f), 0.001f, riseTrig[s]);
          riseTrig[s] = ifelse(newStage!=1.f, 0.f, riseTrig[s]);
          outputs[RISE_OUTPUT].setVoltageSimd(ifelse(riseTrig[s]>0.f, 10.f, 0.f), c);
          riseTrig[s] -= args.sampleTime;
          break;
        case 2: // Rise end trigger
          riseTrig[s] = ifelse((stage[s]==1.f) & (newStage!=1.f), 0.001f, riseTrig[s]);
          riseTrig[s] = ifelse(newStage==1.f, 0.f, riseTrig[s]);
          outputs[RISE_OUTPUT].setVoltageSimd(ifelse(riseTrig[s]>0.f, 10.f, 0.f), c);
          riseTrig[s] -= args.sampleTime;
          break;
        default: // Rise gate
          outputs[RISE_OUTPUT].setVoltageSimd(ifelse(newStage==1.f, 10.f, 0.f), c);
          riseTrig[s] = 0.f;
      }
      switch (static_cast<int>(params[SUS_OUT_PARAM].getValue())){
        case 1: // Sus start trigger
          susTrig[s] = ifelse((newStage==2.f) & (stage[s]!=2.f), 0.001f, susTrig[s]);
          susTrig[s] = ifelse(newStage!=2.f, 0.f, susTrig[s]);
          outputs[SUS_OUTPUT].setVoltageSimd(ifelse(susTrig[s]>0.f, 10.f, 0.f), c);
          susTrig[s] -= args.sampleTime;
          break;
        case 2: // Sus end trigger
          susTrig[s] = ifelse((stage[s]==2.f) & (newStage!=2.f), 0.001f, susTrig[s]);
          susTrig[s] = ifelse(newStage==2.f, 0.f, susTrig[s]);
          outputs[SUS_OUTPUT].setVoltageSimd(ifelse(susTrig[s]>0.f, 10.f, 0.f), c);
          susTrig[s] -= args.sampleTime;
          break;
        default: // Sus gate
          outputs[SUS_OUTPUT].setVoltageSimd(ifelse(newStage==2.f, 10.f, 0.f), c);
          susTrig[s] = 0.f;
      }
      switch (static_cast<int>(params[FALL_OUT_PARAM].getValue())){
        case 1: // Fall start trigger
          fallTrig[s] = ifelse((newStage==3.f) & (stage[s]!=3.f), 0.001f, fallTrig[s]);
          fallTrig[s] = ifelse(newStage!=3.f, 0.f, fallTrig[s]);
          outputs[FALL_OUTPUT].setVoltageSimd(ifelse(fallTrig[s]>0.f, 10.f, 0.f), c);
          fallTrig[s] -= args.sampleTime;
          break;
        case 2: // Fall end trigger
          fallTrig[s] = ifelse((stage[s]==3.f) & (newStage!=3.f), 0.001f, fallTrig[s]);
          fallTrig[s] = ifelse(newStage==3.f, 0.f, fallTrig[s]);
          outputs[FALL_OUTPUT].setVoltageSimd(ifelse(fallTrig[s]>0.f, 10.f, 0.f), c);
          fallTrig[s] -= args.sampleTime;
          break;
        case 3: // EOC trigger
          fallTrig[s] = ifelse(eocMask, 0.001f, fallTrig[s]);
          fallTrig[s] = ifelse(newStage==3.f, 0.f, fallTrig[s]);
          outputs[FALL_OUTPUT].setVoltageSimd(ifelse(fallTrig[s]>0.f, 10.f, 0.f), c);
          fallTrig[s] -= args.sampleTime;
          break;
        default: // Fall gate
          outputs[FALL_OUTPUT].setVoltageSimd(ifelse(newStage==3.f, 10.f, 0.f), c);
          fallTrig[s] = 0.f;
      }
      // update global states
      stage[s] = newStage;
      fullRise[s] = ifelse(anyTrig>0.f, ifelse(fullTrig>0.f, 1.f, 0.f), fullRise[s]);
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
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
    }
  };
  
  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
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

  struct OutSwitch : GlowingSvgSwitchLockable {
    OutSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
    }
  };

  struct EnvSwitch : GlowingSvgSwitchLockable {
    EnvSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
    }
  };

  AD_ASRWidget(AD_ASR* module) {
    setModule(module);
    setVenomPanel("AD_ASR");

    addParam(createLockableParam<SpeedSwitch>(Vec(12.6f, 44.297f), module, AD_ASR::SPEED_PARAM));
    addParam(createLockableParam<ModeSwitch>(Vec(33.1f, 44.297f), module, AD_ASR::MODE_PARAM));
    addParam(createLockableParam<OnOffSwitch>(Vec(53.6f, 44.297f), module, AD_ASR::TOGGLE_PARAM));

    addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteLight>>>(Vec(21.f, 78.5), module, AD_ASR::TRIG_PARAM, AD_ASR::TRIG_LIGHT));
    addParam(createLockableLightParamCentered<VCVLightBezelLockable<MediumSimpleLight<WhiteLight>>>(Vec(54.f, 78.5), module, AD_ASR::GATE_PARAM, AD_ASR::GATE_LIGHT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(21.f, 112.5f), module, AD_ASR::RISE_SHAPE_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(54.f, 112.5f), module, AD_ASR::FALL_SHAPE_PARAM));

    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(21.f, 153.f), module, AD_ASR::RISE_TIME_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(54.f, 153.f), module, AD_ASR::FALL_TIME_PARAM));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(21.f, 189.f), module, AD_ASR::RISE_CV_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(54.f, 189.f), module, AD_ASR::FALL_CV_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(21.f, 222.5), module, AD_ASR::RISE_CV_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(54.f, 222.5), module, AD_ASR::FALL_CV_INPUT));

    addInput(createInputCentered<PolyPort>(Vec(21.f, 264.5), module, AD_ASR::TRIG_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(54.f, 264.5), module, AD_ASR::GATE_INPUT));

    addParam(createLockableParam<OutSwitch>(Vec(4.f, 283.249f), module, AD_ASR::RISE_OUT_PARAM));
    addParam(createLockableParam<OutSwitch>(Vec(62.2f, 283.249f), module, AD_ASR::FALL_OUT_PARAM));
    addOutput(createOutputCentered<PolyPort>(Vec(21.f, 304.5), module, AD_ASR::RISE_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(54.f, 304.5), module, AD_ASR::FALL_OUTPUT));

    addParam(createLockableParam<OutSwitch>(Vec(4.f, 320.206f), module, AD_ASR::SUS_OUT_PARAM));
    addParam(createLockableParam<EnvSwitch>(Vec(62.2f, 320.206f), module, AD_ASR::ENV_OUT_PARAM));
    addOutput(createOutputCentered<PolyPort>(Vec(21.f, 341.5), module, AD_ASR::SUS_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(54.f, 341.5), module, AD_ASR::ENV_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    AD_ASR* mod = dynamic_cast<AD_ASR*>(this->module);
    if(mod) {
      mod->lights[AD_ASR::TRIG_LIGHT].setBrightness(mod->trigBtnState>0 ? 1.f : 0.02f);
      mod->lights[AD_ASR::GATE_LIGHT].setBrightness(mod->gateBtnState>0 ? 1.f : 0.02f);
    }
  }

};

}

Model* modelVenomAD_ASR = createModel<Venom::AD_ASR, Venom::AD_ASRWidget>("AD_ASR");

// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "MixModule.hpp"
#include "math.hpp"
#include <cfloat>
#include "Filter.hpp"

struct VCAMix4Stereo : MixBaseModule {
  enum ParamId {
    ENUMS(LEVEL_PARAMS, 4),
    MIX_LEVEL_PARAM,
    MODE_PARAM,
    CLIP_PARAM,
    DCBLOCK_PARAM,
    VCAMODE_PARAM,
    EXCLUDE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(LEFT_INPUTS, 4),
    ENUMS(RIGHT_INPUTS, 4),
    LEFT_CHAIN_INPUT,
    RIGHT_CHAIN_INPUT,
    ENUMS(CV_INPUTS, 4),
    MIX_CV_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(LEFT_OUTPUTS, 4),
    ENUMS(RIGHT_OUTPUTS, 4),
    LEFT_MIX_OUTPUT,
    RIGHT_MIX_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  int mode = -1;
  bool connected[4] = {false, false, false, false};
  float normal = 0.f;
  float scale = 1.f;
  float offset = 0.f;
  int oversample = 4, sampleRate = 0;
  OversampleFilter_4 leftUpSample[4]{}, leftDownSample[4]{}, 
                     rightUpSample[4]{}, rightDownSample[4]{},
                     cvVcaBandlimit[5][4]{},
                     inLeftVcaBandlimit[5][4]{}, inRightVcaBandlimit[5][4]{},
                     outLeftVcaBandlimit[5][4]{}, outRightVcaBandlimit[5][4]{};
  DCBlockFilter_4 leftDcBlockBeforeFilter[4]{}, leftDcBlockAfterFilter[4]{}, 
                  rightDcBlockBeforeFilter[4]{}, rightDcBlockAfterFilter[4]{};

  VCAMix4Stereo() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    mixType = VCAMIX4ST_TYPE;
    baseMod = true;
    stereo = true;
    for (int i=0; i < 4; i++){
      configInput(CV_INPUTS+i, string::f("Channel %d CV", i + 1));
      configParam(LEVEL_PARAMS+i, 0.f, 2.f, 1.f, string::f("Channel %d level", i + 1), " dB", -10.f, 20.f);
      configInput(LEFT_INPUTS+i, string::f("Left channel %d", i + 1));
      configInput(RIGHT_INPUTS+i, string::f("Right channel %d", i + 1))->description = string::f("Normalled to left channel %d input", i+1);
      configOutput(LEFT_OUTPUTS+i, string::f("Left channel %d", i + 1));
      configOutput(RIGHT_OUTPUTS+i, string::f("Right channel %d", i + 1));
    }
    configInput(MIX_CV_INPUT, "Mix CV");
    configParam(MIX_LEVEL_PARAM, 0.f, 2.f, 1.f, "Mix level", " dB", -10.f, 20.f);
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 4.f, 0.f, "Level Mode", {
      "Unipolar dB (audio x2)", "Unipolar poly sum dB (audio x2)", "Bipolar % (CV)", "Bipolar x2 (CV)", "Bipolar x10 (CV)"
    });
    configSwitch<FixedSwitchQuantity>(VCAMODE_PARAM, 0.f, 5.f, 0.f, "VCA Mode", {
      "Unipolar linear - CV clamped 0-10V", "Unipolar exponential - CV clamped 0-10V",
      "Bipolar linear - CV unclamped", "Bipolar exponential - CV unclamped",
      "Bipolar linear band limited - CV unclamped", "Bipolar exponential band limited - CV unclamped"
    });
    configSwitch<FixedSwitchQuantity>(DCBLOCK_PARAM, 0.f, 3.f, 0.f, "Mix DC Block", {"Off", "Before clipping", "Before and after clipping", "After clipping"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 7.f, 0.f, "Mix Clipping", {"Off", "Hard post-level at 10V", "Soft post-level at 10V", "Soft oversampled post-levl at 10V", 
                                                                                         "Hard pre-level at 10V", "Soft pre-level at 10V", "Soft oversampled pre-level at 10V",
                                                                                         "Saturate (Soft oversampled post-level at 6V)"});
    configSwitch<FixedSwitchQuantity>(EXCLUDE_PARAM, 0.f, 1.f, 0.f, "Exclude Patched Outs from Mix", {"Off", "On"});
    configInput(LEFT_CHAIN_INPUT, "Left chain");
    configInput(RIGHT_CHAIN_INPUT, "Right chain")->description = "Normalled to left chain input";
    configOutput(LEFT_MIX_OUTPUT, "Left mix");
    configOutput(RIGHT_MIX_OUTPUT, "Right mix");
    for (int i=0; i<4; i++){
      configBypass(LEFT_INPUTS+i, LEFT_OUTPUTS+i);
    }
    for (int i=0; i<4; i++){
      configBypass(inputs[RIGHT_INPUTS+i].isConnected() ? RIGHT_INPUTS+i : LEFT_INPUTS+i, RIGHT_OUTPUTS+i);
    }
    oversampleStages = 5;
    setOversample();
  }

  void setOversample() override {
    for (int i=0; i<4; i++){
      leftUpSample[i].setOversample(oversample, oversampleStages);
      leftDownSample[i].setOversample(oversample, oversampleStages);
      rightUpSample[i].setOversample(oversample, oversampleStages);
      rightDownSample[i].setOversample(oversample, oversampleStages);
      for (int j=0; j<5; j++){
        cvVcaBandlimit[j][i].setOversample(oversample, oversampleStages);
        inLeftVcaBandlimit[j][i].setOversample(oversample, oversampleStages);
        outLeftVcaBandlimit[j][i].setOversample(oversample, oversampleStages);
        inRightVcaBandlimit[j][i].setOversample(oversample, oversampleStages);
        outRightVcaBandlimit[j][i].setOversample(oversample, oversampleStages);
      }
    }
  }

  void onReset(const ResetEvent& e) override {
    mode = -1;
    setOversample();
    Module::onReset(e);
  }

  void onPortChange(const PortChangeEvent& e) override {
    if (e.type == Port::INPUT && e.portId >= RIGHT_INPUTS && e.portId < RIGHT_INPUTS+4)
      bypassRoutes[e.portId].inputId = e.connecting ? e.portId : e.portId - 4;
  }

  void process(const ProcessArgs& args) override {
    MixBaseModule::process(args);
    if (args.sampleRate != sampleRate){
      sampleRate = args.sampleRate;
      for (int i=0; i<4; i++){
        leftDcBlockBeforeFilter[i].init(oversample, sampleRate);
        leftDcBlockAfterFilter[i].init(oversample, sampleRate);
        rightDcBlockBeforeFilter[i].init(oversample, sampleRate);
        rightDcBlockAfterFilter[i].init(oversample, sampleRate);
      }
    }
    if( static_cast<int>(params[MODE_PARAM].getValue()) != mode ||
      connected[0] != (inputs[LEFT_INPUTS + 0].isConnected() || inputs[RIGHT_INPUTS + 0].isConnected()) ||
      connected[1] != (inputs[LEFT_INPUTS + 1].isConnected() || inputs[RIGHT_INPUTS + 1].isConnected()) ||
      connected[2] != (inputs[LEFT_INPUTS + 2].isConnected() || inputs[RIGHT_INPUTS + 2].isConnected()) ||
      connected[3] != (inputs[LEFT_INPUTS + 3].isConnected() || inputs[RIGHT_INPUTS + 3].isConnected())
    ){
      mode = static_cast<int>(params[MODE_PARAM].getValue());
      ParamQuantity* q;
      for (int i=0; i<4; i++) {
        connected[i] = inputs[LEFT_INPUTS + i].isConnected() || inputs[RIGHT_INPUTS + i].isConnected();
        q = paramQuantities[LEVEL_PARAMS + i];
        q->unit = mode <= 1 ? " dB" : !connected[i] ? " V" : mode == 2 ? "%" : "x";
        q->displayBase = mode <= 1 ? -10.f : 0.f;
        q->displayMultiplier = mode <= 1 ? 20.f : (mode == 2 && connected[i]) ? 100.f : (mode == 3 && connected[i]) ? 2.f : 10.f;
        q->displayOffset = mode <= 1 ? 0.f : (mode == 2 && connected[i]) ? -100.f : (mode == 3 && connected[i]) ? -2.f : -10.f;
      }
      q = paramQuantities[MIX_LEVEL_PARAM];
      q->unit = mode <= 1 ? " dB" : mode == 2 ? "%" : "x";
      q->displayBase = mode <= 1 ? -10.f : 0.f;
      q->displayMultiplier = mode <= 1 ? 20.f : mode == 2 ? 100.f : mode == 3 ? 2.f : 10.f;
      q->displayOffset = mode <= 1 ? 0.f : mode == 2 ? -100.f : mode == 3 ? -2.f : -10.f;
      q->defaultValue = mode <= 1 ? 1.f : mode == 2 ? 2.f : mode == 3 ? 1.5f : 1.1f;
      normal = mode <= 1 ? 0.f : mode == 2 ? 10.f : mode == 3 ? 5.f : 1.f;
      scale = mode == 4 ? 10.f : mode == 3 ? 2.f : 1.f;
      offset = mode <= 1 ? 0.f : -1.f;
    }
    int clip = static_cast<int>(params[CLIP_PARAM].getValue());
    int dcBlock = static_cast<int>(params[DCBLOCK_PARAM].getValue());
    int vcaMode = static_cast<int>(params[VCAMODE_PARAM].getValue());
    bool exclude = static_cast<bool>(params[EXCLUDE_PARAM].getValue());
    float preOff[4], postOff[4];
    for (int ch=0; ch<4; ch++) {
      int Cnt = mode == 1 ? std::max({1, inputs[LEFT_INPUTS+ch].getChannels(), inputs[RIGHT_INPUTS+ch].getChannels()}) : 1;
      preOff[ch] = offsetExpander ? offsetExpander->params[PRE_OFFSET_PARAM+ch].getValue() * Cnt : 0.f;
      postOff[ch] = offsetExpander ? offsetExpander->params[POST_OFFSET_PARAM+ch].getValue() * Cnt : 0.f;
    }

    int inChannels[4];
    int channels = mode == 1 ? 1 : std::max({1, inputs[LEFT_CHAIN_INPUT].getChannels(), inputs[RIGHT_CHAIN_INPUT].getChannels(), inputs[MIX_CV_INPUT].getChannels()});
    int loopChannels = channels;
    for (int i=0; i<4; i++){
      inChannels[i] = mode == 1 ? 1 : std::max({
        1, inputs[CV_INPUTS+i].getChannels(),
        inputs[LEFT_INPUTS+i].getChannels(), inputs[RIGHT_INPUTS+i].getChannels()
      });
      if (inChannels[i] > channels) {
        loopChannels = inChannels[i];
        if (!exclude || (!outputs[LEFT_OUTPUTS+i].isConnected() && !outputs[RIGHT_OUTPUTS+i].isConnected()))
          channels = inChannels[i];
      }
    }
    simd::float_4 leftOut, rightOut, leftRtn, rightRtn, cv, leftChannel[4]{}, rightChannel[4]{};
    bool sendChain;
    float channelScale;
    float fadeLevel[5];
    fadeLevel[4] = 1.f; //initialize final mix fade factor
    bool isFadeType = fadeExpander && fadeExpander->mixType == MIXFADE_TYPE;
    for (int c=0; c<loopChannels; c+=4){
      int vcaOversample = 0;
      leftOut = mode==1 ? inputs[LEFT_CHAIN_INPUT].getVoltageSum() : inputs[LEFT_CHAIN_INPUT].getPolyVoltageSimd<simd::float_4>(c);
      if (inputs[RIGHT_CHAIN_INPUT].isConnected())
        rightOut = mode==1 ? inputs[RIGHT_CHAIN_INPUT].getVoltageSum() : inputs[RIGHT_CHAIN_INPUT].getPolyVoltageSimd<simd::float_4>(c);
      else
        rightOut = leftOut;
      for (int i=0; i<4; i++){
        cv = inputs[CV_INPUTS+i].isConnected() ? (mode==1 ? inputs[CV_INPUTS+i].getVoltageSum()/10.f : inputs[CV_INPUTS+i].getPolyVoltageSimd<simd::float_4>(c)/10.f) : 1.0f;
        if (inputs[RIGHT_INPUTS+i].isConnected()) {
          leftChannel[i] = (mode==1 ? inputs[LEFT_INPUTS+i].getVoltageSum() : inputs[LEFT_INPUTS+i].getPolyVoltageSimd<simd::float_4>(c)) + preOff[i];
          rightChannel[i] = (mode==1 ? inputs[RIGHT_INPUTS+i].getVoltageSum() : inputs[RIGHT_INPUTS+i].getPolyVoltageSimd<simd::float_4>(c)) + preOff[i];
        }
        else {
          leftChannel[i] = (mode==1 ? inputs[LEFT_INPUTS+i].getVoltageSum() : inputs[LEFT_INPUTS+i].getNormalPolyVoltageSimd<simd::float_4>(normal, c)) + preOff[i];
          rightChannel[i] = leftChannel[i];
        }
        vcaOversample = vcaMode>=4 && inputs[CV_INPUTS+i].isConnected() && (inputs[LEFT_INPUTS+i].isConnected() || inputs[RIGHT_INPUTS+i].isConnected()) ? 4 : 1;
        channelScale = (params[LEVEL_PARAMS+i].getValue()+offset)*scale;
        for (int s=0; s<vcaOversample; s++) {
          if (vcaOversample > 1) {
            cv = cvVcaBandlimit[i][c/4].process(s ? 0.f : cv*vcaOversample);
            leftChannel[i] = inLeftVcaBandlimit[i][c/4].process(s ? 0.f : leftChannel[i]*vcaOversample);
            rightChannel[i] = inRightVcaBandlimit[i][c/4].process(s ? 0.f : rightChannel[i]*vcaOversample);
          }
          if (vcaMode <= 1)
            cv = simd::clamp(cv, 0.f, 1.f);
          if (vcaMode == 1 || vcaMode == 3 || vcaMode == 5)
            cv = simd::sgn(cv)*simd::pow(simd::abs(cv), 4);
          leftChannel[i] *= channelScale*cv;
          rightChannel[i] *= channelScale*cv;
          if (vcaOversample > 1){
            leftChannel[i] = outLeftVcaBandlimit[i][c/4].process(leftChannel[i]);
            rightChannel[i] = outRightVcaBandlimit[i][c/4].process(rightChannel[i]);
          }
        }
        leftChannel[i] += postOff[i];
        outputs[LEFT_OUTPUTS+i].setVoltageSimd(leftChannel[i], c);
        if (exclude && outputs[LEFT_OUTPUTS+i].isConnected())
          leftChannel[i] = 0.f;
        rightChannel[i] += postOff[i];
        outputs[RIGHT_OUTPUTS+i].setVoltageSimd(rightChannel[i], c);
        if (exclude && outputs[RIGHT_OUTPUTS+i].isConnected())
          rightChannel[i] = 0.f;
      }
      for (unsigned int x=0; x<expandersCnt; x++){
        MixModule* exp = expanders[x];
        MixModule* soloMod = NULL;
        MixModule* muteMod = NULL;
        float shape;
        switch(exp->mixType) {
          case MIXMUTE_TYPE:
            muteMod = exp;
            soloMod = muteSoloExpander;
            break;
          case MIXSOLO_TYPE:
            muteMod = muteSoloExpander;
            soloMod = exp;
            break;
          case MIXPAN_TYPE:
            for (int i=0; i<4; i++) {
              simd::float_4 pan = simd::clamp(exp->params[PAN_PARAM+i].getValue() + exp->inputs[PAN_INPUT+i].getPolyVoltageSimd<simd::float_4>(c)*exp->params[PAN_CV_PARAM+i].getValue()/5.f, -1.f, 1.f);
              int panLaw = !inputs[RIGHT_INPUTS+i].isConnected() || stereoPanLaw==10 ? monoPanLaw : stereoPanLaw;
              switch (panLaw) {
                case 0: // 0 dB
                  leftChannel[i]  *= simd::ifelse(pan>0, 1.f - pan, 1.f);
                  rightChannel[i] *= simd::ifelse(pan<0, 1.f + pan, 1.f);
                  break;
                case 1: // +1.5 dB side
                  leftChannel[i]  *= simd::ifelse(pan>0, 1.f - pan, 1.f - pan*0.25f);
                  rightChannel[i] *= simd::ifelse(pan<0, 1.f + pan, 1.f + pan*0.25f);
                  break;
                case 2: // +3 dB side
                  leftChannel[i]  *= simd::ifelse(pan>0, 1.f - pan, 1.f - pan*0.5f);
                  rightChannel[i] *= simd::ifelse(pan<0, 1.f + pan, 1.f + pan*0.5f);
                  break;
                case 3: // +4.5 dB side
                  leftChannel[i]  *= simd::ifelse(pan>0, 1.f - pan, 1.f - pan*0.75f);
                  rightChannel[i] *= simd::ifelse(pan<0, 1.f + pan, 1.f + pan*0.75f);
                  break;
                case 4: // +6 dB side
                  leftChannel[i]  *= 1 - pan;
                  rightChannel[i] *= 1 + pan;
                  break;
                case 5: // -1.5 dB center
                  leftChannel[i]  *= simd::ifelse(pan>0, 1.f - pan, 1.f - pan*0.25f) * 0.875f;
                  rightChannel[i] *= simd::ifelse(pan<0, 1.f + pan, 1.f + pan*0.25f) * 0.875f;
                  break;
                case 6: // -3 dB center
                  leftChannel[i]  *= simd::ifelse(pan>0, 1.f - pan, 1.f - pan*0.5f) * 0.75f;
                  rightChannel[i] *= simd::ifelse(pan<0, 1.f + pan, 1.f + pan*0.5f) * 0.75f;
                  break;
                case 7: // -4.5 dB center
                  leftChannel[i]  *= simd::ifelse(pan>0, 1.f - pan, 1.f - pan*0.75f) * 0.625f;
                  rightChannel[i] *= simd::ifelse(pan<0, 1.f + pan, 1.f + pan*0.75f) * 0.625f;
                  break;
                case 8: // -6 dB center
                  leftChannel[i]  *= (1 - pan)*0.5f;
                  rightChannel[i] *= (1 + pan)*0.5f;
                  break;
                case 9: // True stereo pan
                  rightChannel[i] += simd::ifelse(pan>0, leftChannel[i] * pan, simd::float_4::zero());
                  rightChannel[i] *= 1.f + simd::ifelse(pan>0, simd::float_4::zero(), pan);
                  leftChannel[i] += simd::ifelse(pan>0, simd::float_4::zero(), rightChannel[i] * -pan);
                  leftChannel[i] *= 1.f - simd::ifelse(pan>0, pan, simd::float_4::zero());
              }
            }
            break;
          case MIXSEND_TYPE:
            if (!c) {
              if (softMute)
                exp->fade[0].process(args.sampleTime, !exp->params[SEND_MUTE_PARAM].getValue());
              else
                exp->fade[0].out = !exp->params[SEND_MUTE_PARAM].getValue();
            }
            leftRtn = exp->inputs[LEFT_RETURN_INPUT].getPolyVoltageSimd<simd::float_4>(c);
            rightRtn = exp->inputs[RIGHT_RETURN_INPUT].getPolyVoltageSimd<simd::float_4>(c);
            sendChain = exp->params[SEND_CHAIN_PARAM].getValue();
            exp->outputs[LEFT_SEND_OUTPUT].setVoltageSimd(
              (  leftChannel[0] * exp->params[SEND_PARAM+0].getValue()
               + leftChannel[1] * exp->params[SEND_PARAM+1].getValue()
               + leftChannel[2] * exp->params[SEND_PARAM+2].getValue()
               + leftChannel[3] * exp->params[SEND_PARAM+3].getValue()
               + (sendChain ? leftRtn : simd::float_4::zero())
              ) * exp->fade[0].out
              ,c
            );
            exp->outputs[RIGHT_SEND_OUTPUT].setVoltageSimd(
              (  rightChannel[0] * exp->params[SEND_PARAM+0].getValue()
               + rightChannel[1] * exp->params[SEND_PARAM+1].getValue()
               + rightChannel[2] * exp->params[SEND_PARAM+2].getValue()
               + rightChannel[3] * exp->params[SEND_PARAM+3].getValue()
               + (sendChain ? rightRtn : simd::float_4::zero())
              ) * exp->fade[0].out
              ,c
            );
            if (channels-c <= 4) {
              exp->outputs[LEFT_SEND_OUTPUT].setChannels(channels);
              exp->outputs[RIGHT_SEND_OUTPUT].setChannels(channels);
            }
            if (!sendChain) {
              leftOut  += leftRtn * exp->params[RETURN_PARAM].getValue();
              rightOut += rightRtn * exp->params[RETURN_PARAM].getValue();
            }
            break;
        }
        if (!c && muteMod && !muteMod->isBypassed()) {
          for (int i=0; i<5; i++) { //assumes MUTE_MIX_PARAM and MUTE_MIX_INPUT follow MUTE_PARAM AND MUTE_MIX_INPUT arrays
            int evnt = muteMod->muteCV[i].processEvent(muteMod->inputs[MUTE_INPUT+i].getVoltage(), 0.1f, 1.f);
            if (toggleMute && evnt>0)
              muteMod->params[MUTE_PARAM+i].setValue(!muteMod->params[MUTE_PARAM+i].getValue());
            if (!toggleMute && evnt)
              muteMod->params[MUTE_PARAM+i].setValue(muteMod->muteCV[i].isHigh());
          }
        }  
        if (!c && soloMod && !soloMod->isBypassed()) {
          for (int i=0; i<4; i++) {
            int evnt = soloMod->soloCV[i].processEvent(soloMod->inputs[SOLO_INPUT+i].getVoltage(), 0.1f, 1.f);
            if (toggleMute && evnt>0)
              soloMod->params[SOLO_PARAM+i].setValue(!soloMod->params[SOLO_PARAM+i].getValue());
            if (!toggleMute && evnt)
              soloMod->params[SOLO_PARAM+i].setValue(soloMod->soloCV[i].isHigh());
          }
        }  
        if (soloMod && !soloMod->isBypassed() && (
             soloMod->params[SOLO_PARAM+0].getValue() || soloMod->params[SOLO_PARAM+1].getValue() || 
             soloMod->params[SOLO_PARAM+2].getValue() || soloMod->params[SOLO_PARAM+3].getValue()
           )){
          for (int i=0; i<4; i++){
            if (!c) {
              if (fadeExpander && !fadeExpander->isBypassed()) {
                fade[i].rise = 1.f/std::max(softMute ? 0.025f : 0.f, fadeExpander->params[(isFadeType ? static_cast<int>(FADE_TIME_PARAM) : static_cast<int>(RISE_TIME_PARAM))+i].getValue());
                fade[i].fall = 1.f/std::max(softMute ? 0.025f : 0.f, fadeExpander->params[(isFadeType ? static_cast<int>(FADE_TIME_PARAM) : static_cast<int>(FALL_TIME_PARAM))+i].getValue());
                fade[i].process(args.sampleTime, soloMod->params[SOLO_PARAM+i].getValue());
                shape = fadeExpander->params[(isFadeType ? static_cast<int>(FADE_SHAPE_PARAM) : static_cast<int>(FADE2_SHAPE_PARAM))+i].getValue();
                fadeLevel[i] = crossfade(fade[i].out, shape>0.f ? 11.f*fade[i].out/(10.f*fade[i].out+1.f) : pow(fade[i].out,4), shape>0.f ? shape : -shape);
                fadeExpander->outputs[FADE_OUTPUT+i].setVoltage(fadeLevel[i]*10.f); // fade & fade2 outputs match
              }  
              else if (softMute){
                fade[i].rise = fade[i].fall = 40.f;
                fadeLevel[i] = fade[i].process(args.sampleTime, soloMod->params[SOLO_PARAM+i].getValue());
              }
              else
                fadeLevel[i] = fade[i].out = soloMod->params[SOLO_PARAM+i].getValue();
            }  
            leftChannel[i] *= fadeLevel[i];
            rightChannel[i] *= fadeLevel[i];
          }
        }
        else if (muteMod && !muteMod->isBypassed()) {
          for (int i=0; i<4; i++){
            if (!c) {
              if (fadeExpander && !fadeExpander->isBypassed()) {
                fade[i].rise = 1.f/std::max(softMute ? 0.025f : 0.f, fadeExpander->params[(isFadeType ? static_cast<int>(FADE_TIME_PARAM) : static_cast<int>(RISE_TIME_PARAM))+i].getValue());
                fade[i].fall = 1.f/std::max(softMute ? 0.025f : 0.f, fadeExpander->params[(isFadeType ? static_cast<int>(FADE_TIME_PARAM) : static_cast<int>(FALL_TIME_PARAM))+i].getValue());
                fade[i].process(args.sampleTime, !muteMod->params[MUTE_PARAM+i].getValue());
                shape = fadeExpander->params[(isFadeType ? static_cast<int>(FADE_SHAPE_PARAM) : static_cast<int>(FADE2_SHAPE_PARAM))+i].getValue();
                fadeLevel[i] = crossfade(fade[i].out, shape>0.f ? 11.f*fade[i].out/(10.f*fade[i].out+1.f) : pow(fade[i].out,4), shape>0.f ? shape : -shape);
                fadeExpander->outputs[FADE_OUTPUT+i].setVoltage(fadeLevel[i]*10.f); // fade & fade2 outputs match
              }  
              else if (softMute) {
                fade[i].rise = fade[i].fall = 40.f;
                fadeLevel[i] = fade[i].process(args.sampleTime, !muteMod->params[MUTE_PARAM+i].getValue());
              }
              else
                fadeLevel[i] = fade[i].out = !muteMod->params[MUTE_PARAM+i].getValue();
            }
            leftChannel[i]  *= fadeLevel[i];
            rightChannel[i] *= fadeLevel[i];
          }
        }
        if (!c && muteMod && !muteMod->isBypassed()){
          if (fadeExpander && !fadeExpander->isBypassed()) {
            fade[4].rise = 1.f/std::max(softMute ? 0.025f : 0.f, fadeExpander->params[isFadeType ? static_cast<int>(FADE_MIX_TIME_PARAM) : static_cast<int>(MIX_RISE_TIME_PARAM)].getValue());
            fade[4].fall = 1.f/std::max(softMute ? 0.025f : 0.f, fadeExpander->params[isFadeType ? static_cast<int>(FADE_MIX_TIME_PARAM) : static_cast<int>(MIX_FALL_TIME_PARAM)].getValue());
            fade[4].process(args.sampleTime, !muteMod->params[MUTE_MIX_PARAM].getValue());
            shape = fadeExpander->params[isFadeType ? static_cast<int>(FADE_MIX_SHAPE_PARAM) : static_cast<int>(FADE2_MIX_SHAPE_PARAM)].getValue();
            fadeLevel[4] = crossfade(fade[4].out, shape>0.f ? 11.f*fade[4].out/(10.f*fade[4].out+1.f) : pow(fade[4].out,4), shape>0.f ? shape : -shape);
            fadeExpander->outputs[FADE_MIX_OUTPUT].setVoltage(fadeLevel[4]); // fade & fade2 outputs match
          }  
          else if (softMute) {
            fade[4].rise = fade[4].fall = 40.f;
            fadeLevel[4] = fade[4].process(args.sampleTime, !muteMod->params[MUTE_MIX_PARAM].getValue());
          }
          else
            fadeLevel[4] = fade[4].out = !muteMod->params[MUTE_MIX_PARAM].getValue();
        }
      }
      float preMixOff = offsetExpander ? offsetExpander->params[PRE_MIX_OFFSET_PARAM].getValue() : 0.f;
      float postMixOff = offsetExpander ? offsetExpander->params[POST_MIX_OFFSET_PARAM].getValue() : 0.f;
      leftOut += leftChannel[0] + leftChannel[1] + leftChannel[2] + leftChannel[3] + preMixOff;
      rightOut += rightChannel[0] + rightChannel[1] + rightChannel[2] + rightChannel[3] + preMixOff;

      cv = inputs[MIX_CV_INPUT].isConnected() ? (mode == 1 ? inputs[MIX_CV_INPUT].getVoltage()/10.f : inputs[MIX_CV_INPUT].getPolyVoltageSimd<simd::float_4>(c)/10.f) : 1.0f;
      vcaOversample = vcaMode>=4 && inputs[MIX_CV_INPUT].isConnected() ? 4 : 1;

      if (dcBlock && dcBlock < 3) {
        leftOut = leftDcBlockBeforeFilter[c/4].process(leftOut);
        rightOut = rightDcBlockBeforeFilter[c/4].process(rightOut);
      }

      if (clip == 4) { // hard pre
        leftOut = clamp(leftOut, -10.f, 10.f);
        rightOut = clamp(rightOut, -10.f, 10.f);
      }
      if (clip == 5) { // soft pre
        leftOut = softClip(leftOut);
        rightOut = softClip(rightOut);
      }
      if (clip==6 && vcaOversample==1) { // soft pre
        for (int i=0; i<oversample; i++){
          leftOut = leftUpSample[c/4].process(i ? simd::float_4::zero() : leftOut*oversample);
          leftOut = softClip(leftOut);
          leftOut = leftDownSample[c/4].process(leftOut);
          rightOut = rightUpSample[c/4].process(i ? simd::float_4::zero() : rightOut*oversample);
          rightOut = softClip(rightOut);
          rightOut = rightDownSample[c/4].process(rightOut);
        }
      }
      for (int s=0; s<vcaOversample; s++) {
        if (vcaOversample > 1) {
          cv = cvVcaBandlimit[4][c/4].process(s ? 0.f : cv*vcaOversample);
          leftOut = inLeftVcaBandlimit[4][c/4].process( s ? 0.f : leftOut*vcaOversample);
          rightOut = inRightVcaBandlimit[4][c/4].process( s ? 0.f : rightOut*vcaOversample);
        }
        if (vcaMode <= 1)
          cv = simd::clamp(cv, 0.f, 1.f);
        if (vcaMode == 1 || vcaMode == 3 || vcaMode == 5)
          cv = simd::sgn(cv)*simd::pow(simd::abs(cv), 4);
        if (clip == 6 && vcaOversample>1) {
          leftOut = softClip(leftOut);
          rightOut = softClip(rightOut);
        }
        leftOut *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale*cv;
        leftOut += postMixOff;
        rightOut *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale*cv;
        rightOut += postMixOff;
        if (clip==3 && vcaOversample>1) {
          leftOut = softClip(leftOut);
          rightOut = softClip(rightOut);
        }
        if (clip==7 && vcaOversample>1) {
          leftOut = softClip(leftOut*1.6667f) / 1.6667f;
          rightOut = softClip(rightOut*1.6667f) / 1.6667f;
        }
        if (vcaOversample > 1) {
          leftOut = outLeftVcaBandlimit[4][c/4].process(leftOut);
          rightOut = outRightVcaBandlimit[4][c/4].process(rightOut);
        }
      }

      if (clip == 1) { // hard post
        leftOut = clamp(leftOut, -10.f, 10.f);
        rightOut = clamp(rightOut, -10.f, 10.f);
      }    
      if (clip == 2) { // { soft post
        leftOut = softClip(leftOut);
        rightOut = softClip(rightOut);
      }    
      if ((clip==3 || clip==7) && vcaOversample==1) { // soft post
        for (int i=0; i<oversample; i++){
          leftOut = leftUpSample[c/4].process(i ? simd::float_4::zero() : leftOut*oversample);
          rightOut = rightUpSample[c/4].process(i ? simd::float_4::zero() : rightOut*oversample);
          if (clip==7) {
            leftOut = softClip(leftOut*1.6667f) / 1.6667f;
            rightOut = softClip(rightOut*1.6667f) / 1.6667f;
          }
          else {
            leftOut = softClip(leftOut);
            rightOut = softClip(rightOut);
          }
          leftOut = leftDownSample[c/4].process(leftOut);
          rightOut = rightDownSample[c/4].process(rightOut);
        }
      }  

      if (dcBlock == 3 || (dcBlock == 2 && clip)) {
        leftOut = leftDcBlockAfterFilter[c/4].process(leftOut);
        rightOut = rightDcBlockAfterFilter[c/4].process(rightOut);
      }

      leftOut  *= fadeLevel[4]; // Mix fade factor
      rightOut *= fadeLevel[4]; // Mix fade factor
      outputs[LEFT_MIX_OUTPUT].setVoltageSimd(leftOut, c);
      outputs[RIGHT_MIX_OUTPUT].setVoltageSimd(rightOut, c);
    }
    for (int i=0; i<4; i++){
      outputs[LEFT_OUTPUTS+i].setChannels(inChannels[i]);
      outputs[RIGHT_OUTPUTS+i].setChannels(inChannels[i]);
    }
    outputs[LEFT_MIX_OUTPUT].setChannels(channels);
    outputs[RIGHT_MIX_OUTPUT].setChannels(channels);
  }

};

struct VCAMix4StereoWidget : MixBaseWidget {

  VCAMix4StereoWidget(VCAMix4Stereo* module) {
    setModule(module);
    setVenomPanel("VCAMix4Stereo");

    addInput(createInputCentered<PolyPort>(Vec(51.240, 42.295), module, VCAMix4Stereo::CV_INPUTS+0));
    addInput(createInputCentered<PolyPort>(Vec(51.240, 73.035), module, VCAMix4Stereo::CV_INPUTS+1));
    addInput(createInputCentered<PolyPort>(Vec(51.240,103.775), module, VCAMix4Stereo::CV_INPUTS+2));
    addInput(createInputCentered<PolyPort>(Vec(51.240,134.515), module, VCAMix4Stereo::CV_INPUTS+3));
    addInput(createInputCentered<PolyPort>(Vec(51.240,168.254), module, VCAMix4Stereo::MIX_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(83.582, 42.295), module, VCAMix4Stereo::LEVEL_PARAMS+0));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(83.582, 73.035), module, VCAMix4Stereo::LEVEL_PARAMS+1));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(83.582, 103.775), module, VCAMix4Stereo::LEVEL_PARAMS+2));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(83.582, 134.515), module, VCAMix4Stereo::LEVEL_PARAMS+3));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(83.582,168.254), module, VCAMix4Stereo::MIX_LEVEL_PARAM));

    addParam(createLockableParamCentered<ModeSwitch>(Vec(114.282,59.5005), module, VCAMix4Stereo::MODE_PARAM));
    addParam(createLockableParamCentered<VCAModeSwitch>(Vec(114.282,90.2395), module, VCAMix4Stereo::VCAMODE_PARAM));
    addParam(createLockableParamCentered<DCBlockSwitch>(Vec(114.282,120.9795), module, VCAMix4Stereo::DCBLOCK_PARAM));
    addParam(createLockableParamCentered<ClipSwitch>(Vec(114.282,151.7195), module, VCAMix4Stereo::CLIP_PARAM));
    addParam(createLockableParamCentered<ExcludeSwitch>(Vec(20.718,151.7195), module, VCAMix4Stereo::EXCLUDE_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(20.718,209.400), module, VCAMix4Stereo::LEFT_INPUTS+0));
    addInput(createInputCentered<PolyPort>(Vec(20.718,241.320), module, VCAMix4Stereo::LEFT_INPUTS+1));
    addInput(createInputCentered<PolyPort>(Vec(20.718,273.240), module, VCAMix4Stereo::LEFT_INPUTS+2));
    addInput(createInputCentered<PolyPort>(Vec(20.718,305.160), module, VCAMix4Stereo::LEFT_INPUTS+3));
    addInput(createInputCentered<PolyPort>(Vec(20.718,340.434), module, VCAMix4Stereo::LEFT_CHAIN_INPUT));

    addInput(createInputCentered<PolyPort>(Vec(51.240,209.400), module, VCAMix4Stereo::RIGHT_INPUTS+0));
    addInput(createInputCentered<PolyPort>(Vec(51.240,241.320), module, VCAMix4Stereo::RIGHT_INPUTS+1));
    addInput(createInputCentered<PolyPort>(Vec(51.240,273.240), module, VCAMix4Stereo::RIGHT_INPUTS+2));
    addInput(createInputCentered<PolyPort>(Vec(51.240,305.160), module, VCAMix4Stereo::RIGHT_INPUTS+3));
    addInput(createInputCentered<PolyPort>(Vec(51.240,340.434), module, VCAMix4Stereo::RIGHT_CHAIN_INPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(83.582,209.400), module, VCAMix4Stereo::LEFT_OUTPUTS+0));
    addOutput(createOutputCentered<PolyPort>(Vec(83.582,241.320), module, VCAMix4Stereo::LEFT_OUTPUTS+1));
    addOutput(createOutputCentered<PolyPort>(Vec(83.582,273.240), module, VCAMix4Stereo::LEFT_OUTPUTS+2));
    addOutput(createOutputCentered<PolyPort>(Vec(83.582,305.160), module, VCAMix4Stereo::LEFT_OUTPUTS+3));
    addOutput(createOutputCentered<PolyPort>(Vec(83.582,340.434), module, VCAMix4Stereo::LEFT_MIX_OUTPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(114.282,209.400), module, VCAMix4Stereo::RIGHT_OUTPUTS+0));
    addOutput(createOutputCentered<PolyPort>(Vec(114.282,241.320), module, VCAMix4Stereo::RIGHT_OUTPUTS+1));
    addOutput(createOutputCentered<PolyPort>(Vec(114.282,273.240), module, VCAMix4Stereo::RIGHT_OUTPUTS+2));
    addOutput(createOutputCentered<PolyPort>(Vec(114.282,305.160), module, VCAMix4Stereo::RIGHT_OUTPUTS+3));
    addOutput(createOutputCentered<PolyPort>(Vec(114.282,340.434), module, VCAMix4Stereo::RIGHT_MIX_OUTPUT));
  }

};

Model* modelVCAMix4Stereo = createModel<VCAMix4Stereo, VCAMix4StereoWidget>("VCAMix4Stereo");

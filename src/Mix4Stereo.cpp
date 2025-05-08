// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "MixModule.hpp"
#include "math.hpp"
#include "Filter.hpp"

struct Mix4Stereo : MixBaseModule {
  enum ParamId {
    ENUMS(LEVEL_PARAMS, 4),
    MIX_LEVEL_PARAM,
    MODE_PARAM,
    CLIP_PARAM,
    DCBLOCK_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(LEFT_INPUT, 4),
    ENUMS(RIGHT_INPUT, 4),
    INPUTS_LEN
  };
  enum OutputId {
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
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
  OversampleFilter_4 leftUpSample[4]{}, leftDownSample[4]{}, rightUpSample[4]{}, rightDownSample[4]{};
  DCBlockFilter_4 leftDcBlockBeforeFilter[4]{}, rightDcBlockBeforeFilter[4]{},
                  leftDcBlockAfterFilter[4]{},  rightDcBlockAfterFilter[4]{};

  Mix4Stereo() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    mixType=MIX4ST_TYPE;
    baseMod = true;
    stereo = true;
    for (int i=0; i < 4; i++){
      configParam(LEVEL_PARAMS+i, 0.f, 2.f, 1.f, string::f("Channel %d level", i + 1), " dB", -10.f, 20.f);
      configInput(LEFT_INPUT+i, string::f("Left channel %d", i + 1));
      configInput(RIGHT_INPUT+i, string::f("Right channel %d", i + 1))->description = string::f("Normalled to left channel %d input", i + 1);
    }
    configParam(MIX_LEVEL_PARAM, 0.f, 2.f, 1.f, "Mix level", " dB", -10.f, 20.f);
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 4.f, 0.f, "Level Mode", {
      "Unipolar dB (audio x2)", "Unipolar poly sum dB (audio x2)", "Bipolar % (CV)", "Bipolar x2 (CV)", "Bipolar x10 (CV)"
    });
    configSwitch<FixedSwitchQuantity>(DCBLOCK_PARAM, 0.f, 3.f, 0.f, "Mix DC Block", {"Off", "Before clipping", "Before and after clipping", "After clipping"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 7.f, 0.f, "Mix Clipping", {"Off", "Hard post-level at 10V", "Soft post-level at 10V", "Soft oversampled post-levl at 10V", 
                                                                                         "Hard pre-level at 10V", "Soft pre-level at 10V", "Soft oversampled pre-level at 10V",
                                                                                         "Saturate (Soft oversampled post-level at 6V)"});
    configOutput(LEFT_OUTPUT, "Left Mix");
    configOutput(RIGHT_OUTPUT, "Right Mix");
    oversampleStages = 5;
    setOversample();
  }

  void setOversample() override {
    for (int i=0; i<4; i++){
      leftUpSample[i].setOversample(oversample, oversampleStages);
      leftDownSample[i].setOversample(oversample, oversampleStages);
      rightUpSample[i].setOversample(oversample, oversampleStages);
      rightDownSample[i].setOversample(oversample, oversampleStages);
    }
  }

  void onReset(const ResetEvent& e) override {
    mode = -1;
    setOversample();
    Module::onReset(e);
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
      connected[0] != (inputs[LEFT_INPUT + 0].isConnected() || inputs[RIGHT_INPUT + 0].isConnected()) ||
      connected[1] != (inputs[LEFT_INPUT + 1].isConnected() || inputs[RIGHT_INPUT + 1].isConnected()) ||
      connected[2] != (inputs[LEFT_INPUT + 2].isConnected() || inputs[RIGHT_INPUT + 2].isConnected()) ||
      connected[3] != (inputs[LEFT_INPUT + 3].isConnected() || inputs[RIGHT_INPUT + 3].isConnected())
    ){
      mode = static_cast<int>(params[MODE_PARAM].getValue());
      ParamQuantity* q;
      for (int i=0; i<4; i++) {
        connected[i] = inputs[LEFT_INPUT + i].isConnected() || inputs[RIGHT_INPUT + i].isConnected();
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
    float preOff[4], postOff[4];
    for (int ch=0; ch<4; ch++) {
      int Cnt = mode == 1 ? std::max({1, inputs[LEFT_INPUT+ch].getChannels(), inputs[RIGHT_INPUT+ch].getChannels()}) : 1;
      preOff[ch] = offsetExpander ? offsetExpander->params[PRE_OFFSET_PARAM+ch].getValue() * Cnt : 0.f;
      postOff[ch] = offsetExpander ? offsetExpander->params[POST_OFFSET_PARAM+ch].getValue() * Cnt : 0.f;
    }

    int channels = mode == 1 ? 1 : std::max({1,
      inputs[LEFT_INPUT].getChannels(), inputs[LEFT_INPUT+1].getChannels(), inputs[LEFT_INPUT+2].getChannels(), inputs[LEFT_INPUT+3].getChannels(),
      inputs[RIGHT_INPUT].getChannels(), inputs[RIGHT_INPUT+1].getChannels(), inputs[RIGHT_INPUT+2].getChannels(), inputs[RIGHT_INPUT+3].getChannels()
    });
    simd::float_4 leftOut, rightOut, leftRtn, rightRtn, leftChannel[4], rightChannel[4];
    bool sendChain;
    float fadeLevel[5];
    fadeLevel[4] = 1.f; //initialize final mix fade factor
    bool isFadeType = fadeExpander && fadeExpander->mixType == MIXFADE_TYPE;
    for (int c=0; c<channels; c+=4){  // c = polyphonic channel
      leftOut = simd::float_4::zero();
      rightOut = simd::float_4::zero();
      if (mode == 1){
        for (int i=0; i<4; i++){
          float channelScale = (params[LEVEL_PARAMS+i].getValue()+offset)*scale;
          leftChannel[i] = (inputs[LEFT_INPUT+i].getVoltageSum() + preOff[i]) * channelScale + postOff[i];
          rightChannel[i] = inputs[RIGHT_INPUT+i].isConnected() 
                          ? (inputs[RIGHT_INPUT+i].getVoltageSum() + preOff[i]) * channelScale + postOff[i]
                          : leftChannel[i];
        }
      }
      else {
        for (int i=0; i<4; i++){
          float channelScale = (params[LEVEL_PARAMS+i].getValue()+offset)*scale;
          if (connected[i]){
            leftChannel[i] = (inputs[LEFT_INPUT+i].getPolyVoltageSimd<simd::float_4>(c) + preOff[i]) * channelScale + postOff[i];
            rightChannel[i] = inputs[RIGHT_INPUT+i].isConnected()
                            ? (inputs[RIGHT_INPUT+i].getPolyVoltageSimd<simd::float_4>(c) + preOff[i]) * channelScale + postOff[i]
                            : leftChannel[i];
          }
          else {
            leftChannel[i] = rightChannel[i] = (normal + preOff[i]) * channelScale + postOff[i];
          }
        }
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
              int panLaw = !inputs[RIGHT_INPUT+i].isConnected() || stereoPanLaw==10 ? monoPanLaw : stereoPanLaw;
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

      if (clip <= 3 || clip == 7) {
        leftOut *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale + postMixOff;
        rightOut *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale + postMixOff;
      }
      if (dcBlock && dcBlock <= 2){ // no oversample applied during DC removal
        leftOut = leftDcBlockBeforeFilter[c/4].process(leftOut);
        rightOut = rightDcBlockBeforeFilter[c/4].process(rightOut);
      }
      if (clip == 1 || clip ==4){
        leftOut = clamp(leftOut, -10.f, 10.f);
        rightOut = clamp(rightOut, -10.f, 10.f);
      }
      if (clip == 2 || clip == 5){
        leftOut = softClip(leftOut);
        rightOut = softClip(rightOut);
      }
      if (clip == 3 || clip >= 6){
        for (int i=0; i<oversample; i++){
          leftOut = leftUpSample[c/4].process(i ? simd::float_4::zero() : leftOut*oversample);
          leftOut = (clip != 7) ? softClip(leftOut) : (softClip(leftOut*1.6667f) / 1.6667f);
          leftOut = leftDownSample[c/4].process(leftOut);
          rightOut = rightUpSample[c/4].process(i ? simd::float_4::zero() : rightOut*oversample);
          rightOut = (clip != 7) ? softClip(rightOut) : (softClip(rightOut*1.6667f) / 1.6667f);
          rightOut = rightDownSample[c/4].process(rightOut);
        }
      }
      if (dcBlock == 3 || (dcBlock == 2 && clip)){ // no oversample applied during DC removal
        leftOut = leftDcBlockAfterFilter[c/4].process(leftOut);
        rightOut = rightDcBlockAfterFilter[c/4].process(rightOut);
      }
      if (clip > 3 && clip < 7) {
        leftOut *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale + postMixOff;
        rightOut *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale + postMixOff;
      }
      leftOut  *= fadeLevel[4]; // Mix fade factor
      rightOut *= fadeLevel[4]; // Mix fade factor
      outputs[LEFT_OUTPUT].setVoltageSimd(leftOut, c);
      outputs[RIGHT_OUTPUT].setVoltageSimd(rightOut, c);
    }
    outputs[LEFT_OUTPUT].setChannels(channels);
    outputs[RIGHT_OUTPUT].setChannels(channels);
  }

};

struct Mix4StereoWidget : MixBaseWidget {
  
  Mix4StereoWidget(Mix4Stereo* module) {
    setModule(module);
    setVenomPanel("Mix4Stereo");

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(37.5, 34.295), module, Mix4Stereo::LEVEL_PARAMS+0));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(37.5, 66.535), module, Mix4Stereo::LEVEL_PARAMS+1));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(37.5, 98.775), module, Mix4Stereo::LEVEL_PARAMS+2));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(37.5,131.014), module, Mix4Stereo::LEVEL_PARAMS+3));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(37.5,168.254), module, Mix4Stereo::MIX_LEVEL_PARAM));
    addParam(createLockableParamCentered<ModeSwitch>(Vec(62.443,50.415), module, Mix4Stereo::MODE_PARAM));
    addParam(createLockableParamCentered<DCBlockSwitch>(Vec(62.443,82.655), module, Mix4Stereo::DCBLOCK_PARAM));
    addParam(createLockableParamCentered<ClipSwitch>(Vec(62.443,114.895), module, Mix4Stereo::CLIP_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(21.812,201.993), module, Mix4Stereo::LEFT_INPUT+0));
    addInput(createInputCentered<PolyPort>(Vec(21.812,235.233), module, Mix4Stereo::LEFT_INPUT+1));
    addInput(createInputCentered<PolyPort>(Vec(21.812,268.473), module, Mix4Stereo::LEFT_INPUT+2));
    addInput(createInputCentered<PolyPort>(Vec(21.812,301.712), module, Mix4Stereo::LEFT_INPUT+3));
    addOutput(createOutputCentered<PolyPort>(Vec(21.812,340.434), module, Mix4Stereo::LEFT_OUTPUT));

    addInput(createInputCentered<PolyPort>(Vec(53.189,201.993), module, Mix4Stereo::RIGHT_INPUT+0));
    addInput(createInputCentered<PolyPort>(Vec(53.189,235.233), module, Mix4Stereo::RIGHT_INPUT+1));
    addInput(createInputCentered<PolyPort>(Vec(53.189,268.473), module, Mix4Stereo::RIGHT_INPUT+2));
    addInput(createInputCentered<PolyPort>(Vec(53.189,301.712), module, Mix4Stereo::RIGHT_INPUT+3));
    addOutput(createOutputCentered<PolyPort>(Vec(53.189,340.434), module, Mix4Stereo::RIGHT_OUTPUT));
  }

};

Model* modelMix4Stereo = createModel<Mix4Stereo, Mix4StereoWidget>("Mix4Stereo");

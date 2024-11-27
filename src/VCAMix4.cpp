// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "MixModule.hpp"
#include "math.hpp"
#include <cfloat>
#include "Filter.hpp"

struct VCAMix4 : MixBaseModule {
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
    ENUMS(CV_INPUTS, 4),
    MIX_CV_INPUT,
    ENUMS(INPUTS, 4),
    CHAIN_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(OUTPUTS, 4),
    MIX_OUTPUT,
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
  int oversample = 4;
  OversampleFilter_4 outUpSample[4]{}, outDownSample[4]{}, cvVcaBandlimit[5][4]{}, inVcaBandlimit[5][4]{}, outVcaBandlimit[5][4];
  DCBlockFilter_4 dcBlockBeforeFilter[4]{}, dcBlockAfterFilter[4]{};

  VCAMix4() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    mixType = VCAMIX4_TYPE;
    baseMod = true;
    for (int i=0; i < 4; i++){
      configInput(CV_INPUTS+i, string::f("Channel %d CV", i + 1));
      configParam(LEVEL_PARAMS+i, 0.f, 2.f, 1.f, string::f("Channel %d level", i + 1), " dB", -10.f, 20.f);
      configInput(INPUTS+i, string::f("Channel %d", i + 1));
      configOutput(OUTPUTS+i, string::f("Channel %d", i + 1));
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
    configInput(CHAIN_INPUT, "Chain");
    configOutput(MIX_OUTPUT, "Mix");
    for (int i=0; i<4; i++)
      configBypass(INPUTS+i, OUTPUTS+i);
    oversampleStages = 5;
    setOversample();
  }

  void setOversample() override {
    for (int i=0; i<4; i++){
      outUpSample[i].setOversample(oversample, oversampleStages);
      outDownSample[i].setOversample(oversample, oversampleStages);
      for (int j=0; j<5; j++){
        cvVcaBandlimit[j][i].setOversample(oversample, oversampleStages);
        inVcaBandlimit[j][i].setOversample(oversample, oversampleStages);
        outVcaBandlimit[j][i].setOversample(oversample, oversampleStages);
      }
    }
  }

  void onReset(const ResetEvent& e) override {
    mode = -1;
    setOversample();
    Module::onReset(e);
  }

  void process(const ProcessArgs& args) override {
    MixBaseModule::process(args);
    if( static_cast<int>(params[MODE_PARAM].getValue()) != mode ||
        connected[0] != inputs[INPUTS + 0].isConnected() ||
        connected[1] != inputs[INPUTS + 1].isConnected() ||
        connected[2] != inputs[INPUTS + 2].isConnected() ||
        connected[3] != inputs[INPUTS + 3].isConnected()
    ){
      mode = static_cast<int>(params[MODE_PARAM].getValue());
      ParamQuantity* q;
      for (int i=0; i<4; i++) {
        connected[i] = inputs[INPUTS + i].isConnected();
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
    for (int i=0; i<4; i++) {
      int cnt = mode == 1 ? std::max({1,inputs[INPUTS+i].getChannels()}) : 1;
      preOff[i] = offsetExpander ? offsetExpander->params[PRE_OFFSET_PARAM+i].getValue() * cnt : 0.f;
      postOff[i] = offsetExpander ? offsetExpander->params[POST_OFFSET_PARAM+i].getValue() * cnt : 0.f;
    }

    int inChannels[4];
    int channels = mode==1 ? 1 : std::max({1, inputs[CHAIN_INPUT].getChannels(), inputs[MIX_CV_INPUT].getChannels()});
    int loopChannels = channels;
    for (int i=0; i<4; i++) {
      inChannels[i] = mode == 1 ? 1 : std::max({1, inputs[CV_INPUTS+i].getChannels(), inputs[INPUTS+i].getChannels()});
      if (inChannels[i] > channels){
        loopChannels = inChannels[i];
        if (!exclude || !outputs[OUTPUTS+i].isConnected())
          channels = inChannels[i];
      }
    }
    simd::float_4 channel[4], out, rtn, cv;
    bool sendChain;
    float fadeLevel[5];
    fadeLevel[4] = 1.f; //initialize final mix fade factor
    bool isFadeType = fadeExpander && fadeExpander->mixType == MIXFADE_TYPE;
    for (int c=0; c<loopChannels; c+=4){
      out = mode==1 ? inputs[CHAIN_INPUT].getVoltageSum() : inputs[CHAIN_INPUT].getPolyVoltageSimd<simd::float_4>(c);
      for (int i=0; i<4; i++){
        cv = inputs[CV_INPUTS+i].isConnected() ? (mode == 1 ? inputs[CV_INPUTS+i].getVoltageSum()/10.f : inputs[CV_INPUTS+i].getPolyVoltageSimd<simd::float_4>(c) / 10.f) : 1.0f;
        channel[i] = preOff[i] + (mode == 1 ? inputs[INPUTS+i].getVoltageSum() : inputs[INPUTS+i].getNormalPolyVoltageSimd<simd::float_4>(normal, c));
        int vcaOversample = vcaMode>=4 && inputs[CV_INPUTS+i].isConnected() && inputs[INPUTS+i].isConnected() ? 4 : 1;
        for (int s=0; s<vcaOversample; s++){
          if (vcaOversample > 1) {
            cv = cvVcaBandlimit[i][c/4].process(s ? 0.f : cv*vcaOversample);
            channel[i] = inVcaBandlimit[i][c/4].process( s ? 0.f : channel[i]*vcaOversample);
          }
          if (vcaMode <= 1)
            cv = simd::clamp(cv, 0.f, 1.f);
          if (vcaMode == 1 || vcaMode == 3 || vcaMode == 5)
            cv = simd::sgn(cv)*simd::pow(simd::abs(cv), 4);
          channel[i] *= (params[LEVEL_PARAMS+i].getValue()+offset)*scale*cv;
          if (vcaOversample > 1) {
            channel[i] = outVcaBandlimit[i][c/4].process(channel[i]);
          }
        }
        channel[i] += postOff[i];
        outputs[OUTPUTS+i].setVoltageSimd(channel[i], c);
        if (exclude && outputs[OUTPUTS+i].isConnected())
          channel[i] = 0.f;
      }

      for (unsigned int x=0; x<expanders.size(); x++){
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
          case MIXSEND_TYPE:
            if (!c) {
              if (softMute)
                exp->fade[0].process(args.sampleTime, !exp->params[SEND_MUTE_PARAM].getValue());
              else
                exp->fade[0].out = !exp->params[SEND_MUTE_PARAM].getValue();
            }
            rtn = exp->inputs[LEFT_RETURN_INPUT].getPolyVoltageSimd<simd::float_4>(c);
            sendChain = exp->params[SEND_CHAIN_PARAM].getValue();
            exp->outputs[LEFT_SEND_OUTPUT].setVoltageSimd(
                 (  channel[0] * exp->params[SEND_PARAM+0].getValue()
                  + channel[1] * exp->params[SEND_PARAM+1].getValue()
                  + channel[2] * exp->params[SEND_PARAM+2].getValue()
                  + channel[3] * exp->params[SEND_PARAM+3].getValue()
                  + (sendChain ? rtn : simd::float_4::zero())
                 ) * exp->fade[0].out
                 ,c
            );
            if (channels-c <= 4) {
              exp->outputs[LEFT_SEND_OUTPUT].setChannels(channels);
              exp->outputs[RIGHT_SEND_OUTPUT].setVoltage(0.f);
              exp->outputs[RIGHT_SEND_OUTPUT].setChannels(1);
            }
            if (!sendChain)
              out += rtn * exp->params[RETURN_PARAM].getValue();
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
            channel[i] *= fadeLevel[i];
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
            channel[i] *= fadeLevel[i];
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
      out += channel[0] + channel[1] + channel[2] + channel[3] + preMixOff;
      cv = inputs[MIX_CV_INPUT].isConnected() ? (mode == 1 ? inputs[MIX_CV_INPUT].getVoltageSum()/10.f : inputs[MIX_CV_INPUT].getPolyVoltageSimd<simd::float_4>(c)/10.f) : 1.0f;
      int vcaOversample = vcaMode>=4 && inputs[MIX_CV_INPUT].isConnected() ? 4 : 1;

      if (dcBlock && dcBlock < 3)
        out = dcBlockBeforeFilter[c/4].process(out);

      if (clip == 4) // hard pre
        out = clamp(out, -10.f, 10.f);
      if (clip == 5) // soft pre
        out = softClip(out);
      if (clip==6 && vcaOversample==1) { // soft pre
        for (int i=0; i<oversample; i++){
          out = outUpSample[c/4].process(i ? simd::float_4::zero() : out*oversample);
          out = softClip(out);
          out = outDownSample[c/4].process(out);
        }
      }
      for (int s=0; s<vcaOversample; s++) {
        if (vcaOversample > 1) {
          cv = cvVcaBandlimit[4][c/4].process(s ? 0.f : cv*vcaOversample);
          out = inVcaBandlimit[4][c/4].process( s ? 0.f : out*vcaOversample);
        }
        if (vcaMode <= 1)
          cv = simd::clamp(cv, 0.f, 1.f);
        if (vcaMode == 1 || vcaMode == 3 || vcaMode == 5)
          cv = simd::sgn(cv)*simd::pow(simd::abs(cv), 4);
        if (clip == 6 && vcaOversample>1)
          out = softClip(out);
        out *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale*cv;
        out += postMixOff;
        if (clip==3 && vcaOversample>1)
          out = softClip(out);
        if (clip==7 && vcaOversample>1)
          out = softClip(out*1.6667f) / 1.6667f;
        if (vcaOversample > 1) {
          out = outVcaBandlimit[4][c/4].process(out);
        }
      }

      if (clip == 1) // hard post
        out = clamp(out, -10.f, 10.f);
      if (clip == 2) // soft post
        out = softClip(out);
      if ((clip==3 || clip==7) && vcaOversample==1) { // soft post
        for (int i=0; i<oversample; i++){
          out = outUpSample[c/4].process(i ? simd::float_4::zero() : out*oversample);
          if (clip==7)
            out = softClip(out*1.6667f) / 1.6667f;
          else
            out = softClip(out);
          out = outDownSample[c/4].process(out);
        }
      }  

      if (dcBlock == 3 || (dcBlock == 2 && clip))
        out = dcBlockAfterFilter[c/4].process(out);
      out *= fadeLevel[4]; // Mix fade factor
      outputs[MIX_OUTPUT].setVoltageSimd(out, c);
    }
    for (int i=0; i<4; i++)
      outputs[OUTPUTS+i].setChannels(inChannels[i]);
    outputs[MIX_OUTPUT].setChannels(channels);
  }

};

struct VCAMix4Widget : MixBaseWidget {

  VCAMix4Widget(VCAMix4* module) {
    setModule(module);
    setVenomPanel("VCAMix4");

    addInput(createInputCentered<PolyPort>(Vec(21.329, 42.295), module, VCAMix4::CV_INPUTS+0));
    addInput(createInputCentered<PolyPort>(Vec(21.329, 73.035), module, VCAMix4::CV_INPUTS+1));
    addInput(createInputCentered<PolyPort>(Vec(21.329,103.775), module, VCAMix4::CV_INPUTS+2));
    addInput(createInputCentered<PolyPort>(Vec(21.329,134.515), module, VCAMix4::CV_INPUTS+3));
    addInput(createInputCentered<PolyPort>(Vec(21.329,168.254), module, VCAMix4::MIX_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.671, 42.295), module, VCAMix4::LEVEL_PARAMS+0));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.671, 73.035), module, VCAMix4::LEVEL_PARAMS+1));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.671, 103.775), module, VCAMix4::LEVEL_PARAMS+2));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.671, 134.515), module, VCAMix4::LEVEL_PARAMS+3));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(53.671,168.254), module, VCAMix4::MIX_LEVEL_PARAM));

    addParam(createLockableParamCentered<ModeSwitch>(Vec(67.9055,59.6655), module, VCAMix4::MODE_PARAM));
    addParam(createLockableParamCentered<VCAModeSwitch>(Vec(67.9055,90.4045), module, VCAMix4::VCAMODE_PARAM));
    addParam(createLockableParamCentered<DCBlockSwitch>(Vec(67.9055,121.1445), module, VCAMix4::DCBLOCK_PARAM));
    addParam(createLockableParamCentered<ClipSwitch>(Vec(67.9055,151.8845), module, VCAMix4::CLIP_PARAM));
    addParam(createLockableParamCentered<ExcludeSwitch>(Vec(7.2725,151.8845), module, VCAMix4::EXCLUDE_PARAM));

    addInput(createInputCentered<PolyPort>(Vec(21.329,209.400), module, VCAMix4::INPUTS+0));
    addInput(createInputCentered<PolyPort>(Vec(21.329,241.320), module, VCAMix4::INPUTS+1));
    addInput(createInputCentered<PolyPort>(Vec(21.329,273.240), module, VCAMix4::INPUTS+2));
    addInput(createInputCentered<PolyPort>(Vec(21.329,305.160), module, VCAMix4::INPUTS+3));
    addInput(createInputCentered<PolyPort>(Vec(21.329,340.434), module, VCAMix4::CHAIN_INPUT));

    addOutput(createOutputCentered<PolyPort>(Vec(53.671,209.400), module, VCAMix4::OUTPUTS+0));
    addOutput(createOutputCentered<PolyPort>(Vec(53.671,241.320), module, VCAMix4::OUTPUTS+1));
    addOutput(createOutputCentered<PolyPort>(Vec(53.671,273.240), module, VCAMix4::OUTPUTS+2));
    addOutput(createOutputCentered<PolyPort>(Vec(53.671,305.160), module, VCAMix4::OUTPUTS+3));
    addOutput(createOutputCentered<PolyPort>(Vec(53.671,340.434), module, VCAMix4::MIX_OUTPUT));
  }

};

Model* modelVCAMix4 = createModel<VCAMix4, VCAMix4Widget>("VCAMix4");

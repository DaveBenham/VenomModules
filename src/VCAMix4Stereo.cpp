// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "math.hpp"
#include <cfloat>
#include "Filter.hpp"

#define MODULE_NAME VCAMix4Stereo

struct VCAMix4Stereo : VenomModule {
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
    ENUMS(LEFT_INPUTS, 4),
    ENUMS(RIGHT_INPUTS, 4),
    LEFT_CHAIN_INPUT,
    RIGHT_CHAIN_INPUT,
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
  int oversample = 4;
  OversampleFilter_4 leftUpSample[4], leftDownSample[4], 
                     rightUpSample[4], rightDownSample[4];
  DCBlockFilter_4 leftDcBlockBeforeFilter[4], leftDcBlockAfterFilter[4], 
                  rightDcBlockBeforeFilter[4], rightDcBlockAfterFilter[4];

  VCAMix4Stereo() {
    struct FixedSwitchQuantity : SwitchQuantity {
      std::string getDisplayValueString() override {
        return labels[getValue()];
      }
    };
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i < 4; i++){
      configInput(CV_INPUTS+i, string::f("Channel %d CV", i + 1));
      configParam(LEVEL_PARAMS+i, 0.f, 2.f, 1.f, string::f("Channel %d level", i + 1), " dB", -10.f, 20.f);
      configInput(LEFT_INPUTS+i, string::f("Left channel %d", i + 1));
      configInput(RIGHT_INPUTS+i, string::f("Right channel %d", i + 1));
      configOutput(LEFT_OUTPUTS+i, string::f("Left channel %d", i + 1));
      configOutput(RIGHT_OUTPUTS+i, string::f("Right channel %d", i + 1));
    }
    configInput(MIX_CV_INPUT, "Mix CV");
    configParam(MIX_LEVEL_PARAM, 0.f, 2.f, 1.f, "Mix level", " dB", -10.f, 20.f);
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 4.f, 0.f, "Level Mode", {
      "Unipolar dB (audio x2)", "Unipolar poly sum dB (audio x2)", "Bipolar % (CV)", "Bipolar x2 (CV)", "Bipolar x10 (CV)"
    });
    configSwitch<FixedSwitchQuantity>(VCAMODE_PARAM, 0.f, 2.f, 0.f, "VCA Mode", {
      "Unipolar linear - CV clamped 0-10V", "Unipolar exponential - CV clamped 0-10V", "Bipolar linear - CV unclamped"
    });
    configSwitch<FixedSwitchQuantity>(DCBLOCK_PARAM, 0.f, 3.f, 0.f, "Mix DC Block", {"Off", "Before clipping", "Before and after clipping", "After clipping"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 3.f, 0.f, "Mix Clipping", {"Off", "Hard CV clipping", "Soft audio clipping", "Soft oversampled audio clipping"});
    configSwitch<FixedSwitchQuantity>(EXCLUDE_PARAM, 0.f, 1.f, 0.f, "Exclude Patched Outs from Mix", {"Off", "On"});
    configInput(LEFT_CHAIN_INPUT, "Left chain");
    configInput(RIGHT_CHAIN_INPUT, "Right chain");
    configOutput(LEFT_MIX_OUTPUT, "Left mix");
    configOutput(LEFT_MIX_OUTPUT, "Right mix");
    initOversample();
    initDCBlock();
  }

  void initOversample(){
    for (int i=0; i<4; i++){
      leftUpSample[i].setOversample(oversample);
      leftDownSample[i].setOversample(oversample);
      rightUpSample[i].setOversample(oversample);
      rightDownSample[i].setOversample(oversample);
    }
  }

  void initDCBlock(){
    float sampleTime = settings::sampleRate;
    for (int i=0; i<4; i++){
      leftDcBlockBeforeFilter[i].init(sampleTime);
      rightDcBlockBeforeFilter[i].init(sampleTime);
      leftDcBlockAfterFilter[i].init(sampleTime);
      rightDcBlockAfterFilter[i].init(sampleTime);
    }
  }

  void onReset(const ResetEvent& e) override {
    mode = -1;
    initOversample();
    Module::onReset(e);
  }

  void onSampleRateChange(const SampleRateChangeEvent& e) override {
    initDCBlock();
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
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

    int inChannels[4];
    for (int i=0; i<4; i++)
      inChannels[i] = mode == 1 ? 1 : std::max({
        1, inputs[CV_INPUTS+i].getChannels(), 
        inputs[LEFT_INPUTS+i].getChannels(), inputs[RIGHT_INPUTS+i].getChannels()
      });
    int channels = std::max({
      inChannels[0], inChannels[1], inChannels[2], inChannels[3],
      inputs[MIX_CV_INPUT].getChannels(), inputs[LEFT_CHAIN_INPUT].getChannels(), inputs[RIGHT_CHAIN_INPUT].getChannels()
    });
    simd::float_4 leftOut, rightOut, out, cv;
    float channelScale;
    for (int c=0; c<channels; c+=4){
      leftOut = inputs[LEFT_CHAIN_INPUT].getPolyVoltageSimd<simd::float_4>(c);
      rightOut = inputs[RIGHT_CHAIN_INPUT].getPolyVoltageSimd<simd::float_4>(c);
      if (mode == 1){
        for (int i=0; i<4; i++){
          cv = inputs[CV_INPUTS+i].getNormalPolyVoltageSimd<simd::float_4>(10.f, c) / 10.f;
          if (vcaMode <= 1)
            cv = simd::clamp(cv, 0.f, 1.f);
          if (vcaMode == 1)
            cv = simd::pow(cv, 4);
          channelScale = (params[LEVEL_PARAMS+i].getValue()+offset)*scale;
          out = inputs[LEFT_INPUTS+i].getVoltageSum() * channelScale * cv;
          outputs[LEFT_OUTPUTS+i].setVoltageSimd(out, c);
          if (!exclude || !outputs[LEFT_OUTPUTS+i].isConnected())
            leftOut += out;
          if (inputs[RIGHT_INPUTS+i].isConnected())
            out = inputs[RIGHT_INPUTS+i].getVoltageSum() * channelScale * cv;
          outputs[RIGHT_OUTPUTS+i].setVoltageSimd(out, c);
          if (!exclude || !outputs[RIGHT_OUTPUTS+i].isConnected())
            rightOut += out;
        }
      }
      else {
        for (int i=0; i<4; i++){
          cv = inputs[CV_INPUTS+i].getNormalPolyVoltageSimd<simd::float_4>(10.f, c) / 10.f;
          if (vcaMode <= 1)
            cv = simd::clamp(cv, 0.f, 1.f);
          if (vcaMode == 1)
            cv = simd::pow(cv, 4);
          channelScale = (params[LEVEL_PARAMS+i].getValue()+offset)*scale;
          if (connected[i]){
            out = inputs[LEFT_INPUTS+i].getPolyVoltageSimd<simd::float_4>(c) * channelScale * cv;
            outputs[LEFT_OUTPUTS+i].setVoltageSimd(out, c);
            if (!exclude || !outputs[LEFT_OUTPUTS+i].isConnected())
              leftOut += out;
            if (inputs[RIGHT_INPUTS+i].isConnected())
              out = inputs[RIGHT_INPUTS+i].getPolyVoltageSimd<simd::float_4>(c) * channelScale * cv;
            outputs[RIGHT_OUTPUTS+i].setVoltageSimd(out, c);
            if (!exclude || !outputs[RIGHT_OUTPUTS+i].isConnected())
              rightOut += out;
          }
          else {
            out = normal * channelScale;
            outputs[LEFT_OUTPUTS+i].setVoltageSimd(out, c);
            outputs[RIGHT_OUTPUTS+i].setVoltageSimd(out, c);
            if (!exclude || !outputs[LEFT_OUTPUTS+i].isConnected())
              leftOut += out;
            if (!exclude || !outputs[RIGHT_OUTPUTS+i].isConnected())
              rightOut += out;
          }
        }
      }
      cv = inputs[MIX_CV_INPUT].getNormalPolyVoltageSimd<simd::float_4>(10.f, c) / 10.f;
      if (vcaMode <= 1)
        cv = simd::clamp(cv, 0.f, 1.f);
      if (vcaMode == 1)
        cv = simd::pow(cv, 4);
      leftOut *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale*cv;
      rightOut *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale*cv;
      if (dcBlock && dcBlock <= 2){
        leftOut = leftDcBlockBeforeFilter[c/4].process(leftOut);
        rightOut = rightDcBlockBeforeFilter[c/4].process(rightOut);
      }
      if (clip == 1){
        leftOut = clamp(leftOut, -10.f, 10.f);
        rightOut = clamp(rightOut, -10.f, 10.f);
      }
      if (clip == 2){
        leftOut = softClip(leftOut);
        rightOut = softClip(rightOut);
      }
      if (clip == 3){
        for (int i=0; i<oversample; i++){
          leftOut = leftUpSample[c/4].process(i ? simd::float_4::zero() : leftOut*oversample);
          leftOut = softClip(leftOut);
          leftOut = leftDownSample[c/4].process(leftOut);
          rightOut = rightUpSample[c/4].process(i ? simd::float_4::zero() : rightOut*oversample);
          rightOut = softClip(rightOut);
          rightOut = rightDownSample[c/4].process(rightOut);
        }
      }
      if (dcBlock == 3 || (dcBlock == 2 && clip)){
        leftOut = leftDcBlockAfterFilter[c/4].process(leftOut);
        rightOut = rightDcBlockAfterFilter[c/4].process(rightOut);
      }
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

struct VCAMix4StereoWidget : VenomWidget {

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPinkButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  struct ClipSwitch : GlowingSvgSwitchLockable {
    ClipSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
    }
  };

  struct DCBlockSwitch : GlowingSvgSwitchLockable {
    DCBlockSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
    }
  };

  struct VCAModeSwitch : GlowingSvgSwitchLockable {
    VCAModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPinkButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
    }
  };

  struct ExcludeSwitch : GlowingSvgSwitchLockable {
    ExcludeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
    }
  };

  VCAMix4StereoWidget(VCAMix4Stereo* module) {
    setModule(module);
    setVenomPanel("VCAMix4Stereo");

    addInput(createInputCentered<PJ301MPort>(Vec(51.240, 42.295), module, VCAMix4Stereo::CV_INPUTS+0));
    addInput(createInputCentered<PJ301MPort>(Vec(51.240, 73.035), module, VCAMix4Stereo::CV_INPUTS+1));
    addInput(createInputCentered<PJ301MPort>(Vec(51.240,103.775), module, VCAMix4Stereo::CV_INPUTS+2));
    addInput(createInputCentered<PJ301MPort>(Vec(51.240,134.515), module, VCAMix4Stereo::CV_INPUTS+3));
    addInput(createInputCentered<PJ301MPort>(Vec(51.240,168.254), module, VCAMix4Stereo::MIX_CV_INPUT));

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

    addInput(createInputCentered<PJ301MPort>(Vec(20.718,209.400), module, VCAMix4Stereo::LEFT_INPUTS+0));
    addInput(createInputCentered<PJ301MPort>(Vec(20.718,241.320), module, VCAMix4Stereo::LEFT_INPUTS+1));
    addInput(createInputCentered<PJ301MPort>(Vec(20.718,273.240), module, VCAMix4Stereo::LEFT_INPUTS+2));
    addInput(createInputCentered<PJ301MPort>(Vec(20.718,305.160), module, VCAMix4Stereo::LEFT_INPUTS+3));
    addInput(createInputCentered<PJ301MPort>(Vec(20.718,340.434), module, VCAMix4Stereo::LEFT_CHAIN_INPUT));

    addInput(createInputCentered<PJ301MPort>(Vec(51.240,209.400), module, VCAMix4Stereo::RIGHT_INPUTS+0));
    addInput(createInputCentered<PJ301MPort>(Vec(51.240,241.320), module, VCAMix4Stereo::RIGHT_INPUTS+1));
    addInput(createInputCentered<PJ301MPort>(Vec(51.240,273.240), module, VCAMix4Stereo::RIGHT_INPUTS+2));
    addInput(createInputCentered<PJ301MPort>(Vec(51.240,305.160), module, VCAMix4Stereo::RIGHT_INPUTS+3));
    addInput(createInputCentered<PJ301MPort>(Vec(51.240,340.434), module, VCAMix4Stereo::RIGHT_CHAIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(Vec(83.582,209.400), module, VCAMix4Stereo::LEFT_OUTPUTS+0));
    addOutput(createOutputCentered<PJ301MPort>(Vec(83.582,241.320), module, VCAMix4Stereo::LEFT_OUTPUTS+1));
    addOutput(createOutputCentered<PJ301MPort>(Vec(83.582,273.240), module, VCAMix4Stereo::LEFT_OUTPUTS+2));
    addOutput(createOutputCentered<PJ301MPort>(Vec(83.582,305.160), module, VCAMix4Stereo::LEFT_OUTPUTS+3));
    addOutput(createOutputCentered<PJ301MPort>(Vec(83.582,340.434), module, VCAMix4Stereo::LEFT_MIX_OUTPUT));

    addOutput(createOutputCentered<PJ301MPort>(Vec(114.282,209.400), module, VCAMix4Stereo::RIGHT_OUTPUTS+0));
    addOutput(createOutputCentered<PJ301MPort>(Vec(114.282,241.320), module, VCAMix4Stereo::RIGHT_OUTPUTS+1));
    addOutput(createOutputCentered<PJ301MPort>(Vec(114.282,273.240), module, VCAMix4Stereo::RIGHT_OUTPUTS+2));
    addOutput(createOutputCentered<PJ301MPort>(Vec(114.282,305.160), module, VCAMix4Stereo::RIGHT_OUTPUTS+3));
    addOutput(createOutputCentered<PJ301MPort>(Vec(114.282,340.434), module, VCAMix4Stereo::RIGHT_MIX_OUTPUT));
  }

};

Model* modelVCAMix4Stereo = createModel<VCAMix4Stereo, VCAMix4StereoWidget>("VCAMix4Stereo");

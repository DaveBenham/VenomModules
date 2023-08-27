// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "math.hpp"
#include "Filter.hpp"

struct Mix4Stereo : VenomModule {
  enum ParamId {
    ENUMS(LEVEL_PARAMS, 4),
    MIX_LEVEL_PARAM,
    MODE_PARAM,
    CLIP_PARAM,
    DCBLOCK_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(LEFT_INPUTS, 4),
    ENUMS(RIGHT_INPUTS, 4),
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
  int oversample = 4;
  OversampleFilter_4 leftUpSample[4], leftDownSample[4], rightUpSample[4], rightDownSample[4];
  DCBlockFilter_4 leftDcBlockBeforeFilter[4], rightDcBlockBeforeFilter[4],
                  leftDcBlockAfterFilter[4],  rightDcBlockAfterFilter[4];

  Mix4Stereo() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i < 4; i++){
      configParam(LEVEL_PARAMS+i, 0.f, 2.f, 1.f, string::f("Channel %d level", i + 1), " dB", -10.f, 20.f);
      configInput(LEFT_INPUTS+i, string::f("Left channel %d", i + 1));
      configInput(RIGHT_INPUTS+i, string::f("Right channel %d", i + 1));
    }
    configParam(MIX_LEVEL_PARAM, 0.f, 2.f, 1.f, "Mix level", " dB", -10.f, 20.f);
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0.f, 4.f, 0.f, "Level Mode", {
      "Unipolar dB (audio x2)", "Unipolar poly sum dB (audio x2)", "Bipolar % (CV)", "Bipolar x2 (CV)", "Bipolar x10 (CV)"
    });
    configSwitch<FixedSwitchQuantity>(DCBLOCK_PARAM, 0.f, 3.f, 0.f, "Mix DC Block", {"Off", "Before clipping", "Before and after clipping", "After clipping"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 3.f, 0.f, "Mix Clipping", {"Off", "Hard CV clipping", "Soft audio clipping", "Soft oversampled audio clipping"});
    configOutput(LEFT_OUTPUT, "Left Mix");
    configOutput(RIGHT_OUTPUT, "Right Mix");
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

    int channels = mode == 1 ? 1 : std::max({1,
      inputs[LEFT_INPUTS].getChannels(), inputs[LEFT_INPUTS+1].getChannels(), inputs[LEFT_INPUTS+2].getChannels(), inputs[LEFT_INPUTS+3].getChannels(),
      inputs[RIGHT_INPUTS].getChannels(), inputs[RIGHT_INPUTS+1].getChannels(), inputs[RIGHT_INPUTS+2].getChannels(), inputs[RIGHT_INPUTS+3].getChannels()
    });
    simd::float_4 leftOut, rightOut, left;
    float channelScale, constant;
    for (int c=0; c<channels; c+=4){
      leftOut = simd::float_4::zero();
      rightOut = simd::float_4::zero();
      if (mode == 1){
        for (int i=0; i<4; i++){
          channelScale = (params[LEVEL_PARAMS+i].getValue()+offset)*scale;
          left = inputs[LEFT_INPUTS+i].getVoltageSum() * channelScale;
          leftOut += left;
          rightOut += inputs[RIGHT_INPUTS+i].isConnected() ?
                      inputs[RIGHT_INPUTS+i].getVoltageSum() * channelScale : left;
        }
      }
      else {
        for (int i=0; i<4; i++){
          channelScale = (params[LEVEL_PARAMS+i].getValue()+offset)*scale;
          if (connected[i]){
            left = inputs[LEFT_INPUTS+i].getPolyVoltageSimd<simd::float_4>(c);
            leftOut += left * channelScale;
            rightOut += inputs[RIGHT_INPUTS+i].getNormalPolyVoltageSimd<simd::float_4>(left, c) * channelScale;
          }
          else {
            constant = normal * channelScale;
            leftOut += constant;
            rightOut += constant;
          }
        }
      }
      leftOut *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale;
      rightOut *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale;
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
      outputs[LEFT_OUTPUT].setVoltageSimd(leftOut, c);
      outputs[RIGHT_OUTPUT].setVoltageSimd(rightOut, c);
    }
    outputs[LEFT_OUTPUT].setChannels(channels);
    outputs[RIGHT_OUTPUT].setChannels(channels);
  }

};

struct Mix4StereoWidget : VenomWidget {
  
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

    addInput(createInputCentered<PJ301MPort>(Vec(21.812,201.993), module, Mix4Stereo::LEFT_INPUTS+0));
    addInput(createInputCentered<PJ301MPort>(Vec(21.812,235.233), module, Mix4Stereo::LEFT_INPUTS+1));
    addInput(createInputCentered<PJ301MPort>(Vec(21.812,268.473), module, Mix4Stereo::LEFT_INPUTS+2));
    addInput(createInputCentered<PJ301MPort>(Vec(21.812,301.712), module, Mix4Stereo::LEFT_INPUTS+3));
    addOutput(createOutputCentered<PJ301MPort>(Vec(21.812,340.434), module, Mix4Stereo::LEFT_OUTPUT));

    addInput(createInputCentered<PJ301MPort>(Vec(53.189,201.993), module, Mix4Stereo::RIGHT_INPUTS+0));
    addInput(createInputCentered<PJ301MPort>(Vec(53.189,235.233), module, Mix4Stereo::RIGHT_INPUTS+1));
    addInput(createInputCentered<PJ301MPort>(Vec(53.189,268.473), module, Mix4Stereo::RIGHT_INPUTS+2));
    addInput(createInputCentered<PJ301MPort>(Vec(53.189,301.712), module, Mix4Stereo::RIGHT_INPUTS+3));
    addOutput(createOutputCentered<PJ301MPort>(Vec(53.189,340.434), module, Mix4Stereo::RIGHT_OUTPUT));
  }

};

Model* modelMix4Stereo = createModel<Mix4Stereo, Mix4StereoWidget>("Mix4Stereo");

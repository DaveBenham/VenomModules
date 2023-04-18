// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "math.hpp"
#include <cfloat>
#include "Filter.hpp"

#define MODULE_NAME VCAMix4

struct VCAMix4 : VenomModule {
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
  OversampleFilter_4 outUpSample[4], outDownSample[4];
  DCBlockFilter_4 dcBlockBeforeFilter[4], dcBlockAfterFilter[4];

  VCAMix4() {
    struct FixedSwitchQuantity : SwitchQuantity {
      std::string getDisplayValueString() override {
        return labels[getValue()];
      }
    };
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
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
    configSwitch<FixedSwitchQuantity>(VCAMODE_PARAM, 0.f, 2.f, 0.f, "VCA Mode", {
      "Unipolar linear - CV clamped 0-10V", "Unipolar exponential - CV clamped 0-10V", "Bipolar linear - CV unclamped"
    });
    configSwitch<FixedSwitchQuantity>(DCBLOCK_PARAM, 0.f, 3.f, 0.f, "Mix DC Block", {"Off", "Before clipping", "Before and after clipping", "After clipping"});
    configSwitch<FixedSwitchQuantity>(CLIP_PARAM, 0.f, 3.f, 0.f, "Mix Clipping", {"Off", "Hard CV clipping", "Soft audio clipping", "Soft oversampled audio clipping"});
    configSwitch<FixedSwitchQuantity>(EXCLUDE_PARAM, 0.f, 1.f, 0.f, "Exclude Patched Outs from Mix", {"Off", "On"});
    configInput(CHAIN_INPUT, "Chain");
    configOutput(MIX_OUTPUT, "Mix");
    initOversample();
    initDCBlock();
  }

  void initOversample(){
    for (int i=0; i<4; i++){
      outUpSample[i].setOversample(oversample);
      outDownSample[i].setOversample(oversample);
    }
  }

  void initDCBlock(){
    float sampleTime = settings::sampleRate;
    for (int i=0; i<4; i++){
      dcBlockBeforeFilter[i].init(sampleTime);
      dcBlockAfterFilter[i].init(sampleTime);
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

    int inChannels[4];
    for (int i=0; i<4; i++)
      inChannels[i] = mode == 1 ? 1 : std::max({1, inputs[CV_INPUTS+i].getChannels(), inputs[INPUTS+i].getChannels()});
    int channels = std::max({inChannels[0], inChannels[1], inChannels[2], inChannels[3], inputs[MIX_CV_INPUT].getChannels(), inputs[CHAIN_INPUT].getChannels()});
    simd::float_4 in, out, cv;
    for (int c=0; c<channels; c+=4){
      out = inputs[CHAIN_INPUT].getPolyVoltageSimd<simd::float_4>(c);
      for (int i=0; i<4; i++){
        cv = inputs[CV_INPUTS+i].getNormalPolyVoltageSimd<simd::float_4>(10.f, c) / 10.f;
        if (vcaMode <= 1)
          cv = simd::clamp(cv, 0.f, 1.f);
        if (vcaMode == 1)
          cv = simd::pow(cv, 4);
        in = mode == 1 ? inputs[INPUTS+i].getVoltageSum() : inputs[INPUTS+i].getNormalPolyVoltageSimd<simd::float_4>(normal, c);
        in *= (params[LEVEL_PARAMS+i].getValue()+offset)*scale*cv;
        outputs[OUTPUTS+i].setVoltageSimd(in, c);
        if (!exclude || !outputs[OUTPUTS+i].isConnected())
          out += in;
      }
      cv = inputs[MIX_CV_INPUT].getNormalPolyVoltageSimd<simd::float_4>(10.f, c) / 10.f;
      if (vcaMode <= 1)
        cv = simd::clamp(cv, 0.f, 1.f);
      if (vcaMode == 1)
        cv = simd::pow(cv, 4);
      out *= (params[MIX_LEVEL_PARAM].getValue()+offset)*scale*cv;
      if (dcBlock && dcBlock <= 2)
        out = dcBlockBeforeFilter[c/4].process(out);
      if (clip == 1)
        out = clamp(out, -10.f, 10.f);
      if (clip == 2)
        out = softClip(out);
      if (clip == 3){
        for (int i=0; i<oversample; i++){
          out = outUpSample[c/4].process(i ? simd::float_4::zero() : out*oversample);
          out = softClip(out);
          out = outDownSample[c/4].process(out);
        }
      }
      if (dcBlock == 3 || (dcBlock == 2 && clip))
        out = dcBlockAfterFilter[c/4].process(out);
      outputs[MIX_OUTPUT].setVoltageSimd(out, c);
    }
    for (int i=0; i<4; i++)
      outputs[OUTPUTS+i].setChannels(inChannels[i]);
    outputs[MIX_OUTPUT].setChannels(channels);
  }

};

struct VCAMix4Widget : VenomWidget {

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

  VCAMix4Widget(VCAMix4* module) {
    setModule(module);
    setVenomPanel("VCAMix4");

    addInput(createInputCentered<PJ301MPort>(Vec(21.329, 42.295), module, VCAMix4::CV_INPUTS+0));
    addInput(createInputCentered<PJ301MPort>(Vec(21.329, 73.035), module, VCAMix4::CV_INPUTS+1));
    addInput(createInputCentered<PJ301MPort>(Vec(21.329,103.775), module, VCAMix4::CV_INPUTS+2));
    addInput(createInputCentered<PJ301MPort>(Vec(21.329,134.515), module, VCAMix4::CV_INPUTS+3));
    addInput(createInputCentered<PJ301MPort>(Vec(21.329,168.254), module, VCAMix4::MIX_CV_INPUT));

    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.671, 42.295), module, VCAMix4::LEVEL_PARAMS+0));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.671, 73.035), module, VCAMix4::LEVEL_PARAMS+1));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.671, 103.775), module, VCAMix4::LEVEL_PARAMS+2));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.671, 134.515), module, VCAMix4::LEVEL_PARAMS+3));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(53.671,168.254), module, VCAMix4::MIX_LEVEL_PARAM));

    addParam(createLockableParamCentered<ModeSwitch>(Vec(67.9055,57.6655), module, VCAMix4::MODE_PARAM));
    addParam(createLockableParamCentered<VCAModeSwitch>(Vec(67.9055,88.4045), module, VCAMix4::VCAMODE_PARAM));
    addParam(createLockableParamCentered<DCBlockSwitch>(Vec(67.9055,119.1445), module, VCAMix4::DCBLOCK_PARAM));
    addParam(createLockableParamCentered<ClipSwitch>(Vec(67.9055,149.8845), module, VCAMix4::CLIP_PARAM));
    addParam(createLockableParamCentered<ExcludeSwitch>(Vec(7.2725,357.2475), module, VCAMix4::EXCLUDE_PARAM));

    addInput(createInputCentered<PJ301MPort>(Vec(21.329,209.400), module, VCAMix4::INPUTS+0));
    addInput(createInputCentered<PJ301MPort>(Vec(21.329,241.320), module, VCAMix4::INPUTS+1));
    addInput(createInputCentered<PJ301MPort>(Vec(21.329,273.240), module, VCAMix4::INPUTS+2));
    addInput(createInputCentered<PJ301MPort>(Vec(21.329,305.160), module, VCAMix4::INPUTS+3));
    addInput(createInputCentered<PJ301MPort>(Vec(21.329,340.434), module, VCAMix4::CHAIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(Vec(53.671,209.400), module, VCAMix4::OUTPUTS+0));
    addOutput(createOutputCentered<PJ301MPort>(Vec(53.671,241.320), module, VCAMix4::OUTPUTS+1));
    addOutput(createOutputCentered<PJ301MPort>(Vec(53.671,273.240), module, VCAMix4::OUTPUTS+2));
    addOutput(createOutputCentered<PJ301MPort>(Vec(53.671,305.160), module, VCAMix4::OUTPUTS+3));
    addOutput(createOutputCentered<PJ301MPort>(Vec(53.671,340.434), module, VCAMix4::MIX_OUTPUT));
  }

};

Model* modelVCAMix4 = createModel<VCAMix4, VCAMix4Widget>("VCAMix4");

// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"

#define CHANNEL_COUNT 10

struct PolySHASR : VenomModule {
  
  enum ParamId {
    OVER_PARAM,
    RANGE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(TRIG_INPUT, CHANNEL_COUNT),
    ENUMS(DATA_INPUT, CHANNEL_COUNT),
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(OUTPUT, CHANNEL_COUNT),
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  int oversample = -1;
  int oversampleValues[6] {1,2,4,8,16,32};
  float rangeScale[6] {1.f,5.f,10.f,2.f,10.f,20.f};
  float rangeOffset[6] {0.f,0.f,0.f,-1.f,-5.f,-10.f};
  simd::float_4 trigState[CHANNEL_COUNT][4]{}, out[CHANNEL_COUNT][4]{}, finalOut[CHANNEL_COUNT][4]{};
  int outCnt[CHANNEL_COUNT]{};
  
  OversampleFilter_4 trigUpSample[CHANNEL_COUNT][4], inUpSample[CHANNEL_COUNT][4], outDownSample[CHANNEL_COUNT][4];

  PolySHASR() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversample", {"Off", "x2", "x4", "x8", "x16", "x32"});
    configSwitch<FixedSwitchQuantity>(RANGE_PARAM, 0.f, 5.f, 2.f, "Random range", {"0-1", "0-5", "0-10", "+/- 1", "+/- 5", "+/- 10"});
    for (int i=0; i<CHANNEL_COUNT; i++){
      std::string iStr = std::to_string(i);
      configInput(TRIG_INPUT+i, "Trigger "+iStr);
      configInput(DATA_INPUT+i, "Data "+iStr);
      configOutput(OUTPUT+i, "Sample & Hold "+iStr);
    }
  }

  void process(const ProcessArgs& args) override {
    using float_4 = simd::float_4;
    VenomModule::process(args);
    if (oversample != oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())]) {
      oversample = oversampleValues[static_cast<int>(params[OVER_PARAM].getValue())];
      for (int c=0; c<CHANNEL_COUNT; c++){
        for (int pi=0; pi<4; pi++){
          trigUpSample[c][pi].setOversample(oversample);
          inUpSample[c][pi].setOversample(oversample);
          outDownSample[c][pi].setOversample(oversample);
        }
      }
    }
    int range = params[RANGE_PARAM].getValue();
    float scale = rangeScale[range], offset = rangeOffset[range];
    int trigCnt[CHANNEL_COUNT]{};
    for (int o=0; o<oversample; o++){
      float_4 trig[CHANNEL_COUNT][4]{};
      for (int c=0; c<CHANNEL_COUNT; c++){
        if (inputs[TRIG_INPUT+c].isConnected()){
          if (!o) trigCnt[c] = inputs[TRIG_INPUT+c].getChannels();
          for (int p=0, pi=0; p<trigCnt[c]; p+=4, pi++){
            float_4 trigIn{}, oldState{};
            if (!o) 
              trigIn = inputs[TRIG_INPUT+c].getPolyVoltageSimd<float_4>(p);
            if (oversample>1)
              trigIn = trigUpSample[c][pi].process(o ? float_4::zero() : trigIn * oversample);
            oldState = trigState[c][pi];
            trigState[c][pi] = simd::ifelse(trigIn>2.f, 2.f, simd::ifelse(trigIn<=0.1f, 0.f, trigState[c][pi]));
            trig[c][pi] = simd::ifelse(oldState>float_4::zero(), 0.f, simd::ifelse(trigState[c][pi]>float_4::zero(), 1.f, 0.f));
          }  
        } else if (c) {
          trigCnt[c] = trigCnt[c-1];
          for (int p=0, pi=0; p<trigCnt[c]; p+=4, pi++)
            trig[c][pi] = trig[c-1][pi];
        } else {
          trigCnt[c] = 1;
          trig[c][0] = float_4::zero();
        }
      }
      for (int c=CHANNEL_COUNT-1; c>=0; c--){
        if (!o) outCnt[c] = std::max(trigCnt[c], inputs[DATA_INPUT+c].isConnected() ? inputs[DATA_INPUT+c].getChannels() : (inputs[TRIG_INPUT+c].isConnected() || !c ? 1 : outCnt[c-1]));
        for (int p=0, pi=0; p<outCnt[c]; p+=4, pi++){
          float_4 tempTrig = trigCnt[c] == 1 ? trig[c][0] : (trigCnt[c]/4 >= pi ? trig[c][pi] : float_4::zero());
          float_4 data{};
          if (inputs[DATA_INPUT+c].isConnected()){
            data = inputs[DATA_INPUT+c].getPolyVoltageSimd<float_4>(p);
            if (oversample>1)
              data = inUpSample[c][pi].process(o ? float_4::zero() : data * oversample);
          } else if (inputs[TRIG_INPUT+c].isConnected()){
            float_4 rnd{random::uniform(),random::uniform(),random::uniform(),random::uniform()};
            data = rnd * scale + offset;
          } else {
            data = c==0 ? float_4::zero() : out[c-1][pi];
          }
          out[c][pi] = simd::ifelse(tempTrig>0.f, data, out[c][pi]);
          finalOut[c][pi] = oversample>1 ? outDownSample[c][pi].process(out[c][pi]) : out[c][pi];
        }
      }
    }
    for (int c=0; c<CHANNEL_COUNT; c++){
      for (int p=0, pi=0; p<trigCnt[c]; p+=4, pi++){
        outputs[OUTPUT+c].setVoltageSimd(finalOut[c][pi], p);
      }
      outputs[OUTPUT+c].setChannels(outCnt[c]);
    }
  }

};

struct PolySHASRWidget : VenomWidget {

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

  struct RangeSwitch : GlowingSvgSwitchLockable {
    RangeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPinkButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
    }
  };

  PolySHASRWidget(PolySHASR* module) {
    setModule(module);
    setVenomPanel("PolySHASR");
    addParam(createLockableParamCentered<OverSwitch>(Vec(36.5,34.5), module, PolySHASR::OVER_PARAM));
    addParam(createLockableParamCentered<RangeSwitch>(Vec(68.5,34.5), module, PolySHASR::RANGE_PARAM));
    float y=66.5f;
    for (int i=0; i<CHANNEL_COUNT; i++){
      addInput(createInputCentered<PolyPort>(Vec(20.5,y), module, PolySHASR::TRIG_INPUT+i));
      addInput(createInputCentered<PolyPort>(Vec(52.5,y), module, PolySHASR::DATA_INPUT+i));
      addOutput(createOutputCentered<PolyPort>(Vec(84.5,y), module, PolySHASR::OUTPUT+i));
      y+=31.f;
    }
  }

};

Model* modelPolySHASR = createModel<PolySHASR, PolySHASRWidget>("PolySHASR");

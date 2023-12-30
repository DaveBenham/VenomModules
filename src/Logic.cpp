// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "Filter.hpp"
#include "plugin.hpp"

#define CHANNEL_COUNT 9
#define LIGHT_OFF 0.02f
#define LIGHT_DIM 0.1f
#define LIGHT_FADE 5e-6f

struct Logic : VenomModule {
  enum ParamId {
    MERGE_PARAM,
    OVER_PARAM,
    RANGE_PARAM,
    DC_PARAM,
    HIGH_PARAM,
    LOW_PARAM,
    ENUMS(RECYCLE_PARAM, CHANNEL_COUNT),
    ENUMS(OP_PARAM, CHANNEL_COUNT),
    PARAMS_LEN
  };
  enum InputId {
    HIGH_INPUT,
    LOW_INPUT,
    ENUMS(A_INPUT, CHANNEL_COUNT),
    ENUMS(B_INPUT, CHANNEL_COUNT),
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(GATE_OUTPUT, CHANNEL_COUNT),
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };

  enum Ops {
    OP_NONE,
    OP_AND,
    OP_OR,
    OP_XOR_1,
    OP_XOR_ODD,
    OP_NAND,
    OP_NOR,
    OP_XNOR_1,
    OP_XNOR_ODD
  };
  
  int oversample = -1;
  std::vector<int> oversampleValues = {1,2,4,8,16,32};
  std::vector<float> highValues = {1.f, 5.f, 10.f, 1.f, 5.f, 10.f};
  std::vector<float> lowValues = {0.f, 0.f, 0.f, -1.f, -5.f, -10.f};
  OversampleFilter   highUpSample, lowUpSample;
  OversampleFilter_4 aUpSample[CHANNEL_COUNT][4], bUpSample[CHANNEL_COUNT][4], outDownSample[CHANNEL_COUNT][4];
  DCBlockFilter_4 dcBlock[CHANNEL_COUNT][4];
  simd::float_4 aState[CHANNEL_COUNT][4]{}, bState[CHANNEL_COUNT][4]{};
  
  Logic() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configSwitch<FixedSwitchQuantity>(MERGE_PARAM, 0.f, 1.f, 0.f, "Merge polyphony", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(RANGE_PARAM, 0.f, 5.f, 2.f, "Output range", {"0-1", "0-5", "0-10", "+/- 1", "+/- 5", "+/- 10"});
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 5.f, 0.f, "Oversampling", {"Off", "x2", "x4", "x8", "x16", "x32"});
    configSwitch<FixedSwitchQuantity>(DC_PARAM, 0.f, 1.f, 0.f, "DC block", {"Off", "On"});

    configParam(HIGH_PARAM, -10.f, 10.f, 2.f, "Rise threshold", "V");
    configInput(HIGH_INPUT,"Rise threshold");
    configParam(LOW_PARAM, -10.f, 10.f, 0.1f, "Fall threshold", "V");
    configInput(LOW_INPUT,"Fall threshold");

    std::vector<std::string> recycleLabels = {"None"};
    std::vector<std::string> opLabels = {
      "Defer",
      "AND",
      "OR",
      "XOR 1",
      "XOR ODD",
      "NAND",
      "NOR",
      "XNOR 1",
      "XNOR ODD"
    };
    for(int i = 0; i < CHANNEL_COUNT; i++){
      std::string i_s = std::to_string(i+1);
      configInput(A_INPUT+i, "Gate A " + i_s);
      configInput(B_INPUT+i, "Gate B " + i_s);
      configSwitch<FixedSwitchQuantity>(RECYCLE_PARAM+i, 0, i, 0, "Reuse for input " + i_s, recycleLabels);
      configSwitch<FixedSwitchQuantity>(OP_PARAM+i, 0, 8, 0, "Logic operation " + i_s, opLabels);
      configOutput(GATE_OUTPUT + i, "Gate " + i_s);
      recycleLabels.push_back("Gate " + i_s + " output");
    }
    initDCBlock();
  }

  void initDCBlock(){
    float sampleTime = settings::sampleRate;
    for (int c=0; c<CHANNEL_COUNT; c++){
      for (int i=0; i<4; i++){
        dcBlock[c][i].init(sampleTime);
      }
    }
  }

  void onSampleRateChange(const SampleRateChangeEvent& e) override {
    initDCBlock();
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    using float_4 = simd::float_4;
    using int32_4 = simd::int32_4;
    if (oversample != oversampleValues[params[OVER_PARAM].getValue()]) {
      oversample = oversampleValues[params[OVER_PARAM].getValue()];
      highUpSample.setOversample(oversample);
      lowUpSample.setOversample(oversample);
      for (int c=0; c<CHANNEL_COUNT; c++){
        for (int pi=0; pi<4; pi++){
          aUpSample[c][pi].setOversample(oversample);
          bUpSample[c][pi].setOversample(oversample);
          outDownSample[c][pi].setOversample(oversample);
        }
      }
    }

    int channelMap[CHANNEL_COUNT];
    int polyCount[CHANNEL_COUNT] {};
    int aChannels[CHANNEL_COUNT], bChannels[CHANNEL_COUNT];
    int current = -1;
    int recycle;
    int endChannel = 0;
    bool merge = params[MERGE_PARAM].getValue();
    int range = static_cast<int>(params[RANGE_PARAM].getValue());
    bool dc = params[DC_PARAM].getValue();
    for (int c=CHANNEL_COUNT-1; c>=0; c--) {
      if (params[OP_PARAM+c].getValue()){
        current = c;
        if (!endChannel) endChannel = c+1;
      }
      channelMap[c] = current;
    }
    for (int c=0; c<endChannel; c++) {
      aChannels[c] = inputs[A_INPUT+c].isConnected() ? inputs[A_INPUT+c].getChannels() : 0;
      bChannels[c] = inputs[B_INPUT+c].isConnected() ? inputs[B_INPUT+c].getChannels() : 0;
      if (merge) polyCount[c] = (channelMap[c] == c);
      else {
        recycle = params[RECYCLE_PARAM+c].getValue();
        polyCount[channelMap[c]] = std::max({polyCount[channelMap[c]], aChannels[c], bChannels[c], recycle ? polyCount[recycle-1] : 0});
      }
    }
    if (endChannel) {
      float highThresh, lowThresh;
      highThresh = params[HIGH_PARAM].getValue() + inputs[HIGH_INPUT].getVoltage();
      lowThresh = params[LOW_PARAM].getValue() + inputs[LOW_INPUT].getVoltage();
      if (highThresh < lowThresh) {
        float temp = highThresh;
        highThresh = lowThresh;
        lowThresh = temp;
      }
      float_4 outCnt[CHANNEL_COUNT]{}, outState[CHANNEL_COUNT][4]{}, outVal[CHANNEL_COUNT][4]{};
      for (int o=0; o<oversample; o++){
        if (oversample>1) {
          highThresh = highUpSample.process(o>1 ? 0.f : highThresh * oversample);
          lowThresh = lowUpSample.process(o>1 ? 0.f : lowThresh * oversample);
        }
        float_4 outSum[CHANNEL_COUNT][4]{};
        for (int c=0; c<endChannel; c++){
          if (aChannels[c]>0){
            if (o==0) outCnt[channelMap[c]] += (merge ? aChannels[c] : 1);
            for (int p=0, pi=0; p < (merge ? aChannels[c] : polyCount[channelMap[c]]); p+=4, pi++) {
              float_4 in=0.f;
              if (o==0){
                in = inputs[A_INPUT+c].getPolyVoltageSimd<float_4>(p);
              }
              if (oversample>1) {
                in = aUpSample[c][pi].process(o ? float_4::zero() : in * oversample);
              }
              aState[c][pi] = simd::ifelse(in>float_4(highThresh), 1.f, aState[c][pi]);
              aState[c][pi] = simd::ifelse(in<=float_4(lowThresh), 0.f, aState[c][pi]);
              if (merge){
                for (int i=p; i<aChannels[c]; i++){
                  outSum[channelMap[c]][0][0] += aState[c][pi][i];
                }
              } else {
                outSum[channelMap[c]][pi] += aState[c][pi];
              }
            }
          }
          if (bChannels[c]>0){
            if (o==0) outCnt[channelMap[c]] += (merge ? bChannels[c] : 1);
            for (int p=0, pi=0; p < (merge ? bChannels[c] : polyCount[channelMap[c]]); p+=4, pi++) {
              float_4 in=0.f;
              if (o==0){
                in = inputs[B_INPUT+c].getPolyVoltageSimd<float_4>(p);
              }
              if (oversample>1) {
                in = bUpSample[c][pi].process(o ? float_4::zero() : in * oversample);
              }
              bState[c][pi] = simd::ifelse(in>float_4(highThresh), 1.f, bState[c][pi]);
              bState[c][pi] = simd::ifelse(in<=float_4(lowThresh), 0.f, bState[c][pi]);
              if (merge){
                for (int i=p; i<bChannels[c]; i++){
                  outSum[channelMap[c]][0][0] += bState[c][pi][i];
                }
              } else {
                outSum[channelMap[c]][pi] += bState[c][pi];
              }
            }
          }
          if ((recycle = params[RECYCLE_PARAM+c].getValue()-1)>=0) {
            if (o==0) outCnt[channelMap[c]]++;
            if (merge) {
              outSum[channelMap[c]][0][0] += outState[recycle][0][0];
            } else {
              for (int p=0, pi=0; p<polyCount[channelMap[c]]; p+=4, pi++) {
                outSum[channelMap[c]][pi] += outState[recycle][pi];
              }
            }
          }
          if (channelMap[c] == c){
            int op = static_cast<int>(params[OP_PARAM+c].getValue());
            for (int p=0, pi=0; p<polyCount[c]; p+=4, pi++) {
              switch(op) {
                case OP_AND:
                  outState[c][pi] = simd::ifelse(outSum[c][pi]==outCnt[c], 1.f, 0.f);
                  break;
                case OP_OR:
                  outState[c][pi] = simd::ifelse(outSum[c][pi]>0.f, 1.f, 0.f);
                  break;
                case OP_XOR_1:
                  outState[c][pi] = simd::ifelse(outSum[c][pi]==1.f, 1.f, 0.f);
                  break;
                case OP_XOR_ODD:
                  outState[c][pi] = simd::ifelse((static_cast<int32_4>(outSum[c][pi])&1)==1, 1.f, 0.f);
                  break;
                case OP_NAND:
                  outState[c][pi] = simd::ifelse(outSum[c][pi]!=outCnt[c], 1.f, 0.f);
                  break;
                case OP_NOR:
                  outState[c][pi] = simd::ifelse(outSum[c][pi]==0.f, 1.f, 0.f);
                  break;
                case OP_XNOR_1:
                  outState[c][pi] = simd::ifelse(outSum[c][pi]!=1.f, 1.f, 0.f);
                  break;
                case OP_XNOR_ODD:
                  outState[c][pi] = simd::ifelse((static_cast<int32_4>(outSum[c][pi])&1)==0, 1.f, 0.f);
                  break;
              }
              outVal[c][pi] = simd::ifelse(outState[c][pi]==0, lowValues[range], highValues[range]);
              if (oversample>1) {
                outVal[c][pi] = outDownSample[c][pi].process(outVal[c][pi]);
              }
            }
          }
        }
      }
      for (int c=0; c<endChannel; c++) {
        for (int p=0, pi=0; p<polyCount[c]; p+=4, pi++) {
          if (dc) {
            outVal[c][pi] = dcBlock[c][pi].process(outVal[c][pi]);
          }  
          outputs[GATE_OUTPUT+c].setVoltageSimd(outVal[c][pi], p);
        }
        outputs[GATE_OUTPUT+c].setChannels(polyCount[c]);
      }
    }
    for (int c=endChannel; c<CHANNEL_COUNT; c++) {
      outputs[GATE_OUTPUT+c].setVoltage(0.f);
      outputs[GATE_OUTPUT+c].setChannels(0);
    }
  }

};


struct LogicWidget : VenomWidget {

  struct MergeSwitch : GlowingSvgSwitchLockable {
    MergeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
    }
  };

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

  struct DCSwitch : GlowingSvgSwitchLockable {
    DCSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
    }
  };

  struct RecycleSwitch : GlowingSvgSwitchLockable {
    RecycleSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/out_none.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/out_1.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/out_2.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/out_3.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/out_4.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/out_5.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/out_6.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/out_7.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/out_8.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/out_9.svg")));
    }
  };

  struct OpSwitch : GlowingSvgSwitchLockable {
    OpSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/op_down.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/op_AND.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/op_OR.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/op_XOR_1.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/op_XOR_ODD.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/op_NAND.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/op_NOR.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/op_XNOR_1.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/op_XNOR_ODD.svg")));
    }
  };

  LogicWidget(Logic* module) {
    setModule(module);
    setVenomPanel("Logic");

    addParam(createLockableParamCentered<MergeSwitch>(Vec(51.33,33.), module, Logic::MERGE_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(73.999,33.), module, Logic::OVER_PARAM));
    addParam(createLockableParamCentered<RangeSwitch>(Vec(96.663,33.), module, Logic::RANGE_PARAM));
    addParam(createLockableParamCentered<DCSwitch>(Vec(119.33,33.), module, Logic::DC_PARAM));
    
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(35.718,61.5), module, Logic::HIGH_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(66.265,61.5), module, Logic::HIGH_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(98.582,61.5), module, Logic::LOW_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(129.198,61.5), module, Logic::LOW_INPUT));

    float y = 101.5f;
    for(int i = 0; i < CHANNEL_COUNT; i++, y += 30.0f){
      addInput(createInputCentered<PolyPJ301MPort>(Vec(20.718,y), module, Logic::A_INPUT+i));
      addInput(createInputCentered<PolyPJ301MPort>(Vec(51.109,y), module, Logic::B_INPUT+i));
      addParam(createParamCentered<RecycleSwitch>(Vec(82.000,y), module, Logic::RECYCLE_PARAM+i));
      addParam(createParamCentered<OpSwitch>(Vec(113.391,y), module, Logic::OP_PARAM+i));
      addOutput(createOutputCentered<PolyPJ301MPort>(Vec(144.282,y), module, Logic::GATE_OUTPUT+i));
    }
  }

};


Model* modelLogic = createModel<Logic, LogicWidget>("Logic");

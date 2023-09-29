// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "math.hpp"
#include <cfloat>
#include "Filter.hpp"

#define HARD_CLIP 1
#define SOFT_CLIP 2

struct ShapedVCA : VenomModule {
  enum ParamId {
    RANGE_PARAM,
    MODE_PARAM,
    CLIP_PARAM,
    OVER_PARAM,
    OFFSET_PARAM,
    LEVEL_PARAM,
    BIAS_PARAM,
    CURVE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    LEVEL_INPUT,
    CURVE_INPUT,
    LEFT_INPUT,
    RIGHT_INPUT,
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

  bool oldLog = false;
  int range = -1; // force initialization
  float levelOffset, levelOffsetVals[6] = {0.f, 0.f, 0.f, -1.f, -2.f, -10.f};
  float levelScale, levelScaleVals[6] = {1.f, 2.f, 10.f, 2.f, 4.f, 20.f};
  float levelDefaultVals[6] = {1.f, 0.5f, 0.1f, 1.f, 0.75f, 0.55f};
  float offsetVals[3] = {0.f, -5.f, 5.f};
  float oldOversample = -1; // force initialization
  int oversample, overVals[5] = {1, 4, 8, 16, 32};
  OversampleFilter_4 levelUpSample[4], curveUpSample[4], 
                     leftUpSample[4], rightUpSample[4], 
                     leftDownSample[4], rightDownSample[4];

  ShapedVCA() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch(RANGE_PARAM, 0.f, 5.f, 0.f, "Level Range", {"0-1", "0-2", "0-10", "+/- 1", "+/- 2", "+/- 10"});
    configSwitch(MODE_PARAM, 0.f, 1.f, 0.f, "VCA Mode", {"Unipolar clipped CV (2 quadrant)", "Bipolar unclipped CV (4 quadrant)"});
    configSwitch(CLIP_PARAM, 0.f, 2.f, 0.f, "Output Clipping", {"Off", "Hard clip", "Soft clip"});
    configSwitch(OVER_PARAM, 0.f, 4.f, 0.f, "Oversample", {"Off", "x4", "x8", "x16", "x32"});
    configParam(LEVEL_PARAM, 0.f, 1.f, 1.f, "Level", "x", 0.f, 1.f, 0.f);
    configInput(LEVEL_INPUT, "Level CV");
    configParam(BIAS_PARAM, 0.f, 0.5f, 0.f, "Level CV bias", " V", 0.f, 10.f, 0.f);
    configParam<ShapeQuantity>(CURVE_PARAM, -1.f, 1.f, 0.f, "Response curve", "%", 0.f, 100.f);
    configInput(CURVE_INPUT, "Response curve");
    configInput(LEFT_INPUT, "Left");
    configInput(RIGHT_INPUT, "Right");
    configOutput(LEFT_OUTPUT, "Left");
    configOutput(RIGHT_OUTPUT, "Right");
    configSwitch(OFFSET_PARAM, 0.f, 2.f, 0.f, "Output offset", {"None", "-5 V", "+5 V"});
    configBypass(LEFT_INPUT, LEFT_OUTPUT);
    configBypass(inputs[RIGHT_INPUT].isConnected() ? RIGHT_INPUT : LEFT_INPUT, RIGHT_OUTPUT);
  }

  void onPortChange(const PortChangeEvent& e) override {
    if (e.type == Port::INPUT && e.portId == RIGHT_INPUT)
      bypassRoutes[1].inputId = e.connecting ? RIGHT_INPUT : LEFT_INPUT;
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    // get channels
    int channels = std::max({1, inputs[LEVEL_INPUT].getChannels(), inputs[CURVE_INPUT].getChannels(), inputs[LEFT_INPUT].getChannels(), inputs[RIGHT_INPUT].getChannels()});
    int simdCnt = (channels+3)/4;

    // configure oversample
    if (params[OVER_PARAM].getValue() != oldOversample) {
      oldOversample = params[OVER_PARAM].getValue();
      oversample = overVals[static_cast<int>(oldOversample)];
      for (int i=0; i<4; i++){
        levelUpSample[i].setOversample(oversample);
        curveUpSample[i].setOversample(oversample);
        leftUpSample[i].setOversample(oversample);
        rightUpSample[i].setOversample(oversample);
        leftDownSample[i].setOversample(oversample);
        rightDownSample[i].setOversample(oversample);
      }
    }
    
    // configure level
    if (static_cast<int>(params[RANGE_PARAM].getValue()) != range){
      range = static_cast<int>(params[RANGE_PARAM].getValue());
      ParamQuantity* q = paramQuantities[LEVEL_PARAM];
      q->displayMultiplier = levelScale = levelScaleVals[range];
      q->displayOffset = levelOffset = levelOffsetVals[range];
      q->defaultValue = levelDefaultVals[range];
    }  

    float level = params[LEVEL_PARAM].getValue() * levelScale + levelOffset;
    float curve = params[CURVE_PARAM].getValue();
    float bias = params[BIAS_PARAM].getValue();
    float offset = offsetVals[static_cast<int>(params[OFFSET_PARAM].getValue())];
    int clip = static_cast<int>(params[CLIP_PARAM].getValue());
    using float_4 = simd::float_4;
    float_4 leftIn[4], rightIn[4], levelIn[4], curveIn[4], gain, shape, leftOut[4], rightOut[4];
    bool leftInConnected = inputs[LEFT_INPUT].isConnected(),
         rightInConnected = inputs[RIGHT_INPUT].isConnected(),
         levelConnected = inputs[LEVEL_INPUT].isConnected(),
         curveConnected = inputs[CURVE_INPUT].isConnected(),
         leftOutConnected = outputs[LEFT_OUTPUT].isConnected(),
         rightOutConnected = outputs[RIGHT_OUTPUT].isConnected(),
         ringMod = (params[MODE_PARAM].getValue()>0.f);

    for( int o=0; o<oversample; o++){
      for( int s=0, c=0; s<simdCnt; s++, c+=4){
        curveIn[s] = curveConnected && !o ? inputs[CURVE_INPUT].getPolyVoltageSimd<float_4>(c) * oversample : float_4::zero(); // normal value is 0.f, so this simpler logic works
        levelIn[s] = levelConnected ? (o ? float_4::zero() : inputs[LEVEL_INPUT].getPolyVoltageSimd<float_4>(c)/10.f * oversample) : 1.f; // normal is non-zero, so a bit more logic needed
        leftIn[s] = leftInConnected ? (o ? float_4::zero() : inputs[LEFT_INPUT].getPolyVoltageSimd<float_4>(c) * oversample) : 10.f; // normal is non-zero, so a bit more logic needed
        if (rightInConnected) rightIn[s] = o ? float_4::zero() : inputs[RIGHT_INPUT].getPolyVoltageSimd<float_4>(c) * oversample; // normal is left, so set later if not connected
        if (oversample>1) {
          if (curveConnected) curveIn[s] = curveUpSample[s].process(curveIn[s]);
          if (levelConnected) levelIn[s] = levelUpSample[s].process(levelIn[s]);
          if (leftInConnected) leftIn[s] = leftUpSample[s].process(leftIn[s]);
          if (rightInConnected) rightIn[s] = rightUpSample[s].process(rightIn[s]);
        } 
        if (!rightInConnected) rightIn[s] = leftIn[s];
        levelIn[s] += bias;
        if (!ringMod) levelIn[s] = clamp(levelIn[s]);
        shape = clamp(curveIn[s]/10.f + curve, -1.f, 1.f);
        if (oldLog)
          gain = crossfade(levelIn[s], ifelse(shape>0.f, 11.f*levelIn[s]/(10.f*levelIn[s]+1.f), simd::pow(levelIn[s],4)), ifelse(shape>0.f, shape, -shape));
        else {
          float_4 absLevel = oldLog ? 1.f : simd::abs(levelIn[s]);
          float_4 signLevel = oldLog ? 1.f : ifelse(levelIn[s]<0.f, -1.f, 1.f);
          gain = crossfade(levelIn[s], ifelse(shape>0.f, signLevel*11.f*absLevel/(10.f*absLevel+1.f), simd::pow(levelIn[s],4)), ifelse(shape>0.f, shape, -shape));
        }
        leftOut[s] = leftIn[s] * gain * level;
        rightOut[s] = rightIn[s] * gain * level;
        if (clip == HARD_CLIP){
          leftOut[s] = clamp(leftOut[s], -10.f, 10.f);
          rightOut[s] = clamp(rightOut[s], -10.f, 10.f);
        }
        if (clip == SOFT_CLIP){
          leftOut[s] = softClip(leftOut[s]);
          rightOut[s] = softClip(rightOut[s]);
        }
        if (oversample>1) {
          if (leftOutConnected) leftOut[s] = leftDownSample[s].process(leftOut[s]);
          if (rightOutConnected) rightOut[s] = rightDownSample[s].process(rightOut[s]);
        }
      }
    }
    for (int s=0, c=0; s<simdCnt; s++, c+=4){
      outputs[LEFT_OUTPUT].setVoltageSimd(leftOut[s]+offset, c);
      outputs[RIGHT_OUTPUT].setVoltageSimd(rightOut[s]+offset, c);
    }
    outputs[LEFT_OUTPUT].setChannels(channels);
    outputs[RIGHT_OUTPUT].setChannels(channels);
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "oldLog", json_boolean(oldLog));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "oldLog")))
      oldLog = json_boolean_value(val);
    else
      oldLog = true;
  }

};

struct ShapedVCAWidget : VenomWidget {
  
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

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
    }
  };

  struct ClipSwitch : GlowingSvgSwitchLockable {
    ClipSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
    }
  };

  struct OverSwitch : GlowingSvgSwitchLockable {
    OverSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  struct OffsetSwitch : GlowingSvgSwitchLockable {
    OffsetSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };

  ShapedVCAWidget(ShapedVCA* module) {
    setModule(module);
    setVenomPanel("ShapedVCA");
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5f,73.f), module, ShapedVCA::LEVEL_PARAM));
    addParam(createLockableParamCentered<RangeSwitch>(Vec(6.727f,41.f), module, ShapedVCA::RANGE_PARAM));
    addParam(createLockableParamCentered<ModeSwitch>(Vec(17.243f,41.f), module, ShapedVCA::MODE_PARAM));
    addParam(createLockableParamCentered<ClipSwitch>(Vec(27.758f,41.f), module, ShapedVCA::CLIP_PARAM));
    addParam(createLockableParamCentered<OverSwitch>(Vec(38.274f,41.f), module, ShapedVCA::OVER_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(22.5f,106.5f), module, ShapedVCA::LEVEL_INPUT));
    addParam(createLockableParamCentered<TrimpotLockable>(Vec(22.5f,138.5f), module, ShapedVCA::BIAS_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f,172.f), module, ShapedVCA::CURVE_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(22.5f,202.5f), module, ShapedVCA::CURVE_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(22.5f,240.f), module, ShapedVCA::LEFT_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(22.5f,270.f), module, ShapedVCA::RIGHT_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(22.5f,309.f), module, ShapedVCA::LEFT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(22.5f,339.f), module, ShapedVCA::RIGHT_OUTPUT));
    addParam(createLockableParamCentered<OffsetSwitch>(Vec(37.531f,326.013f), module, ShapedVCA::OFFSET_PARAM));
  }

  void appendContextMenu(Menu* menu) override {
    ShapedVCA* module = dynamic_cast<ShapedVCA*>(this->module);

    menu->addChild(new MenuSeparator);
    menu->addChild(createBoolMenuItem("Old negative log behavior", "",
      [=]() {
        return module->oldLog;
      },
      [=](bool val) {
        module->oldLog = val;
      }
    ));    

    VenomWidget::appendContextMenu(menu);
  }

};

Model* modelShapedVCA = createModel<ShapedVCA, ShapedVCAWidget>("ShapedVCA");

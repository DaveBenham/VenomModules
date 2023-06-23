// Venom Modules (c) 2022 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "math.hpp"
#include <cfloat>

#define MODULE_NAME ShapedVCA

struct ShapedVCA : VenomModule {
  enum ParamId {
    LEVEL_PARAM,
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

  ShapedVCA() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(LEVEL_PARAM, 0.f, 1.f, 1.f, "Level");
    configInput(LEVEL_INPUT, "Level");
    configParam(CURVE_PARAM, -1.f, 1.f, 0.f, "Response curve");
    configInput(CURVE_INPUT, "Response curve");
    configInput(LEFT_INPUT, "Left");
    configInput(RIGHT_INPUT, "Right");
    configOutput(LEFT_OUTPUT, "Left");
    configOutput(RIGHT_OUTPUT, "Right");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    int channels = std::max({1, inputs[LEVEL_INPUT].getChannels(), inputs[CURVE_INPUT].getChannels(), inputs[LEFT_INPUT].getChannels(), inputs[RIGHT_INPUT].getChannels()});
    float level = params[LEVEL_PARAM].getValue();
    float curve = params[CURVE_PARAM].getValue();
    using float_4 = simd::float_4;
    float_4 gain, shape, left, right;
    for( int c=0; c<channels; c+=4){
      shape = clamp(inputs[CURVE_INPUT].getNormalPolyVoltageSimd<float_4>(0.f, c)/10.f + curve, -1.f, 1.f);
      gain = clamp(inputs[LEVEL_INPUT].getNormalPolyVoltageSimd<float_4>(10.f, c)/10.f);
      gain = crossfade(gain, ifelse(shape>0.f, 11.f*gain/(10.f*gain+1.f), simd::pow(gain,4)), ifelse(shape>0.f, shape, -shape));
      left = inputs[LEFT_INPUT].getNormalPolyVoltageSimd<float_4>(10.f, c);
      right = inputs[RIGHT_INPUT].getNormalPolyVoltageSimd<float_4>(left, c) * gain * level;
      left *= gain * level;
      outputs[LEFT_OUTPUT].setVoltageSimd(left, c);
      outputs[RIGHT_OUTPUT].setVoltageSimd(right, c);
    }  
    outputs[LEFT_OUTPUT].setChannels(channels);
    outputs[RIGHT_OUTPUT].setChannels(channels);
  }

};

struct ShapedVCAWidget : VenomWidget {

  ShapedVCAWidget(ShapedVCA* module) {
    setModule(module);
    setVenomPanel("ShapedVCA");
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5f,66.f), module, ShapedVCA::LEVEL_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(22.5f,101.5f), module, ShapedVCA::LEVEL_INPUT));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(22.5f,146.f), module, ShapedVCA::CURVE_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(22.5f,181.5f), module, ShapedVCA::CURVE_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(22.5f,225.f), module, ShapedVCA::LEFT_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(22.5f,258.f), module, ShapedVCA::RIGHT_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(22.5f,303.f), module, ShapedVCA::LEFT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(22.5f,336.f), module, ShapedVCA::RIGHT_OUTPUT));
  }

};

Model* modelShapedVCA = createModel<ShapedVCA, ShapedVCAWidget>("ShapedVCA");

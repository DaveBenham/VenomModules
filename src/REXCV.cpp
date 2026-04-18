// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#define SLIDER_COUNT 8

namespace Venom {

struct REXCV : VenomModule {

  enum ParamId {
    RANGE1_PARAM,
    OFFSET1_PARAM,
    RANGE2_PARAM,
    OFFSET2_PARAM,
    RANGE3_PARAM,
    OFFSET3_PARAM,
    DIR_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RANDOM1_INPUT,
    RANDOM2_INPUT,
    RANDOM3_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(CV1_OUTPUT,8),
    ENUMS(CV2_OUTPUT,8),
    ENUMS(CV3_OUTPUT,8),
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(LEFT_LIGHT,2),
    ENUMS(RIGHT_LIGHT,2),
    LIGHTS_LEN
  };

  REXCV() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    lights[LEFT_LIGHT].setBrightness(false);
    lights[LEFT_LIGHT+1].setBrightness(false);
    lights[RIGHT_LIGHT].setBrightness(false);
    lights[RIGHT_LIGHT+1].setBrightness(false);

    configLight(LEFT_LIGHT, "Left connection indicator");
    configSwitch<FixedSwitchQuantity>(DIR_PARAM, 0.f, 1.f, 0.f, "Parent direction",{"Left","Right"});
    configLight(RIGHT_LIGHT, "Left connection indicator");

    configParam(RANGE1_PARAM, 1.f, 10.f, 1.f, "Range1", " V");
    configParam(OFFSET1_PARAM, -10.f, 10.f, 0.f, "Offset1", " V");
    configInput(RANDOM1_INPUT, "Random1");

    configParam(RANGE2_PARAM, 1.f, 10.f, 1.f, "Range2", " V");
    configParam(OFFSET2_PARAM, -10.f, 10.f, 0.f, "Offset2", " V");
    configInput(RANDOM2_INPUT, "Random2");

    configParam(RANGE3_PARAM, 1.f, 10.f, 1.f, "Range3", " V");
    configParam(OFFSET3_PARAM, -10.f, 10.f, 0.f, "Offset3", " V");
    configInput(RANDOM3_INPUT, "Random3");

    std::string nStr[8] {"1","2","3","4","5","6","7","8"};
    for (int i=0; i<8; i++) {
      configOutput(CV1_OUTPUT+i, "CV1 "+nStr[i]);
      configOutput(CV2_OUTPUT+i, "CV2 "+nStr[i]);
      configOutput(CV3_OUTPUT+i, "CV2 "+nStr[i]);
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
  
};

struct REXCVWidget : VenomWidget {

  struct DirSwitch : GlowingSvgSwitchLockable {
    DirSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
    }
  };

  REXCVWidget(REXCV* module) {
    setModule(module);
    setVenomPanel("REXCV");

    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(6.5f, 21.5f), module, REXCV::LEFT_LIGHT));
    addParam(createLockableParamCentered<DirSwitch>(Vec(52.5f,23.f), module, REXCV::DIR_PARAM));
    addChild(createLightCentered<SmallLight<YellowRedLight<>>>(Vec(98.5f, 21.5f), module, REXCV::RIGHT_LIGHT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(20.5f, 50.f), module, REXCV::RANGE1_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(20.5f, 80.f), module, REXCV::OFFSET1_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(20.5f, 112.5f), module, REXCV::RANDOM1_INPUT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 50.f), module, REXCV::RANGE2_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(52.5f, 80.f), module, REXCV::OFFSET2_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(52.5f, 112.5f), module, REXCV::RANDOM2_INPUT));

    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(84.5f, 50.f), module, REXCV::RANGE3_PARAM));
    addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(84.5f, 80.f), module, REXCV::OFFSET3_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(84.5f, 112.5f), module, REXCV::RANDOM3_INPUT));

    for (int i=0; i<8; i++){
      addOutput(createOutputCentered<MonoPort>(Vec(20.5f, 151.5f+i*27.f), module, REXCV::CV1_OUTPUT+i));
      addOutput(createOutputCentered<MonoPort>(Vec(52.5f, 151.5f+i*27.f), module, REXCV::CV2_OUTPUT+i));
      addOutput(createOutputCentered<MonoPort>(Vec(84.5f, 151.5f+i*27.f), module, REXCV::CV3_OUTPUT+i));
    }
  }
  
  void step() override {
    VenomWidget::step();
    if (module) {
      if (module->params[REXCV::DIR_PARAM].getValue()) { // right parent
        Module *parent = module->getRightExpander().module;
        bool connected = parent && parent->model == modelVenomRhythmExplorer;
        module->lights[REXCV::RIGHT_LIGHT].setBrightness(connected);
        module->lights[REXCV::RIGHT_LIGHT+1].setBrightness(!connected);
        module->lights[REXCV::LEFT_LIGHT].setBrightness(0);
        module->lights[REXCV::LEFT_LIGHT+1].setBrightness(0);
      }
      else { // left parent
        Module *parent = module->getLeftExpander().module;
        bool connected = parent && parent->model == modelVenomRhythmExplorer;
        module->lights[REXCV::LEFT_LIGHT].setBrightness(connected);
        module->lights[REXCV::LEFT_LIGHT+1].setBrightness(!connected);
        module->lights[REXCV::RIGHT_LIGHT].setBrightness(0);
        module->lights[REXCV::RIGHT_LIGHT+1].setBrightness(0);
      }
    }
  }
  
};

}

Model* modelVenomREXCV = createModel<Venom::REXCV, Venom::REXCVWidget>("REXCV");

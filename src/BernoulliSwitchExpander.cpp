// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

#define MODULE_NAME BernoulliSwitchExpander

struct BernoulliSwitchExpander : VenomModule {
  #include "BernoulliSwitchExpander.hpp"

  BernoulliSwitchExpander() {
    venomConfig(EXP_PARAMS_LEN, EXP_INPUTS_LEN, EXP_OUTPUTS_LEN, EXP_LIGHTS_LEN);
    lights[EXPAND_LIGHT].setBrightness(false);
    configInput(MODE_CV_INPUT, "Mode CV");
    configParam(PROB_CV_PARAM, -1.f, 1.f, 1.f, "Probability CV");
    configInput(RISE_CV_INPUT, "Rise Threshold CV");
    configParam(RISE_CV_PARAM, -1.f, 1.f, 0.f, "Rise threshold CV");
    configInput(FALL_CV_INPUT, "Fall Threshold CV");
    configParam(FALL_CV_PARAM, -1.f, 1.f, 0.f, "Fall threshold CV");
    configInput(OFFSET_CV_A_INPUT, "A Offset CV");
    configParam(OFFSET_CV_A_PARAM, -1.f, 1.f, 0.f, "A Offset CV");
    configInput(OFFSET_CV_B_INPUT, "B Offset CV");
    configParam(OFFSET_CV_B_PARAM, -1.f, 1.f, 0.f, "B Offset CV");
    configInput(SCALE_CV_A_INPUT, "A Scale CV");
    configParam(SCALE_CV_A_PARAM, -1.f, 1.f, 0.f, "A scale CV");
    configInput(SCALE_CV_B_INPUT, "B Scale CV");
    configParam(SCALE_CV_B_PARAM, -1.f, 1.f, 0.f, "B scale CV");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }
  
  void onExpanderChange(const ExpanderChangeEvent& e) override {
    if (!e.side) {
      Module* left = getLeftExpander().module;
      lights[EXPAND_LIGHT].setBrightness(left && left->model == modelBernoulliSwitch);
    }  
  }  

};

struct BernoulliSwitchExpanderWidget : VenomWidget {
  BernoulliSwitchExpanderWidget(BernoulliSwitchExpander* module) {
    setModule(module);
    setVenomPanel("BernoulliSwitchExpander");

    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(10.f, 30.f), module, BernoulliSwitchExpander::EXPAND_LIGHT));
    addInput(createInputCentered<PJ301MPort>(Vec(22.f, 97.f), module, BernoulliSwitchExpander::MODE_CV_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.f, 97.f), module, BernoulliSwitchExpander::PROB_CV_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(22.f, 137.f), module, BernoulliSwitchExpander::RISE_CV_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.f, 137.f), module, BernoulliSwitchExpander::RISE_CV_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(22.f, 177.f), module, BernoulliSwitchExpander::FALL_CV_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.f, 177.f), module, BernoulliSwitchExpander::FALL_CV_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(22.f, 217.f), module, BernoulliSwitchExpander::OFFSET_CV_A_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.f, 217.f), module, BernoulliSwitchExpander::OFFSET_CV_A_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(22.f, 257.f), module, BernoulliSwitchExpander::OFFSET_CV_B_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.f, 257.f), module, BernoulliSwitchExpander::OFFSET_CV_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(22.f, 297.f), module, BernoulliSwitchExpander::SCALE_CV_A_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.f, 297.f), module, BernoulliSwitchExpander::SCALE_CV_A_PARAM));
    addInput(createInputCentered<PJ301MPort>(Vec(22.f, 337.f), module, BernoulliSwitchExpander::SCALE_CV_B_INPUT));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(53.f, 337.f), module, BernoulliSwitchExpander::SCALE_CV_B_PARAM));
  }
};

Model* modelBernoulliSwitchExpander = createModel<BernoulliSwitchExpander, BernoulliSwitchExpanderWidget>("BernoulliSwitchExpander");

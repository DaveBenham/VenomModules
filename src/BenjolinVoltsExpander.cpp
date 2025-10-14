// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "BenjolinModule.hpp"

struct BenjolinVoltsExpanderWidget : BenjolinExpanderWidget {
  int venomDelCnt = 0;

  struct BinarySwitch : GlowingSvgSwitchLockable {
    BinarySwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
    }
  };

  BenjolinVoltsExpanderWidget(BenjolinVoltsExpander* module) {
    setModule(module);
    setVenomPanel("BenjolinVoltsExpander");

    addParam(createLockableParamCentered<BinarySwitch>(Vec(35.78f,43.63f), module, BenjolinModule::VOLTS_BINARY_PARAM));
    float delta=8.5f;
    for(int i=0; i<8; i++){
      addParam(createLockableParamCentered<RoundTinyBlackKnobLockable>(Vec(22.5f, 50.f+delta), module, BenjolinModule::VOLT_PARAM+i));
      delta+=25.f;
    }
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 265.f), module, BenjolinModule::VOLTS_RANGE_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(22.5f, 299.f), module, BenjolinModule::VOLTS_OFFSET_PARAM));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f,339.5f), module, BenjolinModule::VOLTS_OUTPUT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(Vec(6.f, 33.f), module, 0));
  }
  
  void step() override {
    BenjolinVoltsExpander* thisMod = dynamic_cast<BenjolinVoltsExpander*>(this->module);
    if (thisMod && thisMod->mode != thisMod->params[BenjolinModule::VOLTS_BINARY_PARAM].getValue()){
      thisMod->mode = thisMod->params[BenjolinModule::VOLTS_BINARY_PARAM].getValue();
      for (int i=0; i<8; i++){
        thisMod->paramQuantities[BenjolinModule::VOLT_PARAM+i]->snapEnabled = thisMod->mode;
      }
    }
    BenjolinExpanderWidget::step();
  }  
};

Model* modelBenjolinVoltsExpander = createModel<BenjolinVoltsExpander, BenjolinVoltsExpanderWidget>("BenjolinVoltsExpander");

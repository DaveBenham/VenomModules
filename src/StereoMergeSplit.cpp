// Venom Modules (c) 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct StereoMergeSplit : VenomModule {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    LEFT_INPUT,
    RIGHT_INPUT,
    STEREO_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    STEREO_OUTPUT,
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    ERROR_LIGHT,
    LIGHTS_LEN
  };
   
  StereoMergeSplit() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(LEFT_INPUT, "Left polyphonic");
    configInput(RIGHT_INPUT, "Right polyphonic");
    configOutput(STEREO_OUTPUT, "Stereo polyphonic");
    configLight(ERROR_LIGHT, "Polyphony overflow indicator");
    configInput(STEREO_INPUT, "Stereo polyphonic");
    configOutput(LEFT_OUTPUT, "Left polyphonic");
    configOutput(RIGHT_OUTPUT, "Right polyphonic");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    int mergeCnt = 1;
    int cnt = inputs[LEFT_INPUT].getChannels();
    if (cnt>mergeCnt)
      mergeCnt = cnt;
    cnt = inputs[RIGHT_INPUT].getChannels();
    if (cnt>mergeCnt)
      mergeCnt = cnt;
    lights[ERROR_LIGHT].setBrightness(mergeCnt>8);
    if (mergeCnt>8)
      mergeCnt = 8;
    for (int c=0; c<mergeCnt; c++) {
      outputs[STEREO_OUTPUT].setVoltage(inputs[LEFT_INPUT].getVoltage(c), c);
      outputs[STEREO_OUTPUT].setVoltage(inputs[RIGHT_INPUT].getNormalVoltage(inputs[LEFT_INPUT].getVoltage(c), c), mergeCnt + c);
    }
    outputs[STEREO_OUTPUT].setChannels(mergeCnt + mergeCnt);
    int splitCnt = (inputs[STEREO_INPUT].getChannels()+1)/2;
    for (int c=0; c<splitCnt; c++) {
      outputs[LEFT_OUTPUT].setVoltage(inputs[STEREO_INPUT].getVoltage(c), c);
      outputs[RIGHT_OUTPUT].setVoltage(inputs[STEREO_INPUT].getVoltage(splitCnt + c), c);
    }
    outputs[LEFT_OUTPUT].setChannels(splitCnt);
    outputs[RIGHT_OUTPUT].setChannels(splitCnt);
  }                         

};

struct StereoMergeSplitWidget : VenomWidget {

  StereoMergeSplitWidget(StereoMergeSplit* module) {
    setModule(module);
    setVenomPanel("StereoMergeSplit");

    addInput(createInputCentered<PolyPort>(Vec(22.5f, 85.5f), module, StereoMergeSplit::LEFT_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 127.5f), module, StereoMergeSplit::RIGHT_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 173.5f), module, StereoMergeSplit::STEREO_OUTPUT));
    addChild(createLightCentered<SmallLight<RedLight>>(Vec(34.5f, 185.5f), module, StereoMergeSplit::ERROR_LIGHT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f, 243.5f), module, StereoMergeSplit::STEREO_INPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 288.5f), module, StereoMergeSplit::LEFT_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 330.5f), module, StereoMergeSplit::RIGHT_OUTPUT));
  }

};

}

Model* modelVenomStereoMergeSplit = createModel<Venom::StereoMergeSplit, Venom::StereoMergeSplitWidget>("StereoMergeSplit");

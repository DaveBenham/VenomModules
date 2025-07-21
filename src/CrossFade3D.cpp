// Venom Modules (c) 2023, 2024, 2025 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

struct CrossFade3D : VenomModule {
  
  enum ParamId {
    X_PARAM,
    Y_PARAM,
    Z_PARAM,
    X_AMT_PARAM,
    Y_AMT_PARAM,
    Z_AMT_PARAM,
    LEVEL_PARAM,
    MONO_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    X_INPUT,
    Y_INPUT,
    Z_INPUT,
    BLF_INPUT,
    BRF_INPUT,
    TLF_INPUT,
    TRF_INPUT,
    BLB_INPUT,
    BRB_INPUT,
    TLB_INPUT,
    TRB_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    FADE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    MONO_LIGHT,
    LIGHTS_LEN
  };
  
  float cvScale = 1.0;

  CrossFade3D() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(X_PARAM, 0.f, 1.f, 0.5f, "X (left to right) fader", "% right", 0, 100, 0);
    configParam(Y_PARAM, 0.f, 1.f, 0.5f, "Y (bottom to top) fader", "% top", 0, 100, 0);
    configParam(Z_PARAM, 0.f, 1.f, 0.5f, "Z (front to back) fader", "% back", 0, 100, 0);
    configParam(X_AMT_PARAM, -1.f, 1.f, 0.f, "X CV amount", "%", 0, 100, 0);
    configParam(Y_AMT_PARAM, -1.f, 1.f, 0.f, "Y CV amount", "%", 0, 100, 0);
    configParam(Z_AMT_PARAM, -1.f, 1.f, 0.f, "Z CV amount", "%", 0, 100, 0);
    configInput(X_INPUT, "X CV");
    configInput(Y_INPUT, "Y CV");
    configInput(Z_INPUT, "Z CV");
    configSwitch<FixedSwitchQuantity>(MONO_PARAM, 0.f, 1.f, 0.f, "Sum output polyphony to mono", {"Off", "On"});
    configParam(LEVEL_PARAM, 0.f, 1.f, 1.f, "Output level", "%", 0, 100, 0);
    configOutput(FADE_OUTPUT, "Fade");
    configInput(BLF_INPUT, "Bottom left front");
    configInput(BRF_INPUT, "Bottom right front");
    configInput(TLF_INPUT, "Top left front");
    configInput(TRF_INPUT, "Top right front");
    configInput(BLB_INPUT, "Bottom left back");
    configInput(BRB_INPUT, "Bottom right back");
    configInput(TLB_INPUT, "Top left back");
    configInput(TRB_INPUT, "Top right back");
  }


  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    using float_4 = simd::float_4;
    float_4 zero = simd::float_4::zero();
    bool mono = params[MONO_PARAM].getValue();
    bool onePolyInput = inputs[BLF_INPUT].isConnected() && !(
        inputs[TLF_INPUT].isConnected() || 
        inputs[BRF_INPUT].isConnected() || 
        inputs[TRF_INPUT].isConnected() || 
        inputs[BLB_INPUT].isConnected() || 
        inputs[TLB_INPUT].isConnected() || 
        inputs[BRB_INPUT].isConnected() || 
        inputs[TRB_INPUT].isConnected()
      );
    int channels=1;
    for (int i=0; i < (onePolyInput ? 3 : INPUTS_LEN); i++)
      channels = std::max(channels, inputs[i].getChannels());
    float level = params[LEVEL_PARAM].getValue();
    for (int c=0; c<channels; c+=4){
      float_4 right  = simd::clamp(params[X_PARAM].getValue() + inputs[X_INPUT].getNormalPolyVoltageSimd(zero, c)/10.f * params[X_AMT_PARAM].getValue() * cvScale);
      float_4 left   = 1.f - right;
      float_4 top    = simd::clamp(params[Y_PARAM].getValue() + inputs[Y_INPUT].getNormalPolyVoltageSimd(zero, c)/10.f * params[Y_AMT_PARAM].getValue() * cvScale);
      float_4 bottom = 1.f - top;
      float_4 back   = simd::clamp(params[Z_PARAM].getValue() + inputs[Z_INPUT].getNormalPolyVoltageSimd(zero, c)/10.f * params[Z_AMT_PARAM].getValue() * cvScale);
      float_4 front  = 1.f - back;
      if (onePolyInput)
        outputs[FADE_OUTPUT].setVoltageSimd((
            inputs[BLF_INPUT].getVoltage(0) * bottom * left * front
          + inputs[BLF_INPUT].getVoltage(1) * bottom * right * front
          + inputs[BLF_INPUT].getVoltage(2) * top * left * front
          + inputs[BLF_INPUT].getVoltage(3) * top * right * front
          + inputs[BLF_INPUT].getVoltage(4) * bottom * left * back
          + inputs[BLF_INPUT].getVoltage(5) * bottom * right * back
          + inputs[BLF_INPUT].getVoltage(6) * top * left * back
          + inputs[BLF_INPUT].getVoltage(7) * top * right * back
        )*level, c);
      else
        outputs[FADE_OUTPUT].setVoltageSimd((
            inputs[BLF_INPUT].getNormalPolyVoltageSimd(zero, c) * bottom * left * front
          + inputs[BRF_INPUT].getNormalPolyVoltageSimd(zero, c) * bottom * right * front
          + inputs[TLF_INPUT].getNormalPolyVoltageSimd(zero, c) * top * left * front
          + inputs[TRF_INPUT].getNormalPolyVoltageSimd(zero, c) * top * right * front
          + inputs[BLB_INPUT].getNormalPolyVoltageSimd(zero, c) * bottom * left * back
          + inputs[BRB_INPUT].getNormalPolyVoltageSimd(zero, c) * bottom * right * back
          + inputs[TLB_INPUT].getNormalPolyVoltageSimd(zero, c) * top * left * back
          + inputs[TRB_INPUT].getNormalPolyVoltageSimd(zero, c) * top * right * back
        )*level, c);
    }
    if (mono) {
      float* out = outputs[FADE_OUTPUT].getVoltages();
      for (int i=1; i<channels; i++)
        out[0] += out[i];
      outputs[FADE_OUTPUT].setChannels(1);
    }
    else
      outputs[FADE_OUTPUT].setChannels(channels);
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "cvScale", json_real(cvScale));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "cvScale"))){
      cvScale = json_real_value(val);
      float scale = cvScale * 100.f;
      paramQuantities[X_AMT_PARAM]->displayMultiplier = scale;
      paramQuantities[Y_AMT_PARAM]->displayMultiplier = scale;
      paramQuantities[Z_AMT_PARAM]->displayMultiplier = scale;
    }
  }

};

struct CrossFade3DWidget : VenomWidget {

  CrossFade3DWidget(CrossFade3D* module) {
    setModule(module);
    setVenomPanel("CrossFade3D");
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(27.f,66.f), module, CrossFade3D::X_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(67.5f,66.f), module, CrossFade3D::Y_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(108.f,66.f), module, CrossFade3D::Z_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(27.f,104.f), module, CrossFade3D::X_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(67.5f,104.f), module, CrossFade3D::Y_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(108.f,104.f), module, CrossFade3D::Z_AMT_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(27.f,139.5f), module, CrossFade3D::X_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(67.5f,139.5f), module, CrossFade3D::Y_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(108.f,139.5f), module, CrossFade3D::Z_INPUT));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(27.f,187.5f), module, CrossFade3D::MONO_PARAM, CrossFade3D::MONO_LIGHT));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(67.5f,187.5f), module, CrossFade3D::LEVEL_PARAM));
    addOutput(createOutputCentered<PolyPort>(Vec(108.f,187.5f), module, CrossFade3D::FADE_OUTPUT));
    addInput(createInputCentered<PolyPort>(Vec(19.5f,339.5f), module, CrossFade3D::BLF_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(83.5f,339.5f), module, CrossFade3D::BRF_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(19.5f,275.5f), module, CrossFade3D::TLF_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(83.5f,275.5f), module, CrossFade3D::TRF_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(51.5f,307.5f), module, CrossFade3D::BLB_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(115.5f,307.5f), module, CrossFade3D::BRB_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(51.5f,243.5f), module, CrossFade3D::TLB_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(115.5f,243.5f), module, CrossFade3D::TRB_INPUT));
  }

  void appendContextMenu(Menu* menu) override {
    CrossFade3D* module = static_cast<CrossFade3D*>(this->module);
    menu->addChild(new MenuSeparator);
    menu->addChild(createIndexSubmenuItem(
      "CV amount scale",
      {"-100% to 100%","-200% to 200%"},
      [=]() {return static_cast<int>(module->cvScale) - 1;},
      [=](int i) {
        module->cvScale = static_cast<float>(i+1);
        float scale = module->cvScale * 100.f;
        module->paramQuantities[CrossFade3D::X_AMT_PARAM]->displayMultiplier = scale;
        module->paramQuantities[CrossFade3D::Y_AMT_PARAM]->displayMultiplier = scale;
        module->paramQuantities[CrossFade3D::Z_AMT_PARAM]->displayMultiplier = scale;
      }
    ));
    VenomWidget::appendContextMenu(menu);
  }

  void step() override {
    VenomWidget::step();
    CrossFade3D* mod = dynamic_cast<CrossFade3D*>(this->module);
    if(mod) {
      mod->lights[CrossFade3D::MONO_LIGHT].setBrightness(mod->params[CrossFade3D::MONO_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }
  }
};

Model* modelCrossFade3D = createModel<CrossFade3D, CrossFade3DWidget>("CrossFade3D");

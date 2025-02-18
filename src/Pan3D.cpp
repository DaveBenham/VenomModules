// Venom Modules (c) 2023, 2024, 2025 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include "Filter.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

struct Pan3D : VenomModule {
  
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
    PAN_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    BLF_OUTPUT,
    BRF_OUTPUT,
    TLF_OUTPUT,
    TRF_OUTPUT,
    BLB_OUTPUT,
    BRB_OUTPUT,
    TLB_OUTPUT,
    TRB_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    MONO_LIGHT,
    LIGHTS_LEN
  };

  Pan3D() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(X_PARAM, 0.f, 1.f, 0.5f, "X (left to right) panner", "% right", 0, 100, 0);
    configParam(Y_PARAM, 0.f, 1.f, 0.5f, "Y (bottom to top) panner", "% top", 0, 100, 0);
    configParam(Z_PARAM, 0.f, 1.f, 0.5f, "Z (front to back) panner", "% back", 0, 100, 0);
    configParam(X_AMT_PARAM, -1.f, 1.f, 0.f, "X CV amount", "%", 0, 100, 0);
    configParam(Y_AMT_PARAM, -1.f, 1.f, 0.f, "Y CV amount", "%", 0, 100, 0);
    configParam(Z_AMT_PARAM, -1.f, 1.f, 0.f, "Z CV amount", "%", 0, 100, 0);
    configInput(X_INPUT, "X CV");
    configInput(Y_INPUT, "Y CV");
    configInput(Z_INPUT, "Z CV");
    configInput(PAN_INPUT, "Pan");
    configParam(LEVEL_PARAM, 0.f, 1.f, 1.f, "Output level", "%", 0, 100, 0);
    configSwitch<FixedSwitchQuantity>(MONO_PARAM, 0.f, 1.f, 0.f, "Sum output polyphony to mono", {"Off", "On"});
    configOutput(BLF_OUTPUT, "Bottom left front");
    configOutput(BRF_OUTPUT, "Bottom right front");
    configOutput(TLF_OUTPUT, "Top left front");
    configOutput(TRF_OUTPUT, "Top right front");
    configOutput(BLB_OUTPUT, "Bottom left back");
    configOutput(BRB_OUTPUT, "Bottom right back");
    configOutput(TLB_OUTPUT, "Top left back");
    configOutput(TRB_OUTPUT, "Top right back");
  }


  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    using float_4 = simd::float_4;
    float_4 zero = simd::float_4::zero();
    int channels=1;
    for (int i=0; i<INPUTS_LEN; i++)
      channels = std::max(channels, inputs[i].getChannels());
    bool mono = params[MONO_PARAM].getValue();
    float level = params[LEVEL_PARAM].getValue();
    for (int c=0; c<channels; c+=4){
      float_4 right  = simd::clamp(params[X_PARAM].getValue() + inputs[X_INPUT].getNormalPolyVoltageSimd(zero, c)/10.f * params[X_AMT_PARAM].getValue());
      float_4 left   = 1.f - right;
      float_4 top    = simd::clamp(params[Y_PARAM].getValue() + inputs[Y_INPUT].getNormalPolyVoltageSimd(zero, c)/10.f * params[Y_AMT_PARAM].getValue());
      float_4 bottom = 1.f - top;
      float_4 back   = simd::clamp(params[Z_PARAM].getValue() + inputs[Z_INPUT].getNormalPolyVoltageSimd(zero, c)/10.f * params[Z_AMT_PARAM].getValue());
      float_4 front  = 1.f - back;
      float_4 in = inputs[PAN_INPUT].getNormalPolyVoltageSimd(zero, c) * level;
      outputs[BLF_OUTPUT].setVoltageSimd(in * bottom * left * front, c);
      outputs[BRF_OUTPUT].setVoltageSimd(in * bottom * right * front, c);
      outputs[TLF_OUTPUT].setVoltageSimd(in * top * left * front, c);
      outputs[TRF_OUTPUT].setVoltageSimd(in * top * right * front, c);
      outputs[BLB_OUTPUT].setVoltageSimd(in * bottom * left * back, c);
      outputs[BRB_OUTPUT].setVoltageSimd(in * bottom * right * back, c);
      outputs[TLB_OUTPUT].setVoltageSimd(in * top * left * back, c);
      outputs[TRB_OUTPUT].setVoltageSimd(in * top * right * back, c);
    }
    for (int i=0; i<8; i++){
      if (mono) {
        float* out = outputs[i].getVoltages();
        for (int c=1; c<channels; c++)
          out[0] += out[i];
        outputs[i].setChannels(1);
      }
      else
        outputs[i].setChannels(channels);
    }
  }

};

struct Pan3DWidget : VenomWidget {

  Pan3DWidget(Pan3D* module) {
    setModule(module);
    setVenomPanel("Pan3D");
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(27.f,66.f), module, Pan3D::X_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(67.5f,66.f), module, Pan3D::Y_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(108.f,66.f), module, Pan3D::Z_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(27.f,104.f), module, Pan3D::X_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(67.5f,104.f), module, Pan3D::Y_AMT_PARAM));
    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(108.f,104.f), module, Pan3D::Z_AMT_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(27.f,139.5f), module, Pan3D::X_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(67.5f,139.5f), module, Pan3D::Y_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(108.f,139.5f), module, Pan3D::Z_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(27.f,187.5f), module, Pan3D::PAN_INPUT));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(67.5f,187.5f), module, Pan3D::LEVEL_PARAM));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(108.f,187.5f), module, Pan3D::MONO_PARAM, Pan3D::MONO_LIGHT));
    addOutput(createOutputCentered<PolyPort>(Vec(19.5f,339.5f), module, Pan3D::BLF_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(83.5f,339.5f), module, Pan3D::BRF_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(19.5f,275.5f), module, Pan3D::TLF_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(83.5f,275.5f), module, Pan3D::TRF_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(51.5f,307.5f), module, Pan3D::BLB_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(115.5f,307.5f), module, Pan3D::BRB_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(51.5f,243.5f), module, Pan3D::TLB_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(115.5f,243.5f), module, Pan3D::TRB_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    Pan3D* mod = dynamic_cast<Pan3D*>(this->module);
    if(mod) {
      mod->lights[Pan3D::MONO_LIGHT].setBrightness(mod->params[Pan3D::MONO_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }
  }
};

Model* modelPan3D = createModel<Pan3D, Pan3DWidget>("Pan3D");

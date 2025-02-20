// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

struct SphereToXYZ : VenomModule {

  enum ParamId {
    SCALE_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    RHO_INPUT,
    THETA_INPUT,
    PHI_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    X_OUTPUT,
    Y_OUTPUT,
    Z_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    LIGHTS_LEN
  };
  
  float radiansPerVolt = 3.14159265f / 5.f;
  float sqrt3 = 1.732050808f;

  SphereToXYZ() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configInput(RHO_INPUT, "Radial distance r");
    configInput(THETA_INPUT, "Polar angle \u03B8 (5V=180\u00B0)");
    configInput(PHI_INPUT, "Aximuthul angle \u03C6 (5V=180\u00B0)");
    configSwitch<FixedSwitchQuantity>(SCALE_PARAM, 0, 1, 0, "Scale", {"1:1", "\u221A3:1"});
    configOutput(X_OUTPUT, "X");
    configOutput(Y_OUTPUT, "Y");
    configOutput(Z_OUTPUT, "Z");
  }
  
  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    using float_4 = simd::float_4;
    int channels = std::max({1, inputs[RHO_INPUT].getChannels(), inputs[THETA_INPUT].getChannels(), inputs[PHI_INPUT].getChannels()});
    float scale = params[SCALE_PARAM].getValue() ? sqrt3 : 1.f;
    for (int c=0; c<channels; c+=4) {
      float_4 rho = inputs[RHO_INPUT].getPolyVoltageSimd<float_4>(c) * scale;
      float_4 theta = inputs[THETA_INPUT].getPolyVoltageSimd<float_4>(c) * radiansPerVolt;
      float_4 phi = inputs[PHI_INPUT].getPolyVoltageSimd<float_4>(c) * radiansPerVolt;
      float_4 sinTheta = sin(theta);
      outputs[X_OUTPUT].setVoltageSimd(rho * sinTheta * cos(phi), c);
      outputs[Y_OUTPUT].setVoltageSimd(rho * sinTheta * sin(phi), c);
      outputs[Z_OUTPUT].setVoltageSimd(rho * cos(theta), c);
    }
    outputs[X_OUTPUT].setChannels(channels);
    outputs[Y_OUTPUT].setChannels(channels);
    outputs[Z_OUTPUT].setChannels(channels);
  }

};

struct SphereToXYZWidget : VenomWidget {

  SphereToXYZWidget(SphereToXYZ* module) {
    setModule(module);
    setVenomPanel("SphereToXYZ");
    addInput(createInputCentered<PolyPort>(Vec(22.5f,59.5f), module, SphereToXYZ::RHO_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f,99.5f), module, SphereToXYZ::THETA_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f,139.5f), module, SphereToXYZ::PHI_INPUT));
    addParam(createLockableParam<CKSSLockable>(Vec(21.f, 178.f), module, SphereToXYZ::SCALE_PARAM));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f,247.5f), module, SphereToXYZ::X_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f,287.5f), module, SphereToXYZ::Y_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f,327.5f), module, SphereToXYZ::Z_OUTPUT));
  }
  
};

Model* modelSphereToXYZ = createModel<SphereToXYZ, SphereToXYZWidget>("SphereToXYZ");

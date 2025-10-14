// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"
#include "Filter.hpp"

struct SphereToXYZ : VenomModule {

  enum ParamId {
    SCALE_PARAM,
    OVER_PARAM,
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
  int oversample = 0;
  std::vector<std::string> oversampleLabels = {"Off","x2","x4","x8","x16","x32"};
  std::vector<int> oversampleValues = {1,2,4,8,16,32};
  OversampleFilter_4 upSample[3][4],
                     downSample[3][4];

  SphereToXYZ() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch<FixedSwitchQuantity>(OVER_PARAM, 0.f, 2.f, 0.f, "Oversample", {"Off", "x2", "x4"});
    configInput(RHO_INPUT, "Radial distance r");
    configInput(THETA_INPUT, "Polar angle \u03B8 (5V=180\u00B0)");
    configInput(PHI_INPUT, "Azimuthal angle \u03C6 (5V=180\u00B0)");
    configSwitch<FixedSwitchQuantity>(SCALE_PARAM, 0, 1, 0, "Scale", {"1:1", "\u221A3:1"});
    configOutput(X_OUTPUT, "X");
    configOutput(Y_OUTPUT, "Y");
    configOutput(Z_OUTPUT, "Z");
    oversampleStages = 5;
  }
  
  void setOversample() override {
    for (int i=0; i<3; i++){
      for (int c=0; c<4; c++){
        upSample[i][c].setOversample(oversample, oversampleStages);
        downSample[i][c].setOversample(oversample, oversampleStages);
      }
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (oversample != oversampleValues[params[OVER_PARAM].getValue()]) {
      oversample = oversampleValues[params[OVER_PARAM].getValue()];
      setOversample();
    }
    using float_4 = simd::float_4;
    int channels = std::max({1, inputs[RHO_INPUT].getChannels(), inputs[THETA_INPUT].getChannels(), inputs[PHI_INPUT].getChannels()});
    float scale = params[SCALE_PARAM].getValue() ? sqrt3 : 1.f;
    for (int c=0; c<channels; c+=4) {
      float_4 rho = inputs[RHO_INPUT].getPolyVoltageSimd<float_4>(c) * scale;
      float_4 theta = inputs[THETA_INPUT].getPolyVoltageSimd<float_4>(c) * radiansPerVolt;
      float_4 phi = inputs[PHI_INPUT].getPolyVoltageSimd<float_4>(c) * radiansPerVolt;
      float_4 sinTheta{};
      float_4 out[3]{};
      for (int o=0; o<oversample; o++){
        if (oversample > 1) {
          rho = upSample[RHO_INPUT][c/4].process(o ? float_4::zero() : rho*oversample);            
          theta = upSample[THETA_INPUT][c/4].process(o ? float_4::zero() : theta*oversample);            
          phi = upSample[PHI_INPUT][c/4].process(o ? float_4::zero() : phi*oversample);            
        }
        sinTheta = sin(theta);
        out[X_OUTPUT] = rho * sinTheta * cos(phi);
        out[Y_OUTPUT] = rho * sinTheta * sin(phi);
        out[Z_OUTPUT] = rho * cos(theta);
        if (oversample > 1) {
          for (int i=0; i<3; i++)
            out[i] = downSample[i][c/4].process(out[i]);
        }
      }
      for (int i=0; i<3; i++)
        outputs[i].setVoltageSimd(out[i], c);
    }
    for (int i=0; i<3; i++)
      outputs[i].setChannels(channels);
  }

};

struct SphereToXYZWidget : VenomWidget {

  struct OverSwitch : GlowingSvgSwitchLockable {
    OverSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };

  SphereToXYZWidget(SphereToXYZ* module) {
    setModule(module);
    setVenomPanel("SphereToXYZ");
    addParam(createLockableParam<OverSwitch>(Vec(18.720f,52.671f), module, SphereToXYZ::OVER_PARAM));
    addInput(createInputCentered<PolyPort>(Vec(22.5f,91.5f), module, SphereToXYZ::RHO_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f,131.5f), module, SphereToXYZ::THETA_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f,171.5f), module, SphereToXYZ::PHI_INPUT));
    addParam(createLockableParam<CKSSLockable>(Vec(21.f, 202.f), module, SphereToXYZ::SCALE_PARAM));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f,257.5f), module, SphereToXYZ::X_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f,297.5f), module, SphereToXYZ::Y_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f,337.5f), module, SphereToXYZ::Z_OUTPUT));
  }
  
};

Model* modelVenomSphereToXYZ = createModel<SphereToXYZ, SphereToXYZWidget>("SphereToXYZ");

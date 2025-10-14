// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

struct Thru : VenomModule {

  enum ExpParamId {
    EXP_PARAMS_LEN
  };
  enum ExpInputId {
    ENUMS(POLY_INPUT,5),
    EXP_INPUTS_LEN
  };
  enum ExpOutputId {
    ENUMS(POLY_OUTPUT,5),
    EXP_OUTPUTS_LEN
  };
  enum ExpLightId {
    EXP_LIGHTS_LEN
  };
  
  float out[4][16]{};

  Thru() {
    venomConfig(EXP_PARAMS_LEN, EXP_INPUTS_LEN, EXP_OUTPUTS_LEN, EXP_LIGHTS_LEN);
    for (int i=0; i<5; i++) {
      std::string label = string::f("Poly %d", i + 1);
      configInput(i, label);
      configOutput(i, label);
      configBypass(i, i);
      outputExtensions[i].portNameLink = i;
      inputExtensions[i].portNameLink = i;
    }
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    for (int i=0; i<16; i++) {
      outputs[4].setVoltage( inputs[4].getNormalVoltage(out[3][i], i), i);
      outputs[3].setVoltage( (out[3][i] = inputs[3].getNormalVoltage(out[2][i], i)), i);
      outputs[2].setVoltage( (out[2][i] = inputs[2].getNormalVoltage(out[1][i], i)), i);
      outputs[1].setVoltage( (out[1][i] = inputs[1].getNormalVoltage(out[0][i], i)), i);
      outputs[0].setVoltage( (out[0][i] = inputs[0].getVoltage(i)), i);
    }
    for (int i=0, cnt=0; i<5; i++) {
      if (inputs[i].isConnected()) cnt = inputs[i].getChannels();
      outputs[i].setChannels(cnt);
    }
  }

};

struct ThruWidget : VenomWidget {

  ThruWidget(Thru* module) {
    setModule(module);
    setVenomPanel("Thru");
    float delta=68.f;
    for(int i=0; i<5; i++){
      addInput(createInputCentered<PolyPort>(Vec(22.5f,40.5f+i*delta), module, Thru::POLY_INPUT+i));
      addOutput(createOutputCentered<PolyPort>(Vec(22.5f,67.5f+i*delta), module, Thru::POLY_OUTPUT+i));
    }  
  }
  
};

Model* modelVenomThru = createModel<Thru, ThruWidget>("Thru");

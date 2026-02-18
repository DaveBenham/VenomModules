// Venom Modules (c) 2023, 2024, 2025, 2026 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

namespace Venom {

struct Merge4x2 : VenomModule {
  enum ParamId {
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(POLY_INPUT,8),
    INPUTS_LEN
  };
  enum OutputId {
    ENUMS(POLY_OUTPUT,2),
    OUTPUTS_LEN
  };
  enum LightId {
    ENUMS(DROP_LIGHT,8),
    LIGHTS_LEN
  };
  
  Merge4x2() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    for (int i=0; i<8; i++) {
      std::string name = "Port " + std::to_string(i+1);
      configInput(POLY_INPUT+i, name);
      configLight(DROP_LIGHT+i, name+" dropped channel(s) indicator");
    }  
    configOutput(0, "Port 1");
    configOutput(1, "Port 2");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    for (int i=0; i<8; i++)
      lights[i].setBrightness(0.f);
    if (outputs[0].isConnected()){
      int maxPort=3;
      while (maxPort && !inputs[maxPort].isConnected())
        maxPort--;
      int outChannels = 0,
          maxChannels = 16;
      for (int i=0; i<=maxPort; i++) {
        int cnt = std::max(inputs[i].getChannels(),1);
        if (cnt>maxChannels){
          cnt=maxChannels;
          lights[i].setBrightness(1.f);
        }
        for (int c=0; c<cnt; c++)
          outputs[0].setVoltage(inputs[i].getVoltage(c), outChannels++);
        maxChannels -= cnt;
      }
      outputs[0].setChannels(outChannels);
      
      if (outputs[1].isConnected()){
        maxPort=7;
        while(maxPort>4 && !inputs[maxPort].isConnected())
          maxPort--;
        outChannels = 0,
        maxChannels = 16;
        for (int i=4; i<=maxPort; i++) {
          int cnt = std::max(inputs[i].getChannels(),1);
          if (cnt>maxChannels){
            cnt=maxChannels;
            lights[i].setBrightness(1.f);
          }
          for (int c=0; c<cnt; c++)
            outputs[1].setVoltage(inputs[i].getVoltage(c), outChannels++);
          maxChannels -= cnt;
        }
        outputs[1].setChannels(outChannels);
      }
    }
    else if (outputs[1].isConnected()){
      int maxPort=7;
      while (maxPort && !inputs[maxPort].isConnected())
        maxPort--;
      int outChannels = 0,
          maxChannels = 16;
      for (int i=0; i<=maxPort; i++) {
        int cnt = std::max(inputs[i].getChannels(),1);
        if (cnt>maxChannels){
          cnt=maxChannels;
          lights[i].setBrightness(1.f);
        }
        for (int c=0; c<cnt; c++)
          outputs[1].setVoltage(inputs[i].getVoltage(c), outChannels++);
        maxChannels -= cnt;
      }
      outputs[1].setChannels(outChannels);
    }
  }

};

struct Merge4x2Widget : VenomWidget {
  
  Merge4x2Widget(Merge4x2* module) {
    setModule(module);
    setVenomPanel("Merge4x2");

    for (int i=0; i<4; i++){
      addInput(createInputCentered<PolyPort>(Vec(22.5f, 51.5f+28.f*i), module, i));
      addInput(createInputCentered<PolyPort>(Vec(22.5f, 219.5f+28.f*i), module, 4+i));
      addChild(createLightCentered<SmallLight<RedLight>>(Vec(35.f, 41.f+28.f*i), module, i));
      addChild(createLightCentered<SmallLight<RedLight>>(Vec(35.f, 209.f+28.f*i), module, 4+i));
    }
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 171.5), module, 0));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f, 339.5), module, 1));
  }

};

}

Model* modelVenomMerge4x2 = createModel<Venom::Merge4x2, Venom::Merge4x2Widget>("Merge4x2");

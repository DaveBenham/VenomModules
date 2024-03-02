// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

struct CloneModule : VenomModule {

  #define EXPANDER_PORTS 4
  enum ExpParamId {
    EXP_PARAMS_LEN
  };
  enum ExpInputId {
    ENUMS(EXP_POLY_INPUT,4),
    EXP_INPUTS_LEN
  };
  enum ExpOutputId {
    ENUMS(EXP_POLY_OUTPUT,4),
    EXP_OUTPUTS_LEN
  };
  enum ExpLightId {
    EXP_CONNECT_LIGHT,
    ENUMS(EXP_POLY_LIGHT,8),
    EXP_LIGHTS_LEN
  };

};

struct CloneModuleBase : CloneModule {
  
  void processExpander(int clones, int goodCh){
    int expanderChannels[EXPANDER_PORTS]{};
    Module* expander = getRightExpander().module;
    expander = expander
             && !expander->isBypassed() 
             && expander->model == modelAuxClone 
              ? expander 
              : NULL;
    if (!expander) return;
    for (int i=0; i<EXPANDER_PORTS; i++){
      if (expander->outputs[EXP_POLY_OUTPUT+i].isConnected())
        expanderChannels[i] = std::max(expander->inputs[EXP_POLY_INPUT+i].getChannels(),1);
    }
    for (int c=0; c<goodCh; c++) {
      for (int e=0; e<EXPANDER_PORTS; e++){
        float val = expander->inputs[EXP_POLY_INPUT+e].getPolyVoltage(c);
        for (int j=0, ec=c*clones; j<clones; j++, ec++)
          expander->outputs[EXP_POLY_OUTPUT+e].setVoltage(val, ec);
      }
    }
    int outCnt = clones * goodCh;
    for (int e=0; e<EXPANDER_PORTS; e++){
      expander->outputs[EXP_POLY_OUTPUT+e].setChannels( expanderChannels[e] ? outCnt : 0 );
    }
  }

  void setExpanderLights(int goodCh) {
    Module* expander = getRightExpander().module;
    expander = expander
             && !expander->isBypassed() 
             && expander->model == modelAuxClone 
              ? expander 
              : NULL;
    if (!expander) return;
    for (int e=0; e<EXPANDER_PORTS; e++){
      int channels = expander->outputs[EXP_POLY_OUTPUT+e].isConnected() ? std::max(expander->inputs[EXP_POLY_INPUT+e].getChannels(),1) : 0;
      expander->lights[EXP_POLY_LIGHT+e*2].setBrightness(
        channels>goodCh ? 0.f : (channels==goodCh || channels==1 ? 1.f : (channels ? 0.2f : 0.f))
      );
      expander->lights[EXP_POLY_LIGHT+e*2+1].setBrightness(
        channels>goodCh ? 1.f : (channels==goodCh || channels==1 ? 0.f : (channels ? 1.0f : 0.f))
      );
    }
  }
  
  void onBypass(const BypassEvent& e) override {
    Module* expander = getRightExpander().module;
    expander = expander
             && expander->model == modelAuxClone 
              ? expander 
              : NULL;
    if (expander){
      for (int i=0; i<EXPANDER_PORTS; i++){
        expander->outputs[EXP_POLY_OUTPUT+i].setVoltage(0.f);
        expander->outputs[EXP_POLY_OUTPUT+i].setChannels(0);
        expander->lights[EXP_POLY_LIGHT+i*2].setBrightness(0.f);
        expander->lights[EXP_POLY_LIGHT+i*2+1].setBrightness(0.f);
      }
    }
  }
  
};

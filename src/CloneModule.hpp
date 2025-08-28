// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

struct CloneModule : VenomModule {

  #define EXPANDER_PORTS 4
  enum ExpParamId {
    ENUMS(EXP_GROUP_PARAM,4),
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
    Module* expander = getRightExpander().module;
    expander = expander
             && !expander->isBypassed() 
             && expander->model == modelAuxClone 
              ? expander 
              : NULL;
    if (!expander) return;
    int outCnt = clones * goodCh;
    for (int p=0; p<EXPANDER_PORTS; p++){
      if (expander->params[EXP_GROUP_PARAM+p].getValue()) { // group by input set
        for (int c=0, o=0; c<clones; c++){
          for (int i=0; i<goodCh; i++, o++){
            float val = expander->inputs[EXP_POLY_INPUT+p].getPolyVoltage(i);
            expander->outputs[EXP_POLY_OUTPUT+p].setVoltage(val, o);
          }
        }
      }
      else { // group by individual input channel
        for (int i=0, o=0; i<goodCh; i++){
          float val = expander->inputs[EXP_POLY_INPUT+p].getPolyVoltage(i);
          for (int c=0; c<clones; c++, o++)
            expander->outputs[EXP_POLY_OUTPUT+p].setVoltage(val, o);
        }
      }
      expander->outputs[EXP_POLY_OUTPUT+p].setChannels(outCnt);
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

struct CloneModuleWidget : VenomWidget {

  struct GroupSwitch : GlowingSvgSwitchLockable {
    GroupSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  void appendContextMenu(Menu* menu) override {
    menu->addChild(new MenuSeparator());
    Module* expander = module->rightExpander.module;
    if (expander && expander->model == modelAuxClone)
      menu->addChild(createMenuLabel("Auxilliary Clone expander connected"));
    else
      menu->addChild(createMenuItem("Add Auxilliary Clone expander", "", [this](){addExpander(modelAuxClone,this);}));
    VenomWidget::appendContextMenu(menu);
  }
};

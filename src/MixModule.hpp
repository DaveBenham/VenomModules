struct MixModule : VenomModule {
  
  enum MixTypeId {
    MIX4_TYPE,
    MIX4ST_TYPE,
    VCAMIX4_TYPE,
    VCAMIX4ST_TYPE,
    MIXMUTE_TYPE,
    MIXOFFSET_TYPE,
    MIXPAN_TYPE,
    MIXSEND_TYPE,
    MIXSOLO_TYPE
  };
  
  int mixType=-1;
  bool softMute = true;
  bool toggleMute = false;

  enum ExpLightId {
    EXP_LIGHT,
    EXP_LIGHTS_LEN
  };
  
  enum MuteParamId {
    ENUMS(MUTE_PARAM,4),
    MUTE_MIX_PARAM,
    MUTE_PARAMS_LEN
  };
  enum MuteInputId {
    ENUMS(MUTE_INPUT,4),
    MUTE_MIX_INPUT,
    MUTE_INPUTS_LEN
  };
  enum MuteOutputId {
    MUTE_OUTPUTS_LEN
  };
  enum MuteLightId {
    MUTE_EXP_LIGHT,
    ENUMS(MUTE_LIGHT,4),
    MUTE_MIX_LIGHT,
    MUTE_LIGHTS_LEN
  };
  
  enum OffsetParamId {
    ENUMS(PRE_OFFSET_PARAM,4),
    PRE_MIX_OFFSET_PARAM,
    ENUMS(POST_OFFSET_PARAM,4),
    POST_MIX_OFFSET_PARAM,
    OFFSET_PARAMS_LEN
  };
  enum OffsetInputId {
    OFFSET_INPUTS_LEN
  };
  enum OffsetOutputId {
    OFFSET_OUTPUTS_LEN
  };
  
  enum PanParamId {
    ENUMS(PAN_PARAM,4),
    ENUMS(PAN_CV_PARAM,4),
    PAN_PARAMS_LEN
  };
  enum PanInputId {
    ENUMS(PAN_INPUT,4),
    PAN_INPUTS_LEN
  };
  enum PanOutputId {
    PAN_OUTPUTS_LEN
  };
  
  enum SendParamId {
    ENUMS(SEND_PARAM,4),
    RETURN_PARAM,
    SEND_MUTE_PARAM,
    SEND_PARAMS_LEN
  };
  enum SendInputId {
    LEFT_RETURN_INPUT,
    RIGHT_RETURN_INPUT,
    SEND_INPUTS_LEN
  };
  enum SendOutputId {
    LEFT_SEND_OUTPUT,
    RIGHT_SEND_OUTPUT,
    SEND_OUTPUTS_LEN
  };
  enum SendLightId {
    RETURN_EXP_LIGHT,
    RETURN_MUTE_LIGHT,
    SEND_LIGHTS_LEN
  };
  
  enum SoloParamId {
    ENUMS(SOLO_PARAM,4),
    SOLO_PARAMS_LEN
  };
  enum SoloInputId {
    SOLO_INPUTS_LEN
  };
  enum SoloOutputId {
    SOLO_OUTPUTS_LEN
  };
  enum SoloLightId {
    SOLO_EXP_LIGHT,
    ENUMS(SOLO_LIGHT,4),
    SOLO_LIGHTS_LEN
  };
  
  MixModule* leftExpander = NULL;
  MixModule* rightExpander = NULL;
  dsp::SchmittTrigger muteCV[5];
  dsp::SlewLimiter fade[5];
  
  MixModule() {
    fade[0].rise = fade[0].fall 
                 = fade[1].rise = fade[1].fall 
                 = fade[2].rise = fade[2].fall 
                 = fade[3].rise = fade[3].fall
                 = fade[4].rise = fade[4].fall
                 = 40.f;
                 
  }  
  
  void onExpanderChange(const ExpanderChangeEvent& e) override {
    if (e.side)
      rightExpander = dynamic_cast<MixModule*>(getRightExpander().module);
    else
      leftExpander = dynamic_cast<MixModule*>(getLeftExpander().module);
  }

};

struct MixExpanderModule : MixModule {
};  

struct MixBaseModule : MixModule {
  bool stereo = false;

  bool mutePresent = false;
  bool offsetPresent = false;
  bool panPresent = false;
  bool sendPresent = true;
  bool soloPresent = false;
  MixModule* offsetExpander = NULL;
  MixModule* muteSoloExpander = NULL;
  MixModule* muteExpander = NULL;
  std::vector<MixModule*> expanders;

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    mutePresent = false;
    offsetPresent = false;
    panPresent = false;
    sendPresent = false;
    soloPresent = false;
    // Clear expanders
    offsetExpander = NULL;
    muteSoloExpander = NULL;
    expanders.clear();
    // Load expanders
    for (MixModule* mod = rightExpander; mod; mod = mod->rightExpander) {
      if (mod->mixType == MIXMUTE_TYPE && !mutePresent) {
        mutePresent = true;
        if (soloPresent) {
          if (!mod->isBypassed()) muteSoloExpander = mod;
        }
        else
          expanders.push_back(mod);
      }  
      else if (mod->mixType == MIXOFFSET_TYPE && !offsetPresent) {
        offsetPresent = true;
        if (!mod->isBypassed()) offsetExpander = mod;
      }
      else if (mod->mixType == MIXPAN_TYPE && stereo && !panPresent) {
        panPresent = true;
        if (!mod->isBypassed()) expanders.push_back(mod);
      }
      else if (mod->mixType == MIXSEND_TYPE) {
        sendPresent = true;
        if (!mod->isBypassed()) expanders.push_back(mod);
      }
      else if (mod->mixType == MIXSOLO_TYPE && !soloPresent) {
        soloPresent = true;
        if (mutePresent) {
          if (!mod->isBypassed()) muteSoloExpander = mod;
        }
        else
          expanders.push_back(mod);
      }
      else
        break;
    }
  }

  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "softMute", json_boolean(softMute));
    json_object_set_new(rootJ, "toggleMute", json_boolean(toggleMute));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "softMute")))
      softMute = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "toggleMute")))
      toggleMute = json_boolean_value(val);
  }

};

struct MixBaseWidget : VenomWidget {

  struct ModeSwitch : GlowingSvgSwitchLockable {
    ModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPinkButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  struct ClipSwitch : GlowingSvgSwitchLockable {
    ClipSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallWhiteButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOrangeButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
    }
  };

  struct DCBlockSwitch : GlowingSvgSwitchLockable {
    DCBlockSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
    }
  };

  struct VCAModeSwitch : GlowingSvgSwitchLockable {
    VCAModeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPinkButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallPurpleButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallLightBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallBlueButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallYellowButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallGreenButtonSwitch.svg")));
    }
  };

  struct ExcludeSwitch : GlowingSvgSwitchLockable {
    ExcludeSwitch() {
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallOffButtonSwitch.svg")));
      addFrame(Svg::load(asset::plugin(pluginInstance,"res/smallRedButtonSwitch.svg")));
    }
  };

  void appendContextMenu(Menu* menu) override {
    MixBaseModule* module = dynamic_cast<MixBaseModule*>(this->module);

    if (module->mutePresent || module->soloPresent || module->sendPresent) {
      menu->addChild(new MenuSeparator);

      menu->addChild(createBoolMenuItem("Soft mute/solo", "",
        [=]() {
          return module->softMute;
        },
        [=](bool val) {
          module->softMute = val;
        }
      ));    

      if (module->mutePresent)
        menu->addChild(createBoolMenuItem("Mute CV toggles on/off", "",
          [=]() {
            return module->toggleMute;
          },
          [=](bool val) {
            module->toggleMute = val;
          }
        ));    
    }

    VenomWidget::appendContextMenu(menu);
  }

};  

struct MixExpanderWidget : VenomWidget {
  void step() override {
    bool connected = false;
    MixBaseModule* base;
    MixModule* mixMod = dynamic_cast<MixModule*>(this->module);
    MixModule* mute = NULL;
    MixModule* offset = NULL;
    MixModule* pan = NULL;
    MixModule* solo = NULL;
    while (mixMod) {
      if ((base = dynamic_cast<MixBaseModule*>(mixMod))) {
        connected = (!pan || base->stereo);
        break;
      } else if (mixMod->mixType == MixModule::MIXMUTE_TYPE) {
        if (mute) break;
        mute = mixMod;
      } else if (mixMod->mixType == MixModule::MIXOFFSET_TYPE) {
        if (offset) break;
        offset = mixMod;
      } else if (mixMod->mixType == MixModule::MIXPAN_TYPE) {
        if (pan) break;
        pan = mixMod;
      } else if (mixMod->mixType == MixModule::MIXSOLO_TYPE) {
        if (solo) break;
        solo = mixMod;
      } else if (mixMod->mixType != MixModule::MIXSEND_TYPE) {
        break;
      }
      mixMod = dynamic_cast<MixModule*>(mixMod->getLeftExpander().module);
    }
    if(this->module) {
      mixMod = dynamic_cast<MixModule*>(this->module);
      mixMod->lights[MixModule::EXP_LIGHT].setBrightness(connected);
      if (!connected)
        for (int i=0; i<mixMod->getNumOutputs(); i++) {
          mixMod->outputs[i].setVoltage(0.f);
          mixMod->outputs[i].setChannels(1);
        }
    }
    
    VenomWidget::step();
  }  
};

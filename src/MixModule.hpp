struct MixModule : VenomModule {
  
  enum MixTypeId {
    MIX4_TYPE,
    MIX4ST_TYPE,
    VCAMIX4_TYPE,
    VCAMIX4ST_TYPE,
    MIXFADE_TYPE,
    MIXFADE2_TYPE,
    MIXMUTE_TYPE,
    MIXOFFSET_TYPE,
    MIXPAN_TYPE,
    MIXSEND_TYPE,
    MIXSOLO_TYPE
  };
  
  // all
  int mixType=-1;
  bool baseMod = false;
  bool stereo = false;

  // base only
  bool softMute = true;
  bool toggleMute = false;
  int monoPanLaw=2;    // +3 dB side
  int stereoPanLaw=10; // Follow mono law

  // expander only
  bool connected = false;

  enum ExpLightId {
    EXP_LIGHT,
    EXP_LIGHTS_LEN
  };
  
  enum FadeParamId {
    ENUMS(FADE_TIME_PARAM,4),
    FADE_MIX_TIME_PARAM,
    ENUMS(FADE_SHAPE_PARAM,4),
    FADE_MIX_SHAPE_PARAM,
    FADE_PARAMS_LEN
  };
  enum FadeInputId {
    FADE_INPUTS_LEN
  };
  enum FadeOutputId {
    ENUMS(FADE_OUTPUT,4),
    FADE_MIX_OUTPUT,
    FADE_OUTPUTS_LEN
  };
  
  enum Fade2ParamId {
    ENUMS(RISE_TIME_PARAM,4),
    ENUMS(FALL_TIME_PARAM,4),
    MIX_RISE_TIME_PARAM,
    MIX_FALL_TIME_PARAM,
    ENUMS(FADE2_SHAPE_PARAM,4),
    FADE2_MIX_SHAPE_PARAM,
    FADE2_PARAMS_LEN
  };
  enum Fade2InputId {
    FADE2_INPUTS_LEN
  };
  enum Fade2OutputId {
    ENUMS(FADE2_OUTPUT,4),
    FADE2_MIX_OUTPUT,
    FADE2_OUTPUTS_LEN
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
    ENUMS(SOLO_INPUT,4),
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
  dsp::SchmittTrigger muteCV[5], soloCV[4];
  dsp::SlewLimiter fade[5];

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
  bool mutePresent = false;
  bool offsetPresent = false;
  bool panPresent = false;
  bool sendPresent = true;
  bool soloPresent = false;
  bool fadePresent = false;
  MixModule* offsetExpander = NULL;
  MixModule* muteSoloExpander = NULL;
  MixModule* fadeExpander = NULL;
  std::vector<MixModule*> expanders;

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);

    mutePresent = false;
    offsetPresent = false;
    panPresent = false;
    sendPresent = false;
    soloPresent = false;
    fadePresent = false;
    
    // Clear expanders
    offsetExpander = NULL;
    muteSoloExpander = NULL;
    fadeExpander = NULL;
    expanders.clear();
    // Load expanders
    for (MixModule* mod = rightExpander; mod; mod = mod->rightExpander) {
      if (mod->mixType == MIXMUTE_TYPE && !mutePresent && (!soloPresent || mod->leftExpander->mixType == MIXSOLO_TYPE)) {
        mutePresent = true;
        if (soloPresent) {
          if (!mod->isBypassed()) muteSoloExpander = mod;
        }
        else
          expanders.push_back(mod);
      }
      else if ((mod->mixType == MIXFADE_TYPE || mod->mixType == MIXFADE2_TYPE) && !fadePresent && (mod->leftExpander->mixType == MIXMUTE_TYPE || mod->leftExpander->mixType == MIXSOLO_TYPE)) {
        fadePresent = true;
        if (!mod->isBypassed()) fadeExpander = mod;
      }  
      else if (mod->mixType == MIXOFFSET_TYPE && rightExpander == mod) {
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
      else if (mod->mixType == MIXSOLO_TYPE && !soloPresent && (!mutePresent || mod->leftExpander->mixType == MIXMUTE_TYPE)) {
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
    json_object_set_new(rootJ, "monoPanLaw", json_integer(monoPanLaw));
    json_object_set_new(rootJ, "stereoPanLaw", json_integer(stereoPanLaw));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "softMute")))
      softMute = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "toggleMute")))
      toggleMute = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "monoPanLaw")))
      monoPanLaw = json_integer_value(val);
    if ((val = json_object_get(rootJ, "stereoPanLaw")))
      stereoPanLaw = json_integer_value(val);
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
    MixBaseModule* module = static_cast<MixBaseModule*>(this->module);

    if (module->mutePresent || module->soloPresent || module->sendPresent || module->panPresent)
      menu->addChild(new MenuSeparator);

    if (module->mutePresent || module->soloPresent || module->sendPresent)
      menu->addChild(createBoolMenuItem("Soft mute/solo", "",
        [=]() {
          return module->softMute;
        },
        [=](bool val) {
          module->softMute = val;
        }
      ));    

    if (module->mutePresent || module->soloPresent)
      menu->addChild(createBoolMenuItem("Mute/Solo CV toggles on/off", "",
        [=]() {
          return module->toggleMute;
        },
        [=](bool val) {
          module->toggleMute = val;
        }
      ));    

    if (module->panPresent) {
      menu->addChild(createIndexPtrSubmenuItem(
        "Mono input pan law", {
          "0 dB (linear: center overpowered)",
          "+1.5 dB side (compromise: center overpowered)", "+3 dB side (equal power)", "+4.5 dB side (compromise: side overpowered)", "+6 dB side (linear: side overpowered)",
          "-1.5 dB center (compromise: center overpowered)", "-3 dB center (equal power)", "-4.5 dB center (compromise: side overpowered)", "-6 dB center (linear: side overpowered)"
        },
        &module->monoPanLaw
      ));
      menu->addChild(createIndexPtrSubmenuItem(
        "Stereo input pan law", {
          "0 dB (linear: center overpowered)",
          "+1.5 dB side (compromise: center overpowered)", "+3 dB side (equal power)", "+4.5 dB side (compromise: side overpowered)", "+6 dB side (linear: side overpowered)",
          "-1.5 dB center (compromise: center overpowered)", "-3 dB center (equal power)", "-4.5 dB center (compromise: side overpowered)", "-6 dB center (linear: side overpowered)",
          "True panning (transfer content)", "Follow mono law"
        },
        &module->stereoPanLaw
      ));
    }

    VenomWidget::appendContextMenu(menu);
  }

};  

struct MixExpanderWidget : VenomWidget {
  void step() override {
    bool connected = false;
    MixModule* mixMod = static_cast<MixModule*>(this->module);
    MixModule* thisMixMod = mixMod;
    MixModule* fade = NULL;
    MixModule* mute = NULL;
    MixModule* offset = NULL;
    MixModule* pan = NULL;
    MixModule* solo = NULL;
    while (mixMod) {
      if (mixMod->baseMod) {
        connected = (!pan || mixMod->stereo);
        break;
      } else if (mixMod->mixType == MixModule::MIXFADE_TYPE || mixMod->mixType == MixModule::MIXFADE2_TYPE) {
        if (fade || mute || solo || !mixMod->leftExpander || !(mixMod->leftExpander->mixType==MixModule::MIXSOLO_TYPE || mixMod->leftExpander->mixType==MixModule::MIXMUTE_TYPE)) break;
        fade = mixMod;
      } else if (mixMod->mixType == MixModule::MIXMUTE_TYPE) {
        if (mute || (solo && solo->leftExpander != mixMod)) break;
        mute = mixMod;
      } else if (mixMod->mixType == MixModule::MIXOFFSET_TYPE) {
        if (offset || !mixMod->leftExpander || !mixMod->leftExpander->baseMod) break;
        offset = mixMod;
      } else if (mixMod->mixType == MixModule::MIXPAN_TYPE) {
        if (pan) break;
        pan = mixMod;
      } else if (mixMod->mixType == MixModule::MIXSOLO_TYPE) {
        if (solo || (mute && mute->leftExpander != mixMod)) break;
        solo = mixMod;
      } else if (mixMod->mixType != MixModule::MIXSEND_TYPE) {
        break;
      }
      mixMod = mixMod->leftExpander;
    }
    if(thisMixMod && thisMixMod->connected != connected) {
      thisMixMod->connected = connected;
      thisMixMod->lights[MixModule::EXP_LIGHT].setBrightness(connected);
      if (!connected){
        for (int i=0; i<thisMixMod->getNumOutputs(); i++) {
          thisMixMod->outputs[i].setVoltage(0.f);
          thisMixMod->outputs[i].setChannels(1);
        }
      }
    }
    
    VenomWidget::step();
  }  
};

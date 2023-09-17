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
    RETURN_MUTE_PARAM,
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
  MixModule* muteModule = NULL;
  MixModule* offsetModule = NULL;
  MixModule* panModule = NULL;
  MixModule* soloModule = NULL;
  std::vector<MixModule*> sendModules;

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    // Clear expanders
    muteModule = NULL;
    offsetModule = NULL;
    panModule = NULL;
    soloModule = NULL;
    sendModules.clear();
    // Load expanders
    for (MixModule* mod = rightExpander; mod; mod = mod->rightExpander) {
      if (mod->mixType == MIXMUTE_TYPE && !muteModule) {
        muteModule = mod;
      }  
      else if (mod->mixType == MIXOFFSET_TYPE && !offsetModule) {
        offsetModule = mod;
      }
      else if (mod->mixType == MIXPAN_TYPE && stereo && !panModule) {
        panModule = mod;
      }
      else if (mod->mixType == MIXSEND_TYPE) {
        sendModules.push_back(mod);
      }
      else if (mod->mixType == MIXSOLO_TYPE && !soloModule) {
        if (!mod->isBypassed()) soloModule = mod;
      }
      else
        break;
    }
    // Clear bypassed expanders
    if (muteModule && muteModule->isBypassed()) muteModule = NULL;
    if (offsetModule && offsetModule->isBypassed()) offsetModule = NULL;
    if (panModule && panModule->isBypassed()) panModule = NULL;
    if (soloModule && soloModule->isBypassed()) soloModule = NULL;
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
    if(this->module) this->module->lights[MixModule::EXP_LIGHT].setBrightness(connected);

    VenomWidget::step();
  }
};

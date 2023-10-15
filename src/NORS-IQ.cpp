// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

struct NORS_IQ : VenomModule {
  
  enum ParamId {
    POI_UNIT_PARAM,
    POI_PARAM,
    EDPO_PARAM,
    LENGTH_PARAM,
    ROOT_PARAM,
    ROOT_UNIT_PARAM,
    ROUND_PARAM,
    EQUI_PARAM,
    MODE_PARAM,
    ENUMS(INTVL_PARAM,10),
    PARAMS_LEN
  };
  enum InputId {
    ENUMS(INTVL_INPUT,10),
    POI_INPUT,
    EDPO_INPUT,
    LENGTH_INPUT,
    ROOT_INPUT,
    IN_INPUT,
    TRIG_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    OUT_OUTPUT,
    TRIG_OUTPUT,
    SCALE_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    EQUI_LIGHT,
    ENUMS(NOTE_LIGHT,10),
    LIGHTS_LEN
  };
  
  enum UnitId {
    VOCT_UNIT,
    CENT_UNIT,
    NOTE_UNIT
  };
  enum RoundId {
    ROUND_DOWN,
    ROUND_NEAR,
    ROUND_UP
  };
  enum ModeId {
    QUANT_MODE,
    CHORD_MODE
  };

  dsp::SchmittTrigger trig[PORT_MAX_CHANNELS];

  NORS_IQ() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch<FixedSwitchQuantity>(POI_UNIT_PARAM, 0, 1, 0, "Interval unit", {"V/Oct", "Cents"});
    configParam(POI_PARAM, 0.f, 2.f, 1.f, "Pseudo-octave interval", " V");
    configParam(EDPO_PARAM, 1.f, 100.f, 12.f, "Equal divisions per pseudo-octave");
    configParam(LENGTH_PARAM, 1.f, 10.f, 7.f, "Scale length");
    configParam(ROOT_PARAM, -4.f, 4.f, 0.f, "Scale root");
    configSwitch<FixedSwitchQuantity>(ROOT_UNIT_PARAM, 0, 2, 0, "Scale root unit", {"V/Oct", "Cents", "Note"});
    configSwitch<FixedSwitchQuantity>(ROUND_PARAM, 0, 2, 1, "Round algorithm", {"Down", "Nearest", "Up"});
    configSwitch<FixedSwitchQuantity>(EQUI_PARAM, 0.f, 1.f, 0.f, "Equi-likely", {"Off", "On"});
    configSwitch<FixedSwitchQuantity>(MODE_PARAM, 0, 1, 0, "Mode", {"Quantize", "Chord"});
    for (int i=0; i<10; i++) {
      std::string name = "Interval " + std::to_string(i+1);
      configParam(INTVL_PARAM+i, 0, 1, 0, name);
      configInput(INTVL_INPUT+i, name + " CV");
    }
    configInput(POI_INPUT, "Pseudo-octave interval CV");
    configInput(EDPO_INPUT, "Equal divisions per pseudo-octave CV");
    configInput(LENGTH_INPUT, "Scale length CV");
    configInput(ROOT_INPUT, "Scale root CV");
    configInput(IN_INPUT, "V/Oct");
    configInput(TRIG_INPUT, "Trigger");
    configOutput(OUT_OUTPUT, "V/Oct");
    configOutput(TRIG_OUTPUT, "Trigger");
    configOutput(SCALE_OUTPUT, "Scale");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
  }


/*
  json_t* dataToJson() override {
    json_t* rootJ = VenomModule::dataToJson();
    json_object_set_new(rootJ, "monitorChannel", json_integer(lightChannel));
    json_object_set_new(rootJ, "inputPolyControl", json_boolean(inputPolyControl));
    json_object_set_new(rootJ, "audioProc", json_integer(audioProc));
    return rootJ;
  }

  void dataFromJson(json_t* rootJ) override {
    VenomModule::dataFromJson(rootJ);
    json_t* val;
    if ((val = json_object_get(rootJ, "monitorChannel")))
      lightChannel = json_integer_value(val);
    if ((val = json_object_get(rootJ, "inputPolyControl")))
      inputPolyControl = json_boolean_value(val);
    if ((val = json_object_get(rootJ, "audioProc")))
      audioProc = json_integer_value(val);
  }
*/

};

struct NORS_IQWidget : VenomWidget {

  template <class TModule>  
  struct NORS_IQDisplay : LedDisplay {
    NORS_IQ* module = NULL;
  };  

  NORS_IQWidget(NORS_IQ* module) {
    setModule(module);
    setVenomPanel("NORS_IQ");

/*
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(mm2px(Vec(5.0, 18.75)), module, NORS_IQ::NO_SWAP_LIGHT));
    addChild(createLightCentered<SmallSimpleLight<YellowLight>>(mm2px(Vec(20.431, 18.75)), module, NORS_IQ::SWAP_LIGHT));
*/    

//    addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(mm2px(Vec(12.7155, 18.75)), module, NORS_IQ::PROB_PARAM));
//    addParam(createLockableLightParamCentered<VCVLightButtonLockable<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(6.5, 31.5)), module, NORS_IQ::TRIG_PARAM, NORS_IQ::TRIG_LIGHT));

    addParam(createLockableParam<CKSSLockable>(Vec(21.698, 72.0), module, NORS_IQ::POI_UNIT_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(55.0, 82.0), module, NORS_IQ::POI_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(94.1, 82.0), module, NORS_IQ::EDPO_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(131.2, 82.0), module, NORS_IQ::LENGTH_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(169.3, 82.0), module, NORS_IQ::ROOT_PARAM));
    addParam(createLockableParam<CKSSThreeLockable>(Vec(188.881, 68.0), module, NORS_IQ::ROOT_UNIT_PARAM));
    addParam(createLockableParam<CKSSThreeLockable>(Vec(227.264, 68.0), module, NORS_IQ::ROUND_PARAM));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(278.188, 85.279), module, NORS_IQ::EQUI_PARAM, NORS_IQ::EQUI_LIGHT));
    addParam(createLockableParam<CKSSLockable>(Vec(271.622, 48.490), module, NORS_IQ::MODE_PARAM));

    float x = 22.5f, y = 206.0f;
    for (int i=0; i<10; i++) {
      addParam(createLockableParamCentered<RoundSmallBlackKnobLockable>(Vec(x, y), module, NORS_IQ::INTVL_PARAM+i));
      addInput(createInputCentered<PJ301MPort>(Vec(x, y+32.5f), module, NORS_IQ::INTVL_INPUT+i));
      x+=30.f;
    }
    addInput(createInputCentered<PJ301MPort>(Vec(55.0, 312.834), module, NORS_IQ::POI_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(94.1, 312.834), module, NORS_IQ::EDPO_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(131.2,312.834), module, NORS_IQ::LENGTH_INPUT));
    addInput(createInputCentered<PolyPJ301MPort>(Vec(169.3,312.834), module, NORS_IQ::ROOT_INPUT));

    addInput(createInputCentered<PolyPJ301MPort>(Vec(217.279, 289.616), module, NORS_IQ::IN_INPUT));
    addInput(createInputCentered<PolyPJ301MPort>(Vec(251.867, 289.616), module, NORS_IQ::TRIG_INPUT));

    addOutput(createOutputCentered<PolyPJ301MPort>(Vec(217.279, 336.052), module, NORS_IQ::OUT_OUTPUT));
    addOutput(createOutputCentered<PolyPJ301MPort>(Vec(251.867, 336.052), module, NORS_IQ::TRIG_OUTPUT));
    addOutput(createOutputCentered<PolyPJ301MPort>(Vec(286.455, 336.052), module, NORS_IQ::SCALE_OUTPUT));
    
    NORS_IQDisplay<NORS_IQ>* display = createWidget<NORS_IQDisplay<NORS_IQ>>(Vec(6.83,102.629));
    display->box.size = Vec(302.233, 84.847);
    display->module = module;
    addChild(display);
  }

  void step() override {
    VenomWidget::step();
    if(this->module) {
      this->module->lights[NORS_IQ::EQUI_LIGHT].setBrightness(this->module->params[NORS_IQ::EQUI_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }  
  }

};

Model* modelNORS_IQ = createModel<NORS_IQ, NORS_IQWidget>("NORS_IQ");

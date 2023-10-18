// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"
#include <float.h>

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
    POCT_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    EQUI_LIGHT,
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

  dsp::SchmittTrigger trigIn[16];
  dsp::PulseGenerator trigOut[16];
  int channelNote[16]{};
  int oldTrigChannels = 0;
  int oldChannels = 0;
  float oldOut[16]{};

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
    for (int i=0; i<10; i++) {
      std::string name = "Interval " + std::to_string(i+1);
      configParam(INTVL_PARAM+i, 1, 100, 1, name);
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
    configOutput(POCT_OUTPUT, "Pseudo-octave");
    configBypass(TRIG_INPUT, TRIG_OUTPUT);
    configBypass(IN_INPUT, OUT_OUTPUT);
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    float intvl = (params[POI_PARAM].getValue() + inputs[POI_INPUT].getVoltage()) 
                / (params[EDPO_PARAM].getValue() + std::round(inputs[EDPO_INPUT].getVoltage()*10.f));
    float root = params[ROOT_PARAM].getValue() + inputs[ROOT_INPUT].getVoltage();
    int len = params[LENGTH_PARAM].getValue() + std::round(inputs[LENGTH_INPUT].getVoltage());
    float step[10];
    float scale = 0.f;
    int round = params[ROUND_PARAM].getValue();
    bool equi = params[EQUI_PARAM].getValue();
    for (int i=0; i<len; i++){
      scale += (step[i] = intvl * (params[INTVL_PARAM+i].getValue() + std::round(inputs[INTVL_INPUT+i].getVoltage()*10.f)));
      outputs[SCALE_OUTPUT].setVoltage(scale+root, i+1);
    }
    outputs[SCALE_OUTPUT].setVoltage(root, 0);
    outputs[SCALE_OUTPUT].setChannels(len+1);
    int trigChannels = inputs[TRIG_INPUT].getChannels();
    if (trigChannels < oldTrigChannels) {
      for (int c=trigChannels; c<oldTrigChannels; c++)
        trigIn[c].process(0.f);
      oldTrigChannels = trigChannels;
    }
    int channels = std::max({1, trigChannels, inputs[IN_INPUT].getChannels()});
    if (channels < oldChannels) {
      for (int c=channels; c<oldChannels; c++)
        trigOut[c].reset();
      oldChannels = channels;
    }  
    for (int c=0; c<channels; c++) {
      if (trigIn[c].process(inputs[TRIG_INPUT].getPolyVoltage(c), 0.1f, 1.f) || !inputs[TRIG_INPUT].isConnected()) {
        float in = inputs[IN_INPUT].getPolyVoltage(c);
        int oct = (in - root) / scale;
        float out = scale * oct + root;
        if (in < out) {
          out -= scale;
          oct--;
        }
        float test = out;
        float testStep = scale / len;
        channelNote[c] = 0;
        for (int i=0; i<len; i++) {
          if (!equi)
            testStep = step[i];
          if (round == ROUND_NEAR && in < test + testStep/2)
            break;
          if (round == ROUND_DOWN && in < test + testStep)
            break;
          if (round == ROUND_UP && in <= test)
            break;
          test += testStep;
          out = equi ? out + step[i] : test;
          channelNote[c]++;
        }
        if (channelNote[c]==len) {
          channelNote[c]=0;
          oct++;
        }
        if (!isNear(oldOut[c],out)) {
          oldOut[c] = out;
          if (!inputs[TRIG_INPUT].isConnected())
            trigOut[c].trigger();
        }
        outputs[OUT_OUTPUT].setVoltage(out, c);
        outputs[POCT_OUTPUT].setVoltage(oct, c);
      }
      trigOut[c].process(args.sampleTime);
      outputs[TRIG_OUTPUT].setVoltage( inputs[TRIG_INPUT].isConnected() ? inputs[TRIG_INPUT].getPolyVoltage(c) : (trigOut[c].remaining > 0.f ? 10.f : 0.f), c);
    }
    outputs[OUT_OUTPUT].setChannels(channels);
    outputs[POCT_OUTPUT].setChannels(channels);
    outputs[TRIG_OUTPUT].setChannels(channels);
  }

};

struct NORS_IQWidget : VenomWidget {

  template <class TModule>  
  struct NORS_IQDisplay : LedDisplay {
    NORS_IQ* module = NULL;
  };  

  NORS_IQWidget(NORS_IQ* module) {
    setModule(module);
    setVenomPanel("NORS_IQ");

    addParam(createLockableParam<CKSSLockable>(Vec(21.698, 72.0), module, NORS_IQ::POI_UNIT_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(55.0, 82.0), module, NORS_IQ::POI_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(94.1, 82.0), module, NORS_IQ::EDPO_PARAM));
    addParam(createLockableParamCentered<RotarySwitch<RoundBlackKnobLockable>>(Vec(131.2, 82.0), module, NORS_IQ::LENGTH_PARAM));
    addParam(createLockableParamCentered<RoundBlackKnobLockable>(Vec(169.3, 82.0), module, NORS_IQ::ROOT_PARAM));
    addParam(createLockableParam<CKSSThreeLockable>(Vec(188.881, 68.0), module, NORS_IQ::ROOT_UNIT_PARAM));
    addParam(createLockableParam<CKSSThreeLockable>(Vec(233.264, 68.0), module, NORS_IQ::ROUND_PARAM));
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(284.188, 82.0), module, NORS_IQ::EQUI_PARAM, NORS_IQ::EQUI_LIGHT));

    float x = 22.5f, y = 206.0f;
    for (int i=0; i<10; i++) {
      addParam(createLockableParamCentered<RotarySwitch<RoundSmallBlackKnobLockable>>(Vec(x, y), module, NORS_IQ::INTVL_PARAM+i));
      addInput(createInputCentered<PJ301MPort>(Vec(x, y+32.5f), module, NORS_IQ::INTVL_INPUT+i));
      x+=30.f;
    }
    addInput(createInputCentered<PJ301MPort>(Vec(55.0, 312.834), module, NORS_IQ::POI_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(94.1, 312.834), module, NORS_IQ::EDPO_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(131.2,312.834), module, NORS_IQ::LENGTH_INPUT));
    addInput(createInputCentered<PJ301MPort>(Vec(169.3,312.834), module, NORS_IQ::ROOT_INPUT));

    addInput(createInputCentered<PolyPJ301MPort>(Vec(217.279, 289.616), module, NORS_IQ::IN_INPUT));
    addInput(createInputCentered<PolyPJ301MPort>(Vec(251.867, 289.616), module, NORS_IQ::TRIG_INPUT));
    addOutput(createOutputCentered<PolyPJ301MPort>(Vec(286.455, 289.616), module, NORS_IQ::TRIG_OUTPUT));

    addOutput(createOutputCentered<PolyPJ301MPort>(Vec(217.279, 336.052), module, NORS_IQ::OUT_OUTPUT));
    addOutput(createOutputCentered<PolyPJ301MPort>(Vec(251.867, 336.052), module, NORS_IQ::POCT_OUTPUT));
    addOutput(createOutputCentered<PolyPJ301MPort>(Vec(286.455, 336.052), module, NORS_IQ::SCALE_OUTPUT));
    
    NORS_IQDisplay<NORS_IQ>* display = createWidget<NORS_IQDisplay<NORS_IQ>>(Vec(6.83,102.629));
    display->box.size = Vec(302.233, 84.847);
    display->module = module;
    addChild(display);
  }

  void step() override {
    VenomWidget::step();
    NORS_IQ* mod = dynamic_cast<NORS_IQ*>(this->module);
    if(mod) {
      mod->lights[NORS_IQ::EQUI_LIGHT].setBrightness(mod->params[NORS_IQ::EQUI_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }
  }

};

Model* modelNORS_IQ = createModel<NORS_IQ, NORS_IQWidget>("NORS_IQ");

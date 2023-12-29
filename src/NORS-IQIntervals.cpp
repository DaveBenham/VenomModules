// Venom Modules (c) 2023 Dave Benham
// Licensed under GNU GPLv3

#include "plugin.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

struct NORS_IQIntervals : VenomModule {

  enum ParamId {
    FOLD_PARAM,
    PARAMS_LEN
  };
  enum InputId {
    TRIG_INPUT,
    CHORD_INPUT,
    INPUTS_LEN
  };
  enum OutputId {
    TRIG_OUTPUT,
    ROOT_OUTPUT,
    LENGTH_OUTPUT,
    INTERVALS_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId {
    FOLD_LIGHT,
    LIGHTS_LEN
  };

  dsp::SchmittTrigger trigIn;
  
  NORS_IQIntervals() {
    venomConfig(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch<FixedSwitchQuantity>(FOLD_PARAM, 0.f, 1.f, 0.f, "Fold at octaves", {"Off", "On"});
    configInput(TRIG_INPUT, "Trigger");
    configInput(CHORD_INPUT, "Chord poly");
    configOutput(TRIG_OUTPUT, "Trigger");
    configOutput(ROOT_OUTPUT, "Scale root");
    configOutput(LENGTH_OUTPUT, "Scale length");
    configOutput(INTERVALS_OUTPUT, "Scale intervals poly");
  }

  void process(const ProcessArgs& args) override {
    VenomModule::process(args);
    if (trigIn.process( inputs[TRIG_INPUT].getVoltage(), 0.1f, 2.f) || !inputs[TRIG_INPUT].isConnected()){
      bool octaveFold = params[FOLD_PARAM].getValue();
      float chord[16]{};
      int channels = inputs[CHORD_INPUT].getChannels();
      inputs[CHORD_INPUT].readVoltages(chord);
      if (octaveFold) {
        if (channels>13) channels = 13;
      } else {
        if (channels>14) channels = 14;
      }
      std::sort(chord, chord+channels);
      if (octaveFold) {
        float whole;
        float frac = std::modf(chord[channels-1]-chord[0], &whole);
        if (frac > 1e-6) {
          chord[channels++] = chord[0] + whole + 1.f;
        }  
      }
      outputs[ROOT_OUTPUT].setVoltage(chord[0]);
      outputs[ROOT_OUTPUT].setChannels(1);
      outputs[LENGTH_OUTPUT].setVoltage(static_cast<float>(channels-1)/2.f-0.5f); //NORS_IQ clamps value to valid value, so neg values OK
      outputs[LENGTH_OUTPUT].setChannels(1);
      for (int c=1; c<channels; c++) {
        outputs[INTERVALS_OUTPUT].setVoltage(chord[c]-chord[c-1], c-1);
      }
      outputs[INTERVALS_OUTPUT].setChannels(channels-1);
    }
    outputs[TRIG_OUTPUT].setVoltage(inputs[TRIG_INPUT].isConnected() && trigIn.state ? 10.f : 0.f);
    outputs[TRIG_OUTPUT].setChannels(1);
  }
  
};

struct NORS_IQIntervalsWidget : VenomWidget {
  NORS_IQIntervalsWidget(NORS_IQIntervals* module) {
    setModule(module);
    setVenomPanel("NORS_IQIntervals");
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(22.5f, 75.f), module, NORS_IQIntervals::FOLD_PARAM, NORS_IQIntervals::FOLD_LIGHT));
    addInput(createInputCentered<PJ301MPort>(Vec(22.5f,120.f), module, NORS_IQIntervals::TRIG_INPUT));
    addInput(createInputCentered<PolyPJ301MPort>(Vec(22.5f,160.f), module, NORS_IQIntervals::CHORD_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(22.5f,215.f), module, NORS_IQIntervals::TRIG_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(22.5f,255.f), module, NORS_IQIntervals::ROOT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(22.5f,295.f), module, NORS_IQIntervals::LENGTH_OUTPUT));
    addOutput(createOutputCentered<PolyPJ301MPort>(Vec(22.5f,335.f), module, NORS_IQIntervals::INTERVALS_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    NORS_IQIntervals* mod = dynamic_cast<NORS_IQIntervals*>(this->module);
    if(mod) {
      mod->lights[NORS_IQIntervals::FOLD_LIGHT].setBrightness(mod->params[NORS_IQIntervals::FOLD_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }
  }
};

Model* modelNORS_IQIntervals = createModel<NORS_IQIntervals, NORS_IQIntervalsWidget>("NORS_IQIntervals");

// Venom Modules (c) 2023, 2024 Dave Benham
// Licensed under GNU GPLv3

#include "Venom.hpp"

#define LIGHT_ON 1.f
#define LIGHT_OFF 0.02f

namespace Venom {

struct NORSIQChord2Scale : VenomModule {

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
  
  NORSIQChord2Scale() {
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

struct NORSIQChord2ScaleWidget : VenomWidget {
  NORSIQChord2ScaleWidget(NORSIQChord2Scale* module) {
    setModule(module);
    setVenomPanel("NORSIQChord2Scale");
    addParam(createLockableLightParamCentered<VCVLightButtonLatchLockable<MediumSimpleLight<WhiteLight>>>(Vec(22.5f, 85.f), module, NORSIQChord2Scale::FOLD_PARAM, NORSIQChord2Scale::FOLD_LIGHT));
    addInput(createInputCentered<MonoPort>(Vec(22.5f,125.f), module, NORSIQChord2Scale::TRIG_INPUT));
    addInput(createInputCentered<PolyPort>(Vec(22.5f,165.f), module, NORSIQChord2Scale::CHORD_INPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f,215.f), module, NORSIQChord2Scale::TRIG_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f,255.f), module, NORSIQChord2Scale::ROOT_OUTPUT));
    addOutput(createOutputCentered<MonoPort>(Vec(22.5f,295.f), module, NORSIQChord2Scale::LENGTH_OUTPUT));
    addOutput(createOutputCentered<PolyPort>(Vec(22.5f,335.f), module, NORSIQChord2Scale::INTERVALS_OUTPUT));
  }

  void step() override {
    VenomWidget::step();
    NORSIQChord2Scale* mod = dynamic_cast<NORSIQChord2Scale*>(this->module);
    if(mod) {
      mod->lights[NORSIQChord2Scale::FOLD_LIGHT].setBrightness(mod->params[NORSIQChord2Scale::FOLD_PARAM].getValue() ? LIGHT_ON : LIGHT_OFF);
    }
  }
};

}

Model* modelVenomNORSIQChord2Scale = createModel<Venom::NORSIQChord2Scale, Venom::NORSIQChord2ScaleWidget>("NORSIQChord2Scale");

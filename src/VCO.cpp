#include "plugin.hpp"
#include "dsp/math.hpp"
#include <array>
#include "util.hpp"

using std::array;

struct VCO : Module {
  enum ParamIds {
    OCT_PARAM,
    PITCH_PARAM,
    PW_PARAM,
    EXP_FM_PARAM,
    LIN_FM_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    RESET_INPUT,
    V_OCT_INPUT,
    EXP_FM_INPUT,
    LIN_FM_INPUT,
    PWM_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    SAW_OUTPUT,
    SQR_OUTPUT,
    TRI_OUTPUT,
    SIN_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  float phase = 0.0f;
  float sqPhase;
  float triPhase;
  bool sawDisc = false;
  bool sawNeg = false;
  bool square = true;
  bool sq50 = true;
  bool sqDisc = false;
  bool sqNeg = false;
  bool tri = true;
  bool triDisc = false;
  bool triNeg = false;
  bool sub = true;
  float oldPhase = 0.0f;
  float oldSqPhase;
  float oldTriPhase;
  bool oldSawDisc = false;
  bool oldSawNeg = false;
  bool oldSqDisc = false;
  bool oldSqNeg = false;
  bool oldTriDisc = false;
  bool oldTriNeg = false;
  
  array<float, 4> sawBuffer;
  array<float, 4> sqrBuffer;
  array<float, 4> triBuffer;
  array<float, 4> pwBuffer;

  float log2sampleFreq = 15.4284f;

  dsp::SchmittTrigger resetTrigger;

  VCO() {
    struct OctaveQuantity : ParamQuantity {
      std::string getDisplayValueString() override {
        VCO* module = reinterpret_cast<VCO*>(this->module);
        int val = static_cast<int>(module->params[VCO::OCT_PARAM].getValue()) - 4;
        if (val == -1)
          return "-1 = 0Hz Carrier";
        else
          return std::to_string(val);
      }
    };
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
//    configParam<OctaveQuantity>(OCT_PARAM, 3, 13, 8, "Octave", "", 0.f, 1.f, -4.f);
    configParam<OctaveQuantity>(OCT_PARAM, 3, 13, 8, "Octave", "");
    configParam(PITCH_PARAM, -5, 5, 0, "V/Oct", " Volt");
    configParam(PW_PARAM, 0.01, 0.99, 0.5, "Pulse Width", "%", 0.f, 100.f, 0.f);
    configParam(EXP_FM_PARAM, -1.0, 1.0, 0.0, "Exp. FM");
    configParam(LIN_FM_PARAM, -11.7, 11.7, 0.0, "Lin. FM");
    configInput(EXP_FM_INPUT, "Exponential FM");
    configInput(V_OCT_INPUT, "Master Pitch");
    configInput(LIN_FM_INPUT, "Linear FM");
    configInput(RESET_INPUT, "Reset");
    configOutput(SAW_OUTPUT, "Sawtooth Wave");
    configInput(PWM_INPUT, "Pulse Width Modulation");
    configOutput(SQR_OUTPUT, "Square Wave");
    configOutput(TRI_OUTPUT, "Triangle Wave");
    configOutput(SIN_OUTPUT, "Sine Wave");
  }
  void process(const ProcessArgs &args) override;
  void onSampleRateChange() override;

};


void VCO::onSampleRateChange() {
  log2sampleFreq = log2f(1.0f / APP->engine->getSampleTime()) - 0.00009f;
}

// quick explanation: the whole thing is driven by a naive sawtooth, which writes to a four-sample buffer for each
// (non-sine) waveform. Discontinuity points (or in the case of triangle, derivative continuity points) are calculated.
// When we calculate the outputs, we look to see if a discontinuity occurred in the previous sample. if one did,
// we calculate the polyblep or polyblamp and add it to each sample in the buffer. the output is the oldest buffer sample,
// which gets overwritten in the following step.

void VCO::process(const ProcessArgs &args) {

  float incr = 0.0f;
  float pw = params[PW_PARAM].getValue();
  if (inputs[PWM_INPUT].isConnected()){
//    pw = clamp(pw + inputs[PWM_INPUT].getVoltage() / 10.0f, 0.01f, 0.99f);
    pw = clamp(pw + inputs[PWM_INPUT].getVoltage() / 10.0f);
  }
  for (int i = 0; i <= 2; ++i) {
    sawBuffer[i] = sawBuffer[i + 1];
    sqrBuffer[i] = sqrBuffer[i + 1];
    triBuffer[i] = triBuffer[i + 1];
    pwBuffer[i] = pwBuffer[i+1];
  }
  pwBuffer[3] = pw * 2.0f - 1.0f;
  if (resetTrigger.process(inputs[RESET_INPUT].getVoltage())) {
    phase = 0.0f;
    square = true;
    sq50 = true;
    tri = true;
    sub = true;
  }
  else {
    float oct = params[OCT_PARAM].getValue();
    float freq = (oct==3 ? -27.0f : oct + params[PITCH_PARAM].getValue() + inputs[V_OCT_INPUT].getVoltage()) + 0.031360
               + params[EXP_FM_PARAM].getValue() * inputs[EXP_FM_INPUT].getVoltage();
    if (freq >= log2sampleFreq) {
      freq = log2sampleFreq;
    }
    freq = powf(2.0f, freq);
    float fm = oct==3 ? (params[PITCH_PARAM].getValue() + inputs[V_OCT_INPUT].getVoltage()) * 0.013976f : 0.0f;
    if (inputs[LIN_FM_INPUT].isConnected()) {
      fm = params[LIN_FM_PARAM].getValue() * params[LIN_FM_PARAM].getValue() * params[LIN_FM_PARAM].getValue() * inputs[LIN_FM_INPUT].getVoltage()
         + (oct==3 ? (params[PITCH_PARAM].getValue() + inputs[V_OCT_INPUT].getVoltage()) * 1.7f : 0.0f);
    }
    incr = clamp(args.sampleTime * (freq + fm), -0.5f, 0.5f);
    phase += incr;

    if ( (sq50 && (phase < 0.0f || phase >= 0.5f))
       ||(!sq50 && (phase < 0.5f || phase >= 1.0f))
    ) sq50 = !sq50;

    if (square && (phase < 0.0f || phase >= pw)) {
      square = false;
      sqDisc = true;
      sqNeg = phase < oldPhase;
    }
    else if(!square && (phase < pw || phase >= 1.0f)){
      square = true;
      sqDisc = true;
      sqNeg = phase < oldPhase;
    }
    else {
      sqDisc = false;
      sqNeg = false;
    }

    if (!tri != (phase >= 0.25f && phase < 0.75f)) {
      tri = !tri;
      triDisc = true;
      triNeg = phase < oldPhase;
    }
    else {
      triDisc = false;
      triNeg = false;
    }

    if (phase >= 1.0f) {
      sawDisc = true;
      sawNeg = false;
      --phase;
      sub = !sub;
    }
    else if (phase < 0.0f) {
      sawDisc = true;
      sawNeg = true;
      ++phase;
      sub = !sub;
    }
    else {
      sawDisc = false;
      sawNeg = false;
    }
  }

  sawBuffer[3] = phase;
  sqPhase = phase*2.0f - (square ? 0.0f : 1.0f);
  sqrBuffer[3] = square ? 1.0f : -1.0f;
  triPhase = tri ? (phase*2.0f + (sq50 ? 0.5f : -1.5f)) : phase*2.0f - 0.5f;
  triBuffer[3] = tri ? triPhase : (1.0f - triPhase);

  if (outputs[SAW_OUTPUT].isConnected()) {
    if (oldSawDisc) polyblep4(
      sawBuffer,
      1.0f - (oldSawNeg ? oldPhase - 1.0f : oldPhase) / incr,
      oldSawNeg ? -1.0f : 1.0f
    );
    outputs[SAW_OUTPUT].setVoltage(clamp(10.0f * (sawBuffer[0] - 0.5f), -5.0f, 5.0f));
  }
  if (outputs[SQR_OUTPUT].isConnected()) {
    if (oldSqDisc) polyblep4(
      sqrBuffer,
      1.0f - (oldSqNeg ? (oldSqPhase - 1.0f) : oldSqPhase) / incr / 2.0f,
      (sqDisc ? 2.0f : -2.0f) * (square ? 1.0f : -1.0f)
    );
//    outputs[SQR_OUTPUT].setVoltage(clamp(4.9999f * sqrBuffer[0], -5.0f, 5.0f));
    if (pwBuffer[0]<-0.99f || pwBuffer[0]>0.99f)
      outputs[SQR_OUTPUT].setVoltage(0.0f);
    else
      outputs[SQR_OUTPUT].setVoltage(clamp(5.0f * (sqrBuffer[0] - pwBuffer[0]), -10.0f, 10.0f));
  }
  if (outputs[TRI_OUTPUT].isConnected()) {
    if (oldTriDisc) polyblamp4(
      triBuffer,
      1.0f - (oldTriNeg ? oldTriPhase - 1.0f : oldTriPhase) / incr / 2.0f,
      (triDisc ? 2.0f : -2.0f) * (sq50 ? 1.0f : -1.0f) * incr * 2.0f
    );
    outputs[TRI_OUTPUT].setVoltage(clamp(10.0f * (triBuffer[0] - 0.5f), -5.0f, 5.0f));
  }

  if (outputs[SIN_OUTPUT].isConnected()) {
    outputs[SIN_OUTPUT].setVoltage(5.0f * sin_01(phase + (phase < 0.75f ? 0.25f : -0.75f)));
  }

  oldPhase = phase;
  oldSqPhase = sqPhase;
  oldTriPhase = triPhase;
  oldSawDisc = sawDisc;
  oldSawNeg = sawNeg;
  oldSqDisc = sqDisc;
  oldSqNeg = sqNeg;
  oldTriDisc = triDisc;
  oldTriNeg = triNeg;

}


struct VCOWidget : ModuleWidget {
  VCOWidget(VCO *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, faceplatePath("VCO"))));

    addParam(createParam<RoundBigBlackKnobSnap>(Vec(38, 40), module, VCO::OCT_PARAM));

    addParam(createParam<RoundLargeBlackKnob>(Vec(14.0, 110.0), module, VCO::PITCH_PARAM));
    addParam(createParam<RoundLargeBlackKnob>(Vec(70, 110), module, VCO::PW_PARAM));

    addParam(createParam<RoundLargeBlackKnob>(Vec(14, 166), module, VCO::EXP_FM_PARAM));
    addParam(createParam<RoundLargeBlackKnob>(Vec(70, 166), module, VCO::LIN_FM_PARAM));

    addInput(createInput<PJ301MPort>(Vec(11, 234), module, VCO::EXP_FM_INPUT));
    addInput(createInput<PJ301MPort>(Vec(48, 234), module, VCO::V_OCT_INPUT));
    addInput(createInput<PJ301MPort>(Vec(85, 234), module, VCO::LIN_FM_INPUT));

    addInput(createInput<PJ301MPort>(Vec(11, 276), module, VCO::RESET_INPUT));
    addOutput(createOutput<PJ301MPort>(Vec(48, 276), module, VCO::SAW_OUTPUT));
    addInput(createInput<PJ301MPort>(Vec(85, 276), module, VCO::PWM_INPUT));

    addOutput(createOutput<PJ301MPort>(Vec(11, 318), module, VCO::SQR_OUTPUT));
    addOutput(createOutput<PJ301MPort>(Vec(48, 318), module, VCO::TRI_OUTPUT));
    addOutput(createOutput<PJ301MPort>(Vec(85, 318), module, VCO::SIN_OUTPUT));

  }
};

Model *modelVCO = createModel<VCO, VCOWidget>("VenomVCO");
